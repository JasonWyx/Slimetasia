#include "Timer.h"

Timer::Timer()
    : m_StartTime(Clock::now())
    , m_CurrentTime(Clock::now())
    , m_Ticks(0)
    , m_FrameTime(0.f)
    , m_TimeScale(1.0f)
    , m_IsEditorPaused(false)
    , m_IsGamePaused(false)
{
}

Timer::Timer(const float& t)
    : m_StartTime(Clock::now())
    , m_CurrentTime(Clock::now())
    , m_Ticks(t)
    , m_FrameTime(0.f)
    , m_TimeScale(1.0f)
    , m_IsEditorPaused(false)
    , m_IsGamePaused(false)
    , m_IsPlayModePause(false)
{
}

void Timer::StartCountdown(const float& t)
{
    m_Ticks = t;
    m_StartTime = m_PreviousTime = m_CurrentTime = Clock::now();
}

bool Timer::CountdownEnded()
{
    if (duration_cast<duration<float>>(Clock::now() - m_StartTime).count() <= m_Ticks) return false;
    return true;
}

void Timer::StartTimer()
{
    m_StartTime = m_PreviousTime = m_CurrentTime = Clock::now();
}

void Timer::EndTimer()
{
    m_CurrentTime = Clock::now();
    m_FrameTime = duration_cast<duration<float>>(m_CurrentTime - m_StartTime).count();
}

float Timer::GetTimePassed()
{
    m_PreviousTime = m_CurrentTime;
    m_CurrentTime = Clock::now();
    return duration_cast<duration<float>>(m_CurrentTime - m_PreviousTime).count();
}

float Timer::GetTimeSinceStart()
{
    return duration_cast<duration<float>>(Clock::now() - m_StartTime).count();
    ;
}

float Timer::GetScaledFrameTime()
{
    if (m_IsEditorPaused || m_IsGamePaused || m_IsPlayModePause) return 0.f;
    return m_FrameTime * m_TimeScale;
}

float Timer::GetActualFrameTime()
{
    return m_FrameTime;
}

void Timer::SetTimeScale(float const& scale)
{
    m_TimeScale = scale;
}

float Timer::GetTimeScale() const
{
    return m_TimeScale;
}

void Timer::SetEditorPaused(bool b)
{
    m_IsEditorPaused = b;
}

bool Timer::IsEditorPaused() const
{
    return m_IsEditorPaused;
}

void Timer::SetGamePaused(bool b)
{
    m_IsGamePaused = b;
}

bool Timer::IsGamePaused() const
{
    return m_IsGamePaused;
}

bool Timer::IsPlayModePaused() const
{
    return m_IsPlayModePause;
}

void Timer::SetPlayModePaused(bool b)
{
    m_IsPlayModePause = b;
}
