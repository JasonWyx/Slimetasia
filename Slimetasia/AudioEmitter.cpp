#include "AudioEmitter.h"

#include "AudioSystem.h"
#include "Transform.h"

// ===========================================================================|
// AUDIOEMITTER                                                               |
// ===========================================================================|

/*
Constructor for AudioEmitter.
*/
AudioEmitter::AudioEmitter(GameObject* parentObject)
    : IComponent{parentObject, "AudioEmitter"}
    , channelGroupName_{"SFX"}
    , fadingOut_{false}
    , timer_{0}
    , channelIndex_{-1}
    ,  /// Which channel is this emiiter using -1 is out of bounds
    info_{}
{
}

/*
Destructor for AudioEmitter.
*/
AudioEmitter::~AudioEmitter()
{
    if (IsPlaying()) Stop();
}

/*
Function to play the attached audioclip in the audioemitter.
*/
void AudioEmitter::Play()
{
    AUDIOSYSTEM.PlayAudio(this);
    fadingOut_ = false;
}

/*
Function to stop playing the attached audioclip in the audioemitter.
*/
void AudioEmitter::Stop()
{
    /// Check if emitter is in use
    if (IsPlaying())
    {
        AUDIOSYSTEM.StopAudio(this);
        fadingOut_ = false;
    }
}

/*
Function to pause playing from the audio emitter.
*/
void AudioEmitter::Pause(bool state)
{
    if (IsPlaying())
    {
        AUDIOSYSTEM.PauseAudio(this, state);
        fadingOut_ = false;
    }
}

/*
Function to set the mute state of the emitter.
*/
void AudioEmitter::SetMute(bool state)
{
    info_.mute_ = state;

    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->setMute(state);
}

/*
Function to get the mute state of the emitter.
*/
bool AudioEmitter::GetMute() const
{
    return info_.mute_;
}

/*
Function to check if the audio emitter is currently playing.
*/
bool AudioEmitter::IsPlaying() const
{
    return channelIndex_ >= 0;
}

/*
Function to check if the audio emitter is currently paused.
*/
bool AudioEmitter::IsPaused() const
{
    return IsPlaying() && AUDIOSYSTEM.channels_[channelIndex_].emitterInfo_->isPaused_;
}

/*
Function to change the volume of the emitter.
v : value between 0 - 1
*/
void AudioEmitter::SetVolume(float v)
{
    /// Fix v to a value between 0 - 1
    v = v < 0.0f ? 0.0f : v;
    v = v > 1.0f ? 1.0f : v;

    /// Assign value
    info_.volume_ = v;

    /// Change volume if currently playing
    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->setVolume(v);
}

/*
Function to get the current volume of the emitter.
Returns : volume between 0 - 1
*/
float AudioEmitter::GetVolume() const
{
    return info_.volume_;
}

/*
Function to set the pitch of the emitter.
*/
void AudioEmitter::SetPitch(float v)
{
    info_.pitch_ = v;

    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->setPitch(v);
}

/*
Function to get the pitch of the emitter.
*/
float AudioEmitter::GetPitch() const
{
    return info_.pitch_;
}

/*
Function to set the loop status of the emiiter.
*/
void AudioEmitter::SetLoop(bool state)
{
    info_.loop_ = state;

    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->setLoopCount(info_.loop_ ? info_.loopCount_ : 0);
}

/*
Set how many times the sound is going to loop for.
*/
void AudioEmitter::SetLoopCount(int loopCount)
{
    info_.loopCount_ = loopCount < 0 ? -1 : loopCount;
    info_.loop_ = loopCount != 0;

    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->setLoopCount(info_.loopCount_);
}

/*
Get how many times is the sound looped for.
*/
int AudioEmitter::GetLoopCount() const
{
    return (info_.loopCount_ > 0) ? info_.loopCount_ + 1 : info_.loopCount_;
}

/*
Function to set the audioclip in the emitter.
audioClipName : name of the audioclip
Returns : true if audioclip is found in the system.
*/
bool AudioEmitter::SetAudioClip(std::string audioClipName)
{
    /// Stop playing current sound
    Stop();

    /// Check if sound exists within the system
    auto c = AUDIOSYSTEM.audioClips_.find(audioClipName);
    if (c == AUDIOSYSTEM.audioClips_.end())
    {
        // std::cout << "AUDIO SYSTEM : " << audioClipName << " does not exist" << std::endl;
        info_.audioClipName_ = std::string{};
        return false;
    }

    info_.audioClipName_ = c->first;
    return true;
}

/*
Function to set the aduioclip in the emitter and play.
audioClipName : name of the audioclip
Returns : true if audioclip is found in the system.
*/
bool AudioEmitter::SetAndPlayAudioClip(std::string audioClipName)
{
    if (SetAudioClip(audioClipName))
    {
        Play();
        return true;
    }

    return false;
}

/*
Function to set the minimum distance that the 3d sound will cease to continue growing louder.
v : distance
*/
void AudioEmitter::SetMinDistance3D(float v)
{
    info_.minDistance3D_ = v;

    /// Check if playing
    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->set3DMinMaxDistance(info_.minDistance3D_, info_.maxDistance3D_);
}

/*
Function to set the maximum distance that the 3d sound will be heard.
v : distance
*/
void AudioEmitter::SetMaxDistance3D(float v)
{
    info_.maxDistance3D_ = v;

    /// Check if playing
    if (IsPlaying()) AUDIOSYSTEM.channels_[channelIndex_].channel_->set3DMinMaxDistance(info_.minDistance3D_, info_.maxDistance3D_);
}

/*
Function to get the maximum distance that the 3d sound will cease to continue growing louder.
*/
float AudioEmitter::GetMaxDistance3D() const
{
    return info_.maxDistance3D_;
}

/*
Function to get the minimum distance that the 3d sound will cease to continue growing louder.
*/
float AudioEmitter::GetMinDistance3D() const
{
    return info_.minDistance3D_;
}

/*
Function to get the emitter's global position.
*/
Vector3 AudioEmitter::GetWorldPosition() const
{
    if (m_OwnerObject)
    {
        auto transformRef_ = m_OwnerObject->GetComponent<Transform>();
        return transformRef_ ? transformRef_->GetWorldPosition() : Vector3{};
    }
    return Vector3{};
}

/*
Function to fade out a track if the emitter is currently in use
*/
void AudioEmitter::FadeOut(float duration)
{
    if (IsPlaying() && !fadingOut_)
    {
        fadingOut_ = true;
        timer_ = 0;
        fadeDuration_ = duration;
    }
}

/*
Function that returns fadingOut_
*/
bool AudioEmitter::IsFadingOut() const
{
    return fadingOut_;
}

bool AudioEmitter::SetChannelGroup(std::string name)
{
    channelGroupName_ = name;
    return true;
}

/*
Reflection.
*/
REFLECT_INIT(AudioEmitter)
REFLECT_PARENT(IComponent)
REFLECT_PROPERTY(info_.audioClipName_)  /// How to do a drop down and select
REFLECT_PROPERTY(channelGroupName_)     /// How to do a drop down and select
REFLECT_PROPERTY(info_.mute_)
REFLECT_PROPERTY(info_.volume_)
REFLECT_PROPERTY(info_.pitch_)
REFLECT_PROPERTY(info_.loop_)
REFLECT_PROPERTY(info_.loopCount_)
REFLECT_PROPERTY(info_.minDistance3D_)
REFLECT_PROPERTY(info_.maxDistance3D_)
REFLECT_END()