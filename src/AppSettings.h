/**
 * @file    AppSettings.h
 * @brief   应用配置读写：把上次设置的倒计时时长和音乐文件路径持久化到 INI 文件。
 *          存储位置由构造函数指定，默认为 %APPDATA%\TickTock\settings.ini。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

#include <string>

namespace TickTock {

/**
 * @class   AppSettings
 * @brief   封装 INI 文件读写，保存倒计时时长和音乐路径。
 *
 *          典型用法：
 *          @code
 *          AppSettings s;          // 使用默认路径
 *          s.Load();               // 读取上次配置
 *          s.SetMinutes(5);
 *          s.Save();               // 写回磁盘
 *          @endcode
 */
class AppSettings
{
public:
    /**
     * @brief   构造函数。
     * @param   iniPath  INI 文件完整路径；空字符串时使用
     *                   %APPDATA%\TickTock\settings.ini
     */
    explicit AppSettings(const std::wstring &iniPath = L"");

    /**
     * @brief   从 INI 文件加载配置。文件不存在时各字段保持默认值。
     */
    void Load();

    /**
     * @brief   将当前字段值写入 INI 文件。目录不存在时自动创建。
     */
    void Save() const;

    /** @brief 设置小时数，取值范围 [0, 23] */
    void SetHours(int hours);
    /** @brief 设置分钟数，取值范围 [0, 59] */
    void SetMinutes(int minutes);
    /** @brief 设置秒数，取值范围 [0, 59] */
    void SetSeconds(int seconds);
    /** @brief 设置音乐文件完整路径，空字符串表示未选择 */
    void SetMusicPath(const std::wstring &path);

    /** @brief 获取小时数，默认 0 */
    int GetHours()    const;
    /** @brief 获取分钟数，默认 5 */
    int GetMinutes()  const;
    /** @brief 获取秒数，默认 0 */
    int GetSeconds()  const;
    /** @brief 获取音乐文件路径，默认空字符串 */
    const std::wstring &GetMusicPath() const;

private:
    /**
     * @brief   若 INI 文件所在目录不存在则创建（递归）。
     */
    void EnsureDirectoryExists_() const;

    std::wstring m_iniPath;    ///< INI 文件完整路径
    int          m_hours;      ///< 倒计时小时数，默认 0
    int          m_minutes;    ///< 倒计时分钟数，默认 5
    int          m_seconds;    ///< 倒计时秒数，默认 0
    std::wstring m_musicPath;  ///< 音乐文件路径，默认空
};

} // namespace TickTock
