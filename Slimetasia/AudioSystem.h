#pragma once
// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include <Windows.h>

/// Others
#include "Action.h"
#include "AppConsole.h"
#include "Application.h"
#include "CorePrerequisites.h"
#include "External Libraries\imgui\imgui.h"
#include "ISystem.h"
#include "Input.h"

/// Fmod
#include <array>
#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <map>

// ===========================================================================|
// DEFINE                                                                     |
// ===========================================================================|
#define MAXAUDIOCHANNELS 300

// ===========================================================================|
// FORWARD DECLARATIONS                                                       |
// ===========================================================================|
class AudioEmitter;
class AudioListener;

// ===========================================================================|
// AUDIO SYSTEM                                                               |
// ===========================================================================|
class AudioSystem : public ISystem<AudioSystem>
{
    /// Friends -----------------------------------------------------------------
    friend class ISystem<AudioSystem>;
    friend class AudioEmitter;
    friend class AudioListener;

    /// Classes ----------------------------------------------------------------- // JJ continue here pls
    struct ChannelInfo
    {
        bool mute_;
        float volume_;
        float pitch_;
        bool loop_;
        int loopCount_;
        float minDistance3D_;
        float maxDistance3D_;
        bool isPaused_;
        Vector3 position_;
        Vector3 lastPosition_;
        std::string audioClipName_;

        ChannelInfo();
        void Reset();
    };

    struct ChannelManager
    {
        bool inUse_;
        AudioEmitter* usedBy_;
        FMOD::Channel* channel_;
        ChannelInfo* emitterInfo_;
        ChannelInfo defaultInfo_;

        /// Functions
        ChannelManager();
        void Reset();
    };

    /// Typedefs ----------------------------------------------------------------
    typedef std::map<std::string, FMOD::Sound*> SoundMap;
    typedef std::map<std::string, FMOD::ChannelGroup*> ChannelGroupMap;
    typedef std::map<std::string, FMOD::Studio::EventInstance*> EventMap;
    typedef std::map<std::string, FMOD::Studio::Bank*> BankMap;
    typedef std::array<ChannelManager, MAXAUDIOCHANNELS> ChannelMap;

    /// Variables ---------------------------------------------------------------
private:

    FMOD::Studio::System* fmodStudioSys_;
    FMOD::System* fmodSys_;

    SoundMap audioClips_;
    ChannelGroupMap channelGroups_;
    EventMap events_;  /// Currently not in use
    BankMap bank_;     /// Currently not in use
    ChannelMap channels_;

    std::vector<AudioListener*> audioListeners_;
    AudioListener* mainListener_;
    Vector3 mainListenerLastPos_;

    /// STATS
    unsigned maxAudioPlayingInThisSession;
    std::map<std::string, unsigned> soundsPlayedInthisSession;

    /// Functions ---------------------------------------------------------------
public:

    AudioSystem();
    ~AudioSystem();

    void Update(float dt);

private:

    static bool ErrorCheck(FMOD_RESULT fResult);

public:

    void LoadAudio(std::string audioClipName, std::string pathName, bool is3D = true, bool loop = false, bool stream = false);
    void UnLoadAudio(std::string audioClipName);

    void SetChannelGrpVol(std::string channelGrpName, float vol);
    float GetChannelGrpVol(std::string channelGrpName);

private:

    void UnloadAllAudio();

    void CreateNewChannelGroup(std::string channelGroupName);

    void PlayAudio(AudioEmitter* emitter);
    void StopAudio(AudioEmitter* emitter);
    void PauseAudio(AudioEmitter* emitter, bool state);

    FMOD_VECTOR VectorToFmod(const Vector3& position);

    void MakeMainListener(AudioListener* listener);
    void FindNewListener(AudioListener* ignore = nullptr);
    void SetListenerPosition(Vector3 pos = Vector3 {}, Vector3 vel = Vector3 {}, Vector3 forward = Transform::worldForward, Vector3 up = Transform::worldUpward);

    // Editor Functions
public:

    void EditorPlayAudio(unsigned index);
    void EditorStopAudio();
    void PauseAudio();
    void UnPauseAudio();
    SoundMap& GetSoundMap() { return audioClips_; }

    void PlayAtLocation(std::string audioClipName, Vector3 Pos, float volume = 1.0f, float minDistance3D = 5, float maxDistance3D = 100, std::string channelGroup = "SFX");
};

#define AUDIOSYSTEM AudioSystem::Instance()
