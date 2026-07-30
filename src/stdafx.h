/**
 * @file    stdafx.h
 * @brief   TickTock 主程序公共头文件。包含 Win32 / GDI+ 基础头，供所有需要
 *          Windows API 的 .cpp 文件包含。纯逻辑模块（TimerEngine / AppSettings
 *          / AudioPlayer）不需要此头，直接包含各自所需的最小头文件即可。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

#define _CRT_SECURE_NO_WARNINGS

// Windows 最低版本：Win7（0x0601）
#define WINVER       0x0601
#define _WIN32_WINNT 0x0601

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX        // 禁止 windows.h 定义 min/max 宏，避免与 std::min/std::max 冲突
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <mmsystem.h>   // PlaySound / SND_ALIAS / SND_ASYNC（winmm.lib 已在 CMakeLists 中链接）

#include <objidl.h>   // IUnknown 等 COM 基础接口（WIN32_LEAN_AND_MEAN 会跳过，需手动补）
#include <gdiplus.h>

#include <string>
#include <memory>
#include <vector>
