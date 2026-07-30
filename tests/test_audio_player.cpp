/**
 * @file    test_audio_player.cpp
 * @brief   AudioPlayer 单元测试。独立控制台程序，所有测试通过返回 0。
 *          测试5（真实播放）跳过，留人工验证。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include <iostream>
#include "AudioPlayer.h"
#include "spdlog/spdlog.h"

using namespace TickTock;

static int g_passed = 0;
static int g_failed = 0;

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

/// 测试1：LoadFile("") 返回 false，不崩溃
static void test_load_empty_path_returns_false()
{
    std::cout << "\n[test1] LoadFile(\"\") 返回 false\n";
    AudioPlayer player;
    bool result = player.LoadFile(L"");
    CHECK(result == false);
}

/// 测试2：LoadFile("不存在的路径") 返回 false
static void test_load_nonexistent_file_returns_false()
{
    std::cout << "\n[test2] LoadFile(不存在路径) 返回 false\n";
    AudioPlayer player;
    bool result = player.LoadFile(L"C:\\不存在的文件路径\\test_xyz_999.mp3");
    CHECK(result == false);
}

/// 测试3：Stop() 在未加载文件时调用，不崩溃
static void test_stop_without_load_no_crash()
{
    std::cout << "\n[test3] Stop() 未加载文件，不崩溃\n";
    AudioPlayer player;
    player.Stop();
    CHECK(true);  // 能执行到这里即代表没崩溃
}

/// 测试4：Play() 在未加载文件时调用，不崩溃
static void test_play_without_load_no_crash()
{
    std::cout << "\n[test4] Play() 未加载文件，不崩溃\n";
    AudioPlayer player;
    player.Play();
    CHECK(true);  // 能执行到这里即代表没崩溃
}

/// 测试5：（跳过）真实文件播放，人工验证
static void test_real_file_play_skipped()
{
    std::cout << "\n[test5] 真实文件播放 → 跳过，人工验证\n";
    std::cout << "  [SKIP] 此测试需要真实音频文件，留手动验证\n";
}

/// 测试6：Play() 后 Stop()，再次 Play() 不崩溃
static void test_play_stop_play_no_crash()
{
    std::cout << "\n[test6] Play→Stop→Play 循环不崩溃\n";
    AudioPlayer player;
    // 未加载文件，三个操作都应该安全返回
    player.Play();
    player.Stop();
    player.Play();
    CHECK(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// 主函数
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    spdlog::set_level(spdlog::level::off);

    test_load_empty_path_returns_false();
    test_load_nonexistent_file_returns_false();
    test_stop_without_load_no_crash();
    test_play_without_load_no_crash();
    test_real_file_play_skipped();
    test_play_stop_play_no_crash();

    std::cout << "\n========================================\n";
    std::cout << "结果：" << g_passed << " 通过，" << g_failed << " 失败\n";
    std::cout << "========================================\n";
    return g_failed;
}
