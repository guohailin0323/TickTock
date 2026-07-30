/**
 * @file    AudioPlayer.cpp
 * @brief   基于 Windows MCI 的音频播放实现。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "AudioPlayer.h"

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "spdlog/spdlog.h"

namespace TickTock {

// MCI 设备别名，mciSendString 命令中用于标识当前打开的设备
static constexpr wchar_t MCI_ALIAS[] = L"TickTockAudio";

// MCI 命令缓冲区大小
static constexpr int MCI_CMD_BUF_SIZE  = 512;
// MCI 错误信息缓冲区大小
static constexpr int MCI_ERR_BUF_SIZE  = 256;

AudioPlayer::AudioPlayer()
    : m_isLoaded(false)
    , m_isPlaying(false)
{
    spdlog::debug("AudioPlayer 创建");
}

AudioPlayer::~AudioPlayer()
{
    Close_();
    spdlog::debug("AudioPlayer 销毁");
}

bool AudioPlayer::LoadFile(const std::wstring &filePath)
{
    if (filePath.empty())
    {
        spdlog::warn("AudioPlayer::LoadFile 路径为空，忽略");
        return false;
    }

    // 检查文件是否存在
    DWORD attr = ::GetFileAttributesW(filePath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        spdlog::warn("AudioPlayer::LoadFile 文件不存在: {}",
                     std::string(filePath.begin(), filePath.end()));
        return false;
    }

    // 先关闭之前的设备
    Close_();

    // 构造 MCI open 命令：open "路径" type mpegvideo alias TickTockAudio
    wchar_t cmd[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(cmd, MCI_CMD_BUF_SIZE,
                 L"open \"%s\" alias %s",
                 filePath.c_str(), MCI_ALIAS);

    wchar_t errBuf[MCI_ERR_BUF_SIZE] = {};
    MCIERROR err = ::mciSendStringW(cmd, nullptr, 0, nullptr);
    if (err != 0)
    {
        ::mciGetErrorStringW(err, errBuf, MCI_ERR_BUF_SIZE);
        spdlog::error("AudioPlayer::LoadFile MCI open 失败: {} ({})",
                      std::string(errBuf, errBuf + ::wcslen(errBuf)),
                      static_cast<unsigned long>(err));
        return false;
    }

    m_filePath  = filePath;
    m_isLoaded  = true;
    m_isPlaying = false;
    spdlog::info("AudioPlayer::LoadFile 加载成功: {}",
                 std::string(filePath.begin(), filePath.end()));
    return true;
}

void AudioPlayer::Play()
{
    if (!m_isLoaded)
    {
        spdlog::debug("AudioPlayer::Play 未加载文件，忽略");
        return;
    }

    // 先回到起点再播放，保证每次从头开始
    wchar_t seek[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(seek, MCI_CMD_BUF_SIZE, L"seek %s to start", MCI_ALIAS);
    ::mciSendStringW(seek, nullptr, 0, nullptr);

    wchar_t cmd[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(cmd, MCI_CMD_BUF_SIZE, L"play %s", MCI_ALIAS);

    wchar_t errBuf[MCI_ERR_BUF_SIZE] = {};
    MCIERROR err = ::mciSendStringW(cmd, nullptr, 0, nullptr);
    if (err != 0)
    {
        ::mciGetErrorStringW(err, errBuf, MCI_ERR_BUF_SIZE);
        spdlog::error("AudioPlayer::Play MCI play 失败: {}",
                      std::string(errBuf, errBuf + ::wcslen(errBuf)));
        return;
    }

    m_isPlaying = true;
    spdlog::info("AudioPlayer::Play 开始播放: {}",
                 std::string(m_filePath.begin(), m_filePath.end()));
}

void AudioPlayer::Stop()
{
    if (!m_isLoaded)
    {
        spdlog::debug("AudioPlayer::Stop 未加载文件，忽略");
        return;
    }

    wchar_t cmd[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(cmd, MCI_CMD_BUF_SIZE, L"stop %s", MCI_ALIAS);
    ::mciSendStringW(cmd, nullptr, 0, nullptr);

    m_isPlaying = false;
    spdlog::info("AudioPlayer::Stop 已停止");
}

bool AudioPlayer::IsLoaded() const
{
    return m_isLoaded;
}

bool AudioPlayer::IsPlaying() const
{
    if (!m_isLoaded)
        return false;

    // 向 MCI 查询实际播放状态，比内部 m_isPlaying 标志可靠（可检测自然播完）
    wchar_t statusBuf[64]         = {};
    wchar_t cmd[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(cmd, MCI_CMD_BUF_SIZE, L"status %s mode", MCI_ALIAS);

    MCIERROR err = ::mciSendStringW(cmd, statusBuf, 64, nullptr);
    if (err != 0)
        return false;

    return ::wcscmp(statusBuf, L"playing") == 0;
}

void AudioPlayer::Close_()
{
    if (!m_isLoaded)
        return;

    wchar_t cmd[MCI_CMD_BUF_SIZE] = {};
    ::swprintf_s(cmd, MCI_CMD_BUF_SIZE, L"close %s", MCI_ALIAS);
    ::mciSendStringW(cmd, nullptr, 0, nullptr);

    m_isLoaded  = false;
    m_isPlaying = false;
    spdlog::debug("AudioPlayer::Close_ MCI 设备已关闭");
}

} // namespace TickTock
