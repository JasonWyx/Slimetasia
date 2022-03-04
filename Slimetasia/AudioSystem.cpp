#include "AudioSystem.h"

#include "AudioEmitter.h"
#include "AudioListener.h"
#include "Transform.h"

/// Remove
#include "ResourceManager.h"

// ===========================================================================|
// Defines                                                                    |
// ===========================================================================|
#define EDITORCHANNEL channels_[MAXAUDIOCHANNELS - 1].channel_

// ===========================================================================|
// ChannelInfo                                                                |
// ===========================================================================|
AudioSystem::ChannelInfo::ChannelInfo()
{
    Reset();
}

void AudioSystem::ChannelInfo::Reset()
{
    mute_ = false;
    volume_ = 1.0f;
    pitch_ = 1.0f;
    loop_ = false;
    loopCount_ = -1;
    minDistance3D_ = 5.0f;
    maxDistance3D_ = 100.0f;
    isPaused_ = false;
    position_ = Vector3(0, 0, 0);
    lastPosition_ = Vector3(0, 0, 0);
    audioClipName_ = "";
}
// ===========================================================================|
// ChannelManager                                                             |
// ===========================================================================|

/*
Constructor for AudioSystem::ChannelManager.
*/
AudioSystem::ChannelManager::ChannelManager()
    : inUse_{false}
    , usedBy_{nullptr}
    , channel_{nullptr}
    , emitterInfo_{nullptr}
    , defaultInfo_{}
{
}

/*
Function to reset the object
*/
void AudioSystem::ChannelManager::Reset()
{
    if (channel_) channel_->stop();

    if (usedBy_) usedBy_->channelIndex_ = -1;

    if (emitterInfo_) emitterInfo_->Reset();
    defaultInfo_.Reset();

    inUse_ = false;
    usedBy_ = nullptr;
    emitterInfo_ = nullptr;
}

// ===========================================================================|
// AUDIO SYSTEM                                                               |
// ===========================================================================|

/*
Constructor for AudioSystem.
*/
AudioSystem::AudioSystem()
    : fmodStudioSys_()
    , fmodSys_()
    , audioClips_()
    , channelGroups_()
    , events_()
    , bank_()
    , channels_()
    , audioListeners_()
    , mainListener_()
    , mainListenerLastPos_()
    , maxAudioPlayingInThisSession(PlayerPref::GetVariable<int>("MaxNumberOfAudioPlayingPerFrame", "ENGINE_STATS"))
{
    /// Set up FMOD system
    if (ErrorCheck(FMOD::Studio::System::create(&fmodStudioSys_))) std::cout << "AUDIO SYSTEM : FAIL - FMOD Studio System" << std::endl;

    /// Set up FMOD channels
    if (ErrorCheck(fmodStudioSys_->initialize(MAXAUDIOCHANNELS, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL))) std::cout << "AUDIO SYSTEM : FAIL - Channels" << std::endl;

    if (ErrorCheck(fmodStudioSys_->getLowLevelSystem(&fmodSys_))) std::cout << "AUDIO SYSTEM : FAIL - FMOD System" << std::endl;

    /// Set up 3D listeners
    if (ErrorCheck(fmodSys_->set3DNumListeners(1))) std::cout << "AUDIO SYSTEM : FAIL - Listener" << std::endl;

    /// Set 3D settings
    if (ErrorCheck(fmodSys_->set3DSettings(1.0, 1.0, 1.0))) std::cout << "AUDIO SYSTEM : FAIL - 3D settings" << std::endl;

    /// Create two channel groups : SFX | BGM
    CreateNewChannelGroup("SFX");
    CreateNewChannelGroup("BGM");

    // HACK FOR ADDING AUDIO CLIPS
    // LoadAudio("MI.ogg"       , ResourceManager::m_ResourcePathAudio + "MI.ogg"       );
    // LoadAudio("BGM_Disco.ogg", ResourceManager::m_ResourcePathAudio + "BGM_Disco.ogg");
    // LoadAudio("BGM_Game.ogg" , ResourceManager::m_ResourcePathAudio + "BGM_Game.ogg" );
    // LoadAudio("SFX_Shoot.ogg", ResourceManager::m_ResourcePathAudio + "SFX_Shoot.ogg");
}

/*
Destructor for AudioSystem.
*/
AudioSystem::~AudioSystem()
{
    /// Stop all audio
    for (auto& c : channels_)
        if (c.inUse_) c.Reset();

    /// Unload all sounds
    UnloadAllAudio();

    /// Release fmod studio
    fmodStudioSys_->release();
}

/*
Function to check for FMOD errors.
*/
bool AudioSystem::ErrorCheck(FMOD_RESULT fResult)
{
    return fResult != FMOD_OK;
}

