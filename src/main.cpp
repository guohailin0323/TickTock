/**
 * @file    main.cpp
 * @brief   TickTock 应用程序入口（WinMain）。
 *          初始化 GDI+、spdlog、公共控件，创建主窗口，进入消息循环。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "stdafx.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "MainWindow.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int /*nCmdShow*/)
{
    // 初始化日志（控制台输出，调试期使用）
    auto logger = spdlog::stdout_color_mt("TickTock");
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("TickTock 启动");

    // 初始化公共控件（EDIT 等系统控件需要）
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_STANDARD_CLASSES;
    ::InitCommonControlsEx(&icc);

    // 初始化 GDI+（自绘按钮、背景、时间文字均使用 GDI+）
    Gdiplus::GdiplusStartupInput gsi;
    ULONG_PTR gpToken = 0;
    Gdiplus::GdiplusStartup(&gpToken, &gsi, nullptr);

    int ret = 0;
    {
        TickTock::MainWindow wnd;
        if (!wnd.Create(hInstance))
        {
            spdlog::error("主窗口创建失败");
            Gdiplus::GdiplusShutdown(gpToken);
            return 1;
        }
        ret = wnd.ShowAndRun();
    }

    Gdiplus::GdiplusShutdown(gpToken);
    spdlog::info("TickTock 退出，返回码={}", ret);
    return ret;
}
