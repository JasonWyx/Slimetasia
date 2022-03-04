#pragma once
#include <chrono>

using namespace std::chrono;
using Clock = std::chrono::high_resolution_clock;

class Timer
{
private:
    Clock::time_point m_StartTime;
    Clock::time_point m_CurrentTime;
    Clock::time_point m_PreviousTime;

    // For countdown use
    float m_Ticks;

    // For general timer use
    float m_FrameTime;
    float m_TimeScale;

    // Paused flags
    bool m_IsEditorPaused;
    bool m_IsGamePaused;
    bool m_IsPlayModePause;

public:
    Timer();
    Timer(const float& t);  // Constructs with countdown initialized

    void StartCountdown(const float& t);  // Start countdown timer
    bool CountdownEnded();                // Checks if countdown ended

    void StartTimer();  // Starts timer resettings frame time values
    void EndTimer();    // Ends timer setting frame time values

    float GetTimePassed();      // Gets time since last call to GetTimePassed() OR StartTimer()
    float GetTimeSinceStart();  // Gets time since timer start. Does NOT alter current timestamp.

    float GetScaledFrameTime();
    float GetActualFrameTime();

    void SetTimeScale(float const& scale);
    float GetTimeScale() const;

    void SetEditorPaused(bool b);
    bool IsEditorPaused() const;
    void SetGamePaused(bool b);
    bool IsGamePaused() const;
    bool IsPlayModePaused() const;
    void SetPlayModePaused(bool b);
};