/*
Function to update AudioSystem.
*/
void AudioSystem::Update(float dt)
{
    /// Stats
    unsigned soundPlayingInThisFrame = 0;

    /// Check channels status
    for (auto& c : channels_)
        if (c.inUse_)
        {
            /// Check if audio is still playing from this emitter
            bool isPlaying = false;
            c.channel_->isPlaying(&isPlaying);
            if (!isPlaying)
            {
                c.Reset();
                continue;
            }

            /// If there is an emitter, update it's position
            if (c.usedBy_)
            {
                /// Update 3D position if there is an emitter
                Vector3 pos = c.usedBy_->GetWorldPosition();
                Vector3 vel = (dt != 0) ? ((pos - c.emitterInfo_->lastPosition_) / dt) : Vector3{0, 0, 0};
                c.emitterInfo_->lastPosition_ = pos;

                const FMOD_VECTOR& fmodPosition = VectorToFmod(pos);
                const FMOD_VECTOR& fmodVelocity = VectorToFmod(vel);

                c.channel_->set3DAttributes(&fmodPosition, &fmodVelocity);

                /// Update sounds fading away
                if (c.usedBy_ && !c.usedBy_->IsPaused() && c.usedBy_->IsFadingOut())
                {
                    c.usedBy_->timer_ += dt;
                    c.channel_->setVolume(Math::Lerp(c.usedBy_->GetVolume(), 0.0f, c.usedBy_->timer_ / c.usedBy_->fadeDuration_));

                    if (c.usedBy_->timer_ >= c.usedBy_->fadeDuration_) c.usedBy_->Stop();
                }

                /// Update stats
                soundPlayingInThisFrame++;
            }
        }

    /// Update stats
    maxAudioPlayingInThisSession = soundPlayingInThisFrame > maxAudioPlayingInThisSession ? soundPlayingInThisFrame : maxAudioPlayingInThisSession;
    PlayerPref::SaveVariable<int>("MaxNumberOfAudioPlayingPerFrame", maxAudioPlayingInThisSession, "ENGINE_STATS");

    // REMEMBER TO CHANGE
    /// If running in editor mode snap make the editor channel always follow the listener
    // if (true)
    //  EDITORCHANNEL->set3DAttributes(&VectorToFmod(Vector3{}), &VectorToFmod(Vector3{}));

    /// Update listener position
    if (mainListener_)
    {
        Vector3 pos = mainListener_->GetPosition();
        Vector3 vel = (dt != 0) ? (pos - mainListenerLastPos_) / dt : Vector3{0, 0, 0};
        Vector3 up = mainListener_->GetUpwardVector();
        Vector3 forward = mainListener_->GetForwardVector();

        SetListenerPosition(mainListener_->GetPosition(), vel, forward, up);
    }
    else
        SetListenerPosition();

    /// OnUpdate Fmod
    fmodStudioSys_->update();
}

/*
Function to add Audio into the system
*/
void AudioSystem::LoadAudio(std::string audioClipName, std::string pathName, bool is3D, bool loop, bool stream)
{
    /// Check if sound is already in the system
    auto c = audioClips_.find(audioClipName);
    if (c != audioClips_.end())
    {
        std::cout << "AUDIO SYSTEM : FAIL - Audio already exist : " << audioClipName << std::endl;
        return;
    }

    /// Create audio clip
    FMOD_MODE mode = FMOD_DEFAULT;
    mode |= is3D ? FMOD_3D : FMOD_2D;
    mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    FMOD::Sound* newClip = nullptr;
    if (ErrorCheck(fmodSys_->createSound((pathName).c_str(), mode, nullptr, &newClip)))
        std::cout << "AUDIO SYSTEM : FAIL - Audio creation : " << audioClipName << std::endl;
    else
        std::cout << "AUDIO SYSTEM : Pass - Audio creation : " << audioClipName << std::endl;

    audioClips_.insert(std::pair<std::string, FMOD::Sound*>(audioClipName, newClip));
}

/*
Function to remove an Audio from the system.
*/
void AudioSystem::UnLoadAudio(std::string audioClipName)
{
    /// Check if sound is already in the system
    auto c = audioClips_.find(audioClipName);
    if (c == audioClips_.end()) return;

    /// Remove audio clip
    ErrorCheck(c->second->release());
    audioClips_.erase(c);
}

/*
Function to set the volume of a channel grp
*/
void AudioSystem::SetChannelGrpVol(std::string channelGrpName, float vol)
{
    /// Check if there is a channel grp with a name
    auto c = channelGroups_.find(channelGrpName);
    if (c == channelGroups_.end()) return;

    /// Set channel grp volume
    c->second->setVolume(vol);
}

