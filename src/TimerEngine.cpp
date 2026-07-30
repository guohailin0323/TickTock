/**
 * @file    TimerEngine.cpp
 * @brief   倒计时状态机实现。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "TimerEngine.h"
#include "spdlog/spdlog.h"

namespace TickTock {

// 单位换算常量，避免代码中出现魔数
static constexpr int MILLISECONDS_PER_SECOND = 1000;  ///< 1 秒 = 1000 毫秒
static constexpr int SECONDS_PER_MINUTE      = 60;    ///< 1 分钟 = 60 秒
static constexpr int SECONDS_PER_HOUR        = 3600;  ///< 1 小时 = 3600 秒

TimerEngine::TimerEngine()
    : m_state(TimerState::Idle)
    , m_totalDurationMs(0)
    , m_remainingMs(0)
    , m_durationHours(0)
    , m_durationMinutes(0)
    , m_durationSeconds(0)
{
    spdlog::debug("TimerEngine 创建");
}

void TimerEngine::SetDuration(int hours, int minutes, int seconds)
{
    m_durationHours   = hours;
    m_durationMinutes = minutes;
    m_durationSeconds = seconds;

    int totalSeconds      = hours * SECONDS_PER_HOUR
                          + minutes * SECONDS_PER_MINUTE
                          + seconds;
    m_totalDurationMs     = totalSeconds * MILLISECONDS_PER_SECOND;
    m_remainingMs         = m_totalDurationMs;
    m_state               = TimerState::Idle;

    spdlog::info("SetDuration: {:02d}:{:02d}:{:02d}，总时长={}ms，状态重置为 Idle",
                 hours, minutes, seconds, m_totalDurationMs);
}

void TimerEngine::Start()
{
    // Running 或 Finished 状态下调用 Start 无意义，直接忽略
    if (m_state == TimerState::Running || m_state == TimerState::Finished)
    {
        spdlog::debug("Start() 忽略：当前状态不允许启动，state={}",
                      static_cast<int>(m_state));
        return;
    }

    TimerState prevState = m_state;
    m_state = TimerState::Running;
    spdlog::info("Start(): {} → Running，剩余={}ms",
                 static_cast<int>(prevState), m_remainingMs);
}

void TimerEngine::Pause()
{
    // 只有 Running 状态才能暂停
    if (m_state != TimerState::Running)
    {
        spdlog::debug("Pause() 忽略：当前不是 Running，state={}",
                      static_cast<int>(m_state));
        return;
    }

    m_state = TimerState::Paused;
    spdlog::info("Pause(): Running → Paused，剩余={}ms", m_remainingMs);
}

void TimerEngine::Reset()
{
    TimerState prevState = m_state;
    m_state              = TimerState::Idle;
    m_remainingMs        = m_totalDurationMs;

    spdlog::info("Reset(): {} → Idle，剩余恢复为 {}ms",
                 static_cast<int>(prevState), m_remainingMs);
}

bool TimerEngine::Tick(int elapsedMs)
{
    // 非 Running 状态不扣时间
    if (m_state != TimerState::Running)
        return false;

    m_remainingMs -= elapsedMs;

    // 剩余时间不允许为负数
    if (m_remainingMs <= 0)
    {
        m_remainingMs = 0;
        m_state       = TimerState::Finished;
        spdlog::info("Tick(): 倒计时归零，Running → Finished");
        return true;
    }

    return false;
}

TimerState TimerEngine::GetState() const
{
    return m_state;
}

int TimerEngine::GetRemainingSeconds() const
{
    // 向下取整：剩余 1500ms → 1 秒
    return m_remainingMs / MILLISECONDS_PER_SECOND;
}

int TimerEngine::GetDurationHours() const
{
    return m_durationHours;
}

int TimerEngine::GetDurationMinutes() const
{
    return m_durationMinutes;
}

int TimerEngine::GetDurationSeconds() const
{
    return m_durationSeconds;
}

} // namespace TickTock
