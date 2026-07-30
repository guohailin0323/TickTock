/**
 * @file    test_app_settings.cpp
 * @brief   AppSettings 单元测试。独立控制台程序，所有测试通过返回 0。
 *          测试前会在系统临时目录创建测试用配置文件，测试后清理。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include <iostream>
#include <windows.h>
#include "AppSettings.h"
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

// 获取测试用的临时 INI 文件路径（避免污染真实配置）
static std::wstring GetTestIniPath()
{
    wchar_t tempDir[MAX_PATH] = {};
    ::GetTempPathW(MAX_PATH, tempDir);
    return std::wstring(tempDir) + L"TickTock_test_settings.ini";
}

// 删除测试文件（清理现场）
static void RemoveTestFile(const std::wstring &path)
{
    ::DeleteFileW(path.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
// 测试用例
// ─────────────────────────────────────────────────────────────────────────────

/// 测试1：保存配置后文件存在于磁盘
static void test_save_creates_file()
{
    std::cout << "\n[test1] 保存后文件存在\n";
    std::wstring path = GetTestIniPath();
    RemoveTestFile(path);

    AppSettings settings(path);
    settings.SetHours(1);
    settings.SetMinutes(30);
    settings.SetSeconds(0);
    settings.SetMusicPath(L"C:/test.mp3");
    settings.Save();

    DWORD attr = ::GetFileAttributesW(path.c_str());
    CHECK(attr != INVALID_FILE_ATTRIBUTES);

    RemoveTestFile(path);
}

/// 测试2：读取刚保存的文件，所有字段与保存值一致
static void test_save_and_load_roundtrip()
{
    std::cout << "\n[test2] 保存再读取，字段值一致\n";
    std::wstring path = GetTestIniPath();
    RemoveTestFile(path);

    AppSettings saved(path);
    saved.SetHours(1);
    saved.SetMinutes(30);
    saved.SetSeconds(0);
    saved.SetMusicPath(L"C:/test.mp3");
    saved.Save();

    AppSettings loaded(path);
    loaded.Load();
    CHECK(loaded.GetHours()     == 1);
    CHECK(loaded.GetMinutes()   == 30);
    CHECK(loaded.GetSeconds()   == 0);
    CHECK(loaded.GetMusicPath() == L"C:/test.mp3");

    RemoveTestFile(path);
}

/// 测试3：文件不存在时读取，返回默认值 {0, 5, 0, ""}
static void test_load_missing_file_returns_defaults()
{
    std::cout << "\n[test3] 文件不存在时返回默认值\n";
    std::wstring path = GetTestIniPath();
    RemoveTestFile(path);

    AppSettings settings(path);
    settings.Load();
    CHECK(settings.GetHours()     == 0);
    CHECK(settings.GetMinutes()   == 5);
    CHECK(settings.GetSeconds()   == 0);
    CHECK(settings.GetMusicPath() == L"");
}

/// 测试4：保存空 musicPath，读取后为空字符串，不崩溃
static void test_save_empty_music_path()
{
    std::cout << "\n[test4] 空 musicPath 存读不崩溃\n";
    std::wstring path = GetTestIniPath();
    RemoveTestFile(path);

    AppSettings settings(path);
    settings.SetHours(0);
    settings.SetMinutes(5);
    settings.SetSeconds(0);
    settings.SetMusicPath(L"");
    settings.Save();

    AppSettings loaded(path);
    loaded.Load();
    CHECK(loaded.GetMusicPath() == L"");

    RemoveTestFile(path);
}

/// 测试5：连续保存两次不同的值，读取得到第二次的值
static void test_second_save_overwrites_first()
{
    std::cout << "\n[test5] 第二次保存覆盖第一次\n";
    std::wstring path = GetTestIniPath();
    RemoveTestFile(path);

    AppSettings first(path);
    first.SetHours(0);
    first.SetMinutes(3);
    first.SetSeconds(0);
    first.SetMusicPath(L"first.mp3");
    first.Save();

    AppSettings second(path);
    second.SetHours(0);
    second.SetMinutes(10);
    second.SetSeconds(30);
    second.SetMusicPath(L"second.mp3");
    second.Save();

    AppSettings loaded(path);
    loaded.Load();
    CHECK(loaded.GetMinutes()   == 10);
    CHECK(loaded.GetSeconds()   == 30);
    CHECK(loaded.GetMusicPath() == L"second.mp3");

    RemoveTestFile(path);
}

// ─────────────────────────────────────────────────────────────────────────────
// 主函数
// ─────────────────────────────────────────────────────────────────────────────

int main()
{
    spdlog::set_level(spdlog::level::off);

    test_save_creates_file();
    test_save_and_load_roundtrip();
    test_load_missing_file_returns_defaults();
    test_save_empty_music_path();
    test_second_save_overwrites_first();

    std::cout << "\n========================================\n";
    std::cout << "结果：" << g_passed << " 通过，" << g_failed << " 失败\n";
    std::cout << "========================================\n";
    return g_failed;
}