/*
Function to set the volume of a channel grp
*/
float AudioSystem::GetChannelGrpVol(std::string channelGrpName)
{
    /// Check if there is a channel grp with a name
    auto c = channelGroups_.find(channelGrpName);
    if (c == channelGroups_.end()) return 0;

    /// Set channel grp volume
    float vol = 0;
    c->second->getVolume(&vol);
    return vol;
}

/*
Function to remove all Audios from the system.
*/
void AudioSystem::UnloadAllAudio()
{
    /// Release all sounds
    for (auto& c : audioClips_)
        c.second->release();

    /// Clear
    audioClips_.clear();
}

/*
Function to create a new channel group.
channelGroupName : Name of the new channel group.
*/
void AudioSystem::CreateNewChannelGroup(std::string channelGroupName)
{
    /// Check if name is already in the system
    auto c = channelGroups_.find(channelGroupName);
    if (c != channelGroups_.end())
    {
        std::cout << "AUDIO SYSTEM : FAIL - Channel group already exist : " << channelGroupName << std::endl;
        return;
    }

    /// Create new channel group
    FMOD::ChannelGroup* newGroup = nullptr;
    if (ErrorCheck(fmodSys_->createChannelGroup(channelGroupName.c_str(), &newGroup)))
        std::cout << "AUDIO SYSTEM : FAIL - New channel group created : " << channelGroupName << std::endl;
    else
        std::cout << "AUDIO SYSTEM : PASS - New channel group created : " << channelGroupName << std::endl;

    /// Add new group to the system
    channelGroups_.insert(std::pair<std::string, FMOD::ChannelGroup*>(channelGroupName, newGroup));
}

/*
Function to play from an emitter
*/
void AudioSystem::PlayAudio(AudioEmitter* emitter)
{
    if (!emitter || !emitter->m_OwnerObject) return;

    /// Check if the audioclip exists
    auto check = audioClips_.find(emitter->info_.audioClipName_);
    if (check == audioClips_.end()) return;

    /// Check if the emitter is assigned a channel
    if (!emitter->IsPlaying())
    {
        for (int i = 0; i < MAXAUDIOCHANNELS; ++i)
            if (!channels_[i].inUse_)
            {
                emitter->channelIndex_ = i;
                channels_[i].inUse_ = true;
                channels_[i].usedBy_ = emitter;
                break;
            }
    }
    else
        channels_[emitter->channelIndex_].channel_->stop();

    /// Play audio
    fmodSys_->playSound(audioClips_[emitter->info_.audioClipName_], channelGroups_[emitter->channelGroupName_], false, &channels_[emitter->channelIndex_].channel_);

    channels_[emitter->channelIndex_].channel_->setMode(FMOD_LOOP_NORMAL);
    channels_[emitter->channelIndex_].channel_->setLoopCount(emitter->info_.loop_ ? emitter->info_.loopCount_ : 0);
    channels_[emitter->channelIndex_].channel_->setVolume(emitter->info_.volume_);
    channels_[emitter->channelIndex_].channel_->setPitch(emitter->info_.pitch_);
    channels_[emitter->channelIndex_].channel_->setMute(emitter->info_.mute_);

    /// 3D sound
    FMOD_MODE is3D;
    audioClips_[emitter->info_.audioClipName_]->getMode(&is3D);
    if (is3D & FMOD_3D)
    {
        /// Check if object has a transform, if no transform play it as a 2D sound
        Transform* trans = emitter->GetOwner()->GetComponent<Transform>();
        if (trans)
        {
            Vector3 objectPos = trans->GetWorldPosition();
            FMOD_VECTOR position = VectorToFmod(objectPos);
            channels_[emitter->channelIndex_].channel_->set3DAttributes(&position, nullptr);
            channels_[emitter->channelIndex_].channel_->set3DMinMaxDistance(emitter->info_.minDistance3D_, emitter->info_.maxDistance3D_);
        }
    }

    /// Assign channel info
    channels_[emitter->channelIndex_].emitterInfo_ = &emitter->info_;
}

/*
Function to stop playing an emitter
*/
void AudioSystem::StopAudio(AudioEmitter* emitter)
{
    if (emitter->channelIndex_ > -1) channels_[emitter->channelIndex_].Reset();
}

/*
Function to stop playing an emitter
*/
void AudioSystem::PauseAudio(AudioEmitter* emitter, bool state)
{
    /// Check if emitter is playing
    if (emitter->channelIndex_ > -1)
    {
        channels_[emitter->channelIndex_].channel_->setPaused(state);
        channels_[emitter->channelIndex_].emitterInfo_->isPaused_ = state;
    }
}

/*
Function to convert 3D position to FMOD position
*/
FMOD_VECTOR AudioSystem::VectorToFmod(const Vector3& position)
{
    FMOD_VECTOR result;
    result.x = position.x;
    result.y = position.y;
    result.z = position.z;
    return result;
}

