/**
 * @file    AudioPlayer.h
 * @brief   音频播放器：基于 Windows MCI 接口，支持 WAV / MP3 / WMA，
 *          不依赖任何第三方音频库，链接 winmm.lib 即可。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

#include <string>

namespace TickTock {

/**
 * @class   AudioPlayer
 * @brief   封装 MCI（mciSendString）实现音频加载、播放、停止。
 *
 *          典型用法：
 *          @code
 *          AudioPlayer player;
 *          if (player.LoadFile(L"C:\\alarm.mp3"))
 *              player.Play();
 *          // 倒计时中...
 *          player.Stop();
 *          @endcode
 *
 *          线程安全性：所有方法应在同一线程（UI 线程）调用。
 */
class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    /**
     * @brief   加载音频文件。加载成功后可调 Play()。
     *          若之前已加载过文件，会先关闭旧的再打开新的。
     * @param   filePath  音频文件完整路径（支持 .wav / .mp3 / .wma）
     * @return  true 表示加载成功；false 表示路径为空或文件无法打开
     */
    bool LoadFile(const std::wstring &filePath);

    /**
     * @brief   播放已加载的音频文件（从头开始）。
     *          未加载文件时调用无效果，不崩溃。
     */
    void Play();

    /**
     * @brief   停止正在播放的音频。
     *          未播放或未加载时调用无效果，不崩溃。
     */
    void Stop();

    /**
     * @brief   查询当前是否已加载了音频文件。
     */
    bool IsLoaded() const;

    /**
     * @brief   实时查询 MCI 设备是否仍在播放。
     *          比内部 m_isPlaying 标志更准确，可检测自然播完的情况。
     * @return  true 表示 MCI 汇报的模式为 "playing"
     */
    bool IsPlaying() const;

private:
    /**
     * @brief   关闭并释放当前 MCI 设备（若已打开）。
     */
    void Close_();

    std::wstring m_filePath;   ///< 当前加载的文件路径，空字符串表示未加载
    bool         m_isLoaded;   ///< 文件是否已成功通过 MCI 打开
    bool         m_isPlaying;  ///< 当前是否正在播放
};

} // namespace TickTock
