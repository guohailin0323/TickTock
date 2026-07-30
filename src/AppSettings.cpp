/**
 * @file    AppSettings.cpp
 * @brief   应用配置读写实现，使用 Windows INI API。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "AppSettings.h"

#include <windows.h>
#include <shlobj.h>
#include "spdlog/spdlog.h"

namespace TickTock {

// INI 区段和键名常量，避免魔数字符串
static constexpr wchar_t INI_SECTION_TIMER[] = L"Timer";
static constexpr wchar_t INI_SECTION_AUDIO[] = L"Audio";
static constexpr wchar_t INI_KEY_HOURS[]     = L"Hours";
static constexpr wchar_t INI_KEY_MINUTES[]   = L"Minutes";
static constexpr wchar_t INI_KEY_SECONDS[]   = L"Seconds";
static constexpr wchar_t INI_KEY_MUSIC[]     = L"MusicPath";

// 默认值
static constexpr int DEFAULT_HOURS   = 0;
static constexpr int DEFAULT_MINUTES = 5;  ///< 默认 5 分钟
static constexpr int DEFAULT_SECONDS = 0;

// 读取 INI 字符串值的缓冲区大小
static constexpr DWORD INI_STRING_BUF_SIZE = 1024;

// 获取 %APPDATA%\TickTock\settings.ini 默认路径
static std::wstring GetDefaultIniPath()
{
    wchar_t appData[MAX_PATH] = {};
    if (FAILED(::SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData)))
    {
        spdlog::error("AppSettings: 获取 APPDATA 路径失败");
        return L"settings.ini";
    }
    return std::wstring(appData) + L"\\TickTock\\settings.ini";
}

AppSettings::AppSettings(const std::wstring &iniPath)
    : m_iniPath(iniPath.empty() ? GetDefaultIniPath() : iniPath)
    , m_hours(DEFAULT_HOURS)
    , m_minutes(DEFAULT_MINUTES)
    , m_seconds(DEFAULT_SECONDS)
{
    spdlog::debug("AppSettings 创建，INI 路径={}",
                  std::string(m_iniPath.begin(), m_iniPath.end()));
}

void AppSettings::Load()
{
    // 文件不存在时 GetPrivateProfileInt 直接返回默认值，不报错
    m_hours   = static_cast<int>(::GetPrivateProfileIntW(
                    INI_SECTION_TIMER, INI_KEY_HOURS, DEFAULT_HOURS, m_iniPath.c_str()));
    m_minutes = static_cast<int>(::GetPrivateProfileIntW(
                    INI_SECTION_TIMER, INI_KEY_MINUTES, DEFAULT_MINUTES, m_iniPath.c_str()));
    m_seconds = static_cast<int>(::GetPrivateProfileIntW(
                    INI_SECTION_TIMER, INI_KEY_SECONDS, DEFAULT_SECONDS, m_iniPath.c_str()));

    wchar_t buf[INI_STRING_BUF_SIZE] = {};
    ::GetPrivateProfileStringW(INI_SECTION_AUDIO, INI_KEY_MUSIC, L"",
                               buf, INI_STRING_BUF_SIZE, m_iniPath.c_str());
    m_musicPath = buf;

    spdlog::info("AppSettings 加载：{:02d}:{:02d}:{:02d}，音乐={}",
                 m_hours, m_minutes, m_seconds,
                 std::string(m_musicPath.begin(), m_musicPath.end()));
}

void AppSettings::Save() const
{
    EnsureDirectoryExists_();

    // 将整数转为字符串写入，避免直接用 WritePrivateProfileInt（不存在此 API）
    auto WriteInt = [&](const wchar_t *section, const wchar_t *key, int value)
    {
        wchar_t buf[32] = {};
        ::_itow_s(value, buf, 32, 10);
        ::WritePrivateProfileStringW(section, key, buf, m_iniPath.c_str());
    };

    WriteInt(INI_SECTION_TIMER, INI_KEY_HOURS,   m_hours);
    WriteInt(INI_SECTION_TIMER, INI_KEY_MINUTES, m_minutes);
    WriteInt(INI_SECTION_TIMER, INI_KEY_SECONDS, m_seconds);
    ::WritePrivateProfileStringW(INI_SECTION_AUDIO, INI_KEY_MUSIC,
                                 m_musicPath.c_str(), m_iniPath.c_str());

    spdlog::info("AppSettings 保存：{:02d}:{:02d}:{:02d}，音乐={}",
                 m_hours, m_minutes, m_seconds,
                 std::string(m_musicPath.begin(), m_musicPath.end()));
}

void AppSettings::EnsureDirectoryExists_() const
{
    // 提取目录部分
    std::wstring dir = m_iniPath;
    size_t slash = dir.rfind(L'\\');
    if (slash == std::wstring::npos)
        return;

    dir = dir.substr(0, slash);
    if (::GetFileAttributesW(dir.c_str()) != INVALID_FILE_ATTRIBUTES)
        return;

    // 目录不存在，递归创建
    ::SHCreateDirectoryExW(NULL, dir.c_str(), NULL);
    spdlog::debug("AppSettings: 创建目录 {}",
                  std::string(dir.begin(), dir.end()));
}

void AppSettings::SetHours(int hours)     { m_hours     = hours; }
void AppSettings::SetMinutes(int minutes) { m_minutes   = minutes; }
void AppSettings::SetSeconds(int seconds) { m_seconds   = seconds; }
void AppSettings::SetMusicPath(const std::wstring &path) { m_musicPath = path; }

int                  AppSettings::GetHours()     const { return m_hours; }
int                  AppSettings::GetMinutes()   const { return m_minutes; }
int                  AppSettings::GetSeconds()   const { return m_seconds; }
const std::wstring & AppSettings::GetMusicPath() const { return m_musicPath; }

} // namespace TickTock