/*
Function to make a listener the main listener in the scene.
*/
void AudioSystem::MakeMainListener(AudioListener* listener)
{
    /// Reset mainListener
    if (mainListener_) mainListener_->isMain_ = false;

    /// Set new listener
    mainListener_ = listener;
    if (mainListener_) mainListener_->isMain_ = true;
}

/*
Function to find the main listener in the scene. Does not set old listener to false.
ignore : This listener will not be the main listener in the scene.
*/
void AudioSystem::FindNewListener(AudioListener* ignore)
{
    /// Take the last listener that was created
    for (size_t i = audioListeners_.size(); i > 0; --i)
    {
        if (audioListeners_[i - 1] == ignore) continue;

        mainListener_ = audioListeners_[i - 1];
        mainListener_->isMain_ = true;
        return;
    }
    mainListener_ = nullptr;
}

/*
Function to set the position of the listener.
*/
void AudioSystem::SetListenerPosition(Vector3 pos, Vector3 vel, Vector3 forward, Vector3 up)
{
    const FMOD_VECTOR& fmodPosition = VectorToFmod(pos);
    const FMOD_VECTOR& fmodVelocity = VectorToFmod(vel);
    const FMOD_VECTOR& fmodForward = VectorToFmod(forward);
    const FMOD_VECTOR& fmodUp = VectorToFmod(up);

    FMOD_RESULT tmp = fmodSys_->set3DListenerAttributes(0, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);

    mainListenerLastPos_ = pos;

    if (ErrorCheck(tmp)) std::cout << "AUDIO SYSTEM : FAIL - Set Listener position" << std::endl;
}

/*
Function for the editor to play an audio that is loaded into the Audio system.
*/
void AudioSystem::EditorPlayAudio(unsigned index)
{
    /// Stop current audio
    EditorStopAudio();

    /// Check for valid audio clip
    if (index > audioClips_.size() - 1)
    {
        std::cout << "AUDIO SYSTEM : FAIL - Attempting to play out of bounds AudioClip : " << index << " / " << audioClips_.size() - 1 << std::endl;
        return;
    }

    /// Play sound in channel dedicated to the editor
    auto iterator = audioClips_.begin();
    for (unsigned i = 0; i < index; ++i, ++iterator)
        ;
    fmodSys_->playSound(iterator->second, nullptr, false, &EDITORCHANNEL);
    EDITORCHANNEL->setVolume(1.0f);
}

/*
Function for the editor to stop playing an audio that is currently playing
*/
void AudioSystem::EditorStopAudio()
{
    bool state;
    EDITORCHANNEL->isPlaying(&state);
    if (state) EDITORCHANNEL->stop();
}

void AudioSystem::PauseAudio()
{
    for (auto& channel : channels_)
        if (channel.usedBy_) PauseAudio(channel.usedBy_, true);
}

void AudioSystem::UnPauseAudio()
{
    for (auto& channel : channels_)
        if (channel.usedBy_) PauseAudio(channel.usedBy_, false);
}

void AudioSystem::PlayAtLocation(std::string audioClipName, Vector3 Pos, float volume, float minDistance3D, float maxDistance3D, std::string channelGroup)
{
    /// Check if the audioclip exists
    auto check = audioClips_.find(audioClipName);
    if (check == audioClips_.end()) return;

    /// Find empty channel to use
    int index = -1;
    for (int i = 0; i < MAXAUDIOCHANNELS; ++i)
        if (!channels_[i].inUse_)
        {
            index = i;
            channels_[i].inUse_ = true;
            break;
        }
    if (index < 0) return;

    /// Play audio
    fmodSys_->playSound(audioClips_[audioClipName], channelGroups_[channelGroup], false, &channels_[index].channel_);

    channels_[index].channel_->setMode(FMOD_LOOP_NORMAL);
    channels_[index].channel_->setLoopCount(0);
    channels_[index].channel_->setVolume(volume);
    channels_[index].channel_->setPitch(1.0f);
    channels_[index].channel_->setMute(false);

    /// 3D sound
    FMOD_MODE is3D;
    audioClips_[audioClipName]->getMode(&is3D);
    if (is3D & FMOD_3D)
    {
        FMOD_VECTOR position = VectorToFmod(Pos);
        channels_[index].channel_->set3DAttributes(&position, nullptr);
        channels_[index].channel_->set3DMinMaxDistance(minDistance3D, maxDistance3D);
    }

    /// Assign channel info
    channels_[index].defaultInfo_.audioClipName_ = audioClipName;
    channels_[index].defaultInfo_.volume_ = volume;
    channels_[index].defaultInfo_.minDistance3D_ = minDistance3D;
    channels_[index].defaultInfo_.maxDistance3D_ = maxDistance3D;
    channels_[index].defaultInfo_.position_ = Pos;
    channels_[index].defaultInfo_.lastPosition_ = Pos;
}
