#pragma once
// ===========================================================================|
// INCLUDES                                                                   |
// ===========================================================================|
#include <Windows.h>

#include "Actions.h"
#include "AppConsole.h"
#include "Application.h"
#include "AudioSystem.h"
#include "CorePrerequisites.h"
#include "External Libraries\imgui\imgui.h"
#include "IComponent.h"

// ===========================================================================|
// AUDIOEMITTER                                                               |
// ===========================================================================|
class AudioEmitter : public IComponent
{
    /// Friend ------------------------------------------------------------------
    friend class AudioSystem;

    /// Variables ---------------------------------------------------------------
private:
    AudioSystem::ChannelInfo info_;  /// Info regarding what the emitter is playing

    bool fadingOut_;      /// Is the emitter fading out
    float timer_;         /// Timer for fade out
    float fadeDuration_;  /// Fade duration

    std::string channelGroupName_;  /// Name of the channel group this emitter is in
    int channelIndex_;              /// Index of the channel that this emitter is using

    /// Functions ---------------------------------------------------------------
public:
    AudioEmitter(GameObject* parentObject);
    ~AudioEmitter();

    void Play();
    void Stop();
    void Pause(bool state);
    void SetMute(bool state);
    bool GetMute() const;
    bool IsPlaying() const;
    bool IsPaused() const;
    void SetVolume(float v);
    float GetVolume() const;
    void SetPitch(float v);
    float GetPitch() const;
    void SetLoop(bool state);
    void SetLoopCount(int loopCount);
    int GetLoopCount() const;
    bool SetAudioClip(std::string audioClipName);
    bool SetAndPlayAudioClip(std::string audioClipName);
    void SetMinDistance3D(float v);
    void SetMaxDistance3D(float v);
    float GetMaxDistance3D() const;
    float GetMinDistance3D() const;
    Vector3 GetWorldPosition() const;
    void FadeOut(float duration);
    bool IsFadingOut() const;

    bool SetChannelGroup(std::string name);

    std::string GetSoundName() const { return info_.audioClipName_; }

    REFLECT();
};
