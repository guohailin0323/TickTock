/**
 * @file    test_timer_engine.cpp
 * @brief   TimerEngine 单元测试。独立控制台程序，所有测试通过返回 0，有失败返回非 0。
 *          运行方式：直接执行 TimerEngineTest.exe，或通过 ctest 运行。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */

#include <iostream>
#include "TimerEngine.h"
#include "spdlog/spdlog.h"

using namespace TickTock;

// 全局计数器，记录通过/失败数量
static int g_passed = 0;
static int g_failed = 0;

/**
 * 断言宏：打印 PASS/FAIL，并统计结果。
 * 不使用 assert() 是因为 assert 失败直接 abort，无法看到后续测试结果。
 */
#define CHECK(expr) \
    do { \
        if (expr) \
        { \
            std::cout << "  [PASS] " << #expr << "\n"; \
            ++g_passed; \
        } \
        else \
        { \
            std::cerr << "  [FAIL] " << #expr \
                      << "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            ++g_failed; \
        } \
    } while (0)

// ─────────────────────────────────────────────────────────────────────────────
// 测试用例
// ─────────────────────────────────────────────────────────────────────────────

/// 测试1：初始状态是 Idle，GetRemainingSeconds() 返回 0
static void test_initial_state()
{
    std::cout << "\n[test1] 初始状态\n";
    TimerEngine engine;
    CHECK(engine.GetState() == TimerState::Idle);
    CHECK(engine.GetRemainingSeconds() == 0);
}

/// 测试2：SetDuration(0,5,0) 后 GetRemainingSeconds() 返回 300
static void test_set_duration_returns_correct_seconds()
{
    std::cout << "\n[test2] SetDuration 设置正确的剩余秒数\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);
    CHECK(engine.GetRemainingSeconds() == 300);
    // SetDuration 会将状态重置为 Idle
    CHECK(engine.GetState() == TimerState::Idle);
}

/// 测试3：Start() 后状态变为 Running
static void test_start_transitions_to_running()
{
    std::cout << "\n[test3] Start 后状态变为 Running\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);
    engine.Start();
    CHECK(engine.GetState() == TimerState::Running);
    // 剩余时间不变
    CHECK(engine.GetRemainingSeconds() == 300);
}

/// 测试4：Running 状态下累计 Tick(300000ms) 后状态变为 Finished，剩余 = 0
static void test_tick_until_finished()
{
    std::cout << "\n[test4] 累计 Tick 300s 后进入 Finished\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);
    engine.Start();
    bool triggered = engine.Tick(300000);
    // Tick 返回 true 表示本次触发了 Running → Finished
    CHECK(triggered == true);
    CHECK(engine.GetState() == TimerState::Finished);
    CHECK(engine.GetRemainingSeconds() == 0);
}

/// 测试5：Running 中 Tick(1000ms) 一次后剩余减少 1 秒
static void test_tick_decrements_one_second()
{
    std::cout << "\n[test5] Tick(1000ms) 减少 1 秒\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);  // 300 秒
    engine.Start();
    bool triggered = engine.Tick(1000);
    CHECK(triggered == false);
    CHECK(engine.GetRemainingSeconds() == 299);
    CHECK(engine.GetState() == TimerState::Running);
}

/// 测试6：Pause() 后状态变 Paused，继续 Tick 不扣时间
static void test_pause_stops_countdown()
{
    std::cout << "\n[test6] Pause 后 Tick 不扣时间\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);
    engine.Start();
    engine.Tick(1000);       // 剩余 299 秒
    engine.Pause();
    CHECK(engine.GetState() == TimerState::Paused);
    engine.Tick(10000);      // Paused 状态 Tick 不应扣时
    CHECK(engine.GetRemainingSeconds() == 299);
}

/// 测试7：Paused 下 Start()（恢复）后状态回 Running，Tick 继续扣时
static void test_resume_restarts_countdown()
{
    std::cout << "\n[test7] Paused 后 Start 恢复，Tick 继续扣时\n";
    TimerEngine engine;
    engine.SetDuration(0, 5, 0);
    engine.Start();
    engine.Tick(1000);   // 剩余 299 秒
    engine.Pause();
    engine.Start();      // 从 Paused 恢复
    CHECK(engine.GetState() == TimerState::Running);
    engine.Tick(1000);   // 继续扣时
    CHECK(engine.GetRemainingSeconds() == 298);
}

/// 测试8：任意状态下 Reset() → Idle，剩余恢复设定值
static void test_reset_from_any_state()
{
    std::cout << "\n[test8] 任意状态 Reset 回 Idle，剩余恢复\n";
    TimerEngine engine;
    engine.SetDuration(0, 3, 0);  // 180 秒

    // 从 Running 重置
    engine.Start();
    engine.Tick(5000);
    engine.Reset();
    CHECK(engine.GetState() == TimerState::Idle);
    CHECK(engine.GetRemainingSeconds() == 180);

    // 从 Paused 重置
    engine.Start();
    engine.Pause();
    engine.Reset();
    CHECK(engine.GetState() == TimerState::Idle);
    CHECK(engine.GetRemainingSeconds() == 180);
}

/// 测试9：Finished 状态下 Reset() → Idle
static void test_reset_from_finished()
{
    std::cout << "\n[test9] Finished 后 Reset 回 Idle\n";
    TimerEngine engine;
    engine.SetDuration(0, 0, 5);  // 5 秒
    engine.Start();
    engine.Tick(5000);
    CHECK(engine.GetState() == TimerState::Finished);
    engine.Reset();
    CHECK(engine.GetState() == TimerState::Idle);
    CHECK(engine.GetRemainingSeconds() == 5);
}

/// 测试10：Tick 超时后剩余 = 0，不为负数
static void test_remaining_never_negative()
{
    std::cout << "\n[test10] Tick 大量超时后剩余 >= 0\n";
    TimerEngine engine;
    engine.SetDuration(1, 0, 0);  // 1 小时 = 3600 秒
    engine.Start();
    engine.Tick(99999999);        // 远超时长
    CHECK(engine.GetRemainingSeconds() == 0);
    CHECK(engine.GetState() == TimerState::Finished);
}

// ─────────────────────────────────────────────────────────────────────────────
// 主函数
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    // 关闭 spdlog 日志输出，避免测试结果被日志淹没；调试时可改为 debug
    spdlog::set_level(spdlog::level::off);

    test_initial_state();
    test_set_duration_returns_correct_seconds();
    test_start_transitions_to_running();
    test_tick_until_finished();
    test_tick_decrements_one_second();
    test_pause_stops_countdown();
    test_resume_restarts_countdown();
    test_reset_from_any_state();
    test_reset_from_finished();
    test_remaining_never_negative();

    std::cout << "\n========================================\n";
    std::cout << "结果：" << g_passed << " 通过，" << g_failed << " 失败\n";
    std::cout << "========================================\n";

    // 全部通过返回 0，有失败返回非 0（ctest 以返回值判断成败）
    return g_failed;
}
