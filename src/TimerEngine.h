/**
 * @file    TimerEngine.h
 * @brief   倒计时状态机：管理剩余时间和 Idle/Running/Paused/Finished 四个状态。
 *          不含任何 UI 逻辑，不依赖 Windows 消息机制，可独立单元测试。
 *          UI 层每 100ms 调用一次 Tick()，由 TimerEngine 自己维护剩余时间。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

namespace TickTock {

/**
 * 倒计时四种状态。
 * 转换关系：
 *   Idle ──[Start]──▶ Running ──[Tick 归零]──▶ Finished
 *     ▲                 │                          │
 *     └──[Reset]────────┘◀──[Start]──Paused◀[Pause]│
 *     └──────────────────────────[Reset]────────────┘
 */
enum class TimerState
{
    Idle,     ///< 未启动或已重置；可以调 SetDuration / Start
    Running,  ///< 倒计时进行中；Tick 每次调用都扣减剩余时间
    Paused,   ///< 已暂停；Tick 不扣时间；可 Start(恢复) 或 Reset
    Finished  ///< 倒计时归零；等待 UI 层处理（播音等）；Reset 返回 Idle
};

/**
 * @class   TimerEngine
 * @brief   纯逻辑倒计时引擎，与 UI 框架完全解耦。
 *
 *          典型用法：
 *          @code
 *          TimerEngine engine;
 *          engine.SetDuration(0, 5, 0);   // 设置 5 分钟
 *          engine.Start();
 *          // 每 100ms：
 *          if (engine.Tick(100))          // 返回 true 表示刚归零
 *              PlayAlarmAudio();
 *          int remaining = engine.GetRemainingSeconds();
 *          @endcode
 *
 *          线程安全性：所有方法必须在同一线程（UI 线程）调用，不支持跨线程访问。
 */
class TimerEngine
{
public:
    TimerEngine();

    /**
     * @brief   设置倒计时时长，同时将状态重置为 Idle。
     *          在任意状态下调用均有效，会中断当前计时。
     * @param   hours   小时，取值范围 [0, 23]
     * @param   minutes 分钟，取值范围 [0, 59]
     * @param   seconds 秒，取值范围 [0, 59]
     */
    void SetDuration(int hours, int minutes, int seconds);

    /**
     * @brief   启动或恢复倒计时。
     *          Idle → Running，Paused → Running。
     *          若已是 Running 或 Finished，调用无效果。
     */
    void Start();

    /**
     * @brief   暂停倒计时。Running → Paused。
     *          若不是 Running，调用无效果。
     */
    void Pause();

    /**
     * @brief   重置计时器到 Idle 状态，剩余时间恢复为 SetDuration 的设定值。
     *          在任意状态下均有效。
     */
    void Reset();

    /**
     * @brief   推进时间，由 UI 层周期性调用（建议每 100ms 一次）。
     *          仅 Running 状态下扣减时间；其他状态调用无效果。
     * @param   elapsedMs  自上次调用以来经过的毫秒数，须 >= 0
     * @return  true  表示本次 Tick 触发了 Running → Finished 状态跃迁
     *          false 表示状态未变化（仍在 Running 或非 Running 状态）
     */
    bool Tick(int elapsedMs);

    /**
     * @brief   返回当前状态。
     */
    TimerState GetState() const;

    /**
     * @brief   返回剩余整秒数（向下取整），最小为 0。
     */
    int GetRemainingSeconds() const;

    /**
     * @brief   返回 SetDuration 设置的小时部分（用于 UI 回显）。
     */
    int GetDurationHours() const;

    /**
     * @brief   返回 SetDuration 设置的分钟部分。
     */
    int GetDurationMinutes() const;

    /**
     * @brief   返回 SetDuration 设置的秒部分。
     */
    int GetDurationSeconds() const;

private:
    TimerState m_state;            ///< 当前计时器状态
    int        m_totalDurationMs;  ///< SetDuration 换算的总时长（毫秒），Reset 时用于还原
    int        m_remainingMs;      ///< 当前剩余时长（毫秒），Running 时由 Tick 递减，最小 0

    int        m_durationHours;    ///< 用户设定的小时数，UI 重置显示用
    int        m_durationMinutes;  ///< 用户设定的分钟数
    int        m_durationSeconds;  ///< 用户设定的秒数
};

} // namespace TickTock
