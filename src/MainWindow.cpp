/**
 * @file    MainWindow.cpp
 * @brief   TickTock 主窗口实现：控件创建、布局计算、消息分派、倒计时驱动。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "MainWindow.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <commdlg.h>

#pragma comment(lib, "Comdlg32.lib")

namespace TickTock {

// ─── WndProc_ / HandleMessage_ ──────────────────────────────────────────

LRESULT CALLBACK MainWindow::WndProc_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    MainWindow *self = nullptr;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW *cs = reinterpret_cast<CREATESTRUCTW *>(lp);
        self = reinterpret_cast<MainWindow *>(cs->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->HandleMessage_(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT CALLBACK MainWindow::ButtonSubclassProc_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                                   UINT_PTR /*subclassId*/, DWORD_PTR refData)
{
    MainWindow *self = reinterpret_cast<MainWindow *>(refData);

    switch (msg)
    {
    case WM_MOUSEMOVE:
        if (self->m_hoveredBtn != hwnd)
        {
            self->m_hoveredBtn = hwnd;

            // 注册鼠标离开通知：鼠标移出按钮时收到 WM_MOUSELEAVE
            TRACKMOUSEEVENT tme = {};
            tme.cbSize    = sizeof(tme);
            tme.dwFlags   = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);

            InvalidateRect(hwnd, nullptr, FALSE);
        }
        break;

    case WM_MOUSELEAVE:
        if (self->m_hoveredBtn == hwnd)
        {
            self->m_hoveredBtn = nullptr;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        break;
    }

    return DefSubclassProc(hwnd, msg, wp, lp);
}

LRESULT MainWindow::HandleMessage_(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        OnCreate_();
        return 0;

    case WM_PAINT:
        OnPaint_();
        return 0;

    case WM_ERASEBKGND:
        // WM_PAINT 统一处理背景，避免闪烁
        return TRUE;

    case WM_TIMER:
        if (static_cast<UINT_PTR>(wp) == static_cast<UINT_PTR>(ID_TIMER))
            OnTimer_();
        return 0;

    case WM_SIZE:
        OnSize_(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_COMMAND:
        OnCommand_(LOWORD(wp), HIWORD(wp));
        return 0;

    case WM_DRAWITEM:
        OnDrawItem_(reinterpret_cast<DRAWITEMSTRUCT *>(lp));
        return TRUE;

    case WM_GETMINMAXINFO:
        OnGetMinMaxInfo_(reinterpret_cast<MINMAXINFO *>(lp));
        return 0;

    case WM_CTLCOLOREDIT:
    {
        // 为所有 EDIT 控件设置奶油色背景和深棕文字
        HDC hdc = reinterpret_cast<HDC>(wp);
        SetBkColor(hdc,   RGB(0xFB, 0xF3, 0xE8)); // #fbf3e8
        SetTextColor(hdc, RGB(0x3C, 0x33, 0x2B)); // #3c332b
        return reinterpret_cast<LRESULT>(m_editBrush);
    }

    case WM_DESTROY:
        OnDestroy_();
        return 0;
    }

    return DefWindowProcW(m_hwnd, msg, wp, lp);
}

// ─── Create / ShowAndRun ─────────────────────────────────────────────────

bool MainWindow::Create(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc_;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // WM_PAINT 负责绘制，不使用系统背景刷
    wc.lpszClassName = WINDOW_CLASS;
    // 从 app.rc 资源加载自定义图标（ID=1，含 16/24/32/48/64/128px 全部尺寸）
    wc.hIcon   = LoadIcon(hInstance, MAKEINTRESOURCEW(1));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCEW(1));

    if (!RegisterClassExW(&wc))
    {
        spdlog::error("MainWindow::Create: 注册窗口类失败，错误码={}", GetLastError());
        return false;
    }

    // 根据期望客户区尺寸推算含标题栏的完整窗口尺寸
    RECT rc = {0, 0, DEFAULT_CLIENT_W, DEFAULT_CLIENT_H};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS,
        L"倒计时",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, this);

    if (!m_hwnd)
    {
        spdlog::error("MainWindow::Create: 创建窗口失败，错误码={}", GetLastError());
        return false;
    }

    spdlog::info("MainWindow::Create: 窗口创建成功，HWND={:p}", static_cast<void *>(m_hwnd));
    return true;
}

int MainWindow::ShowAndRun()
{
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    UpdateWindow(m_hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

// ─── OnCreate_ ──────────────────────────────────────────────────────────

void MainWindow::OnCreate_()
{
    spdlog::info("MainWindow::OnCreate_: 窗口初始化开始");

    m_settings.Load();
    m_hours   = m_settings.GetHours();
    m_minutes = m_settings.GetMinutes();
    m_seconds = m_settings.GetSeconds();
    spdlog::info("MainWindow::OnCreate_: 加载配置 H={} M={} S={}", m_hours, m_minutes, m_seconds);

    const std::wstring &musicPath = m_settings.GetMusicPath();
    if (!musicPath.empty())
    {
        if (m_audioPlayer.LoadFile(musicPath))
            spdlog::info("MainWindow::OnCreate_: 音乐文件预加载成功");
        else
            spdlog::warn("MainWindow::OnCreate_: 音乐文件预加载失败，将使用系统提示音");
    }

    CreateControls_();

    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    RepositionControls_(rcClient.right, rcClient.bottom);

    WriteEditValues_();
    UpdateControlsVisibility_();

    spdlog::info("MainWindow::OnCreate_: 窗口初始化完成");
}

// ─── CreateControls_ ────────────────────────────────────────────────────

void MainWindow::CreateControls_()
{
    // 创建 EDIT 字体（Segoe UI Bold 24pt，与调节模式数字框视觉大小接近）
    HDC screenDC   = GetDC(nullptr);
    int fontHeight = -MulDiv(24, GetDeviceCaps(screenDC, LOGPIXELSY), 72);
    ReleaseDC(nullptr, screenDC);

    m_editFont = CreateFontW(
        fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    m_editBrush = CreateSolidBrush(RGB(0xFB, 0xF3, 0xE8)); // #fbf3e8

    // EDIT 控件（输入模式可见，初始为隐藏）
    // 不用 WS_EX_CLIENTEDGE：去掉系统 3D 凹陷边框，改由 OnPaint_ 中 DrawInputBox 绘制圆角边框
    DWORD editStyle = WS_CHILD | WS_TABSTOP | ES_CENTER | ES_NUMBER;
    m_editHours   = CreateWindowExW(0, L"EDIT", L"0",
        editStyle, 0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_EDIT_HOURS,   m_hInstance, nullptr);
    m_editMinutes = CreateWindowExW(0, L"EDIT", L"5",
        editStyle, 0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_EDIT_MINUTES, m_hInstance, nullptr);
    m_editSeconds = CreateWindowExW(0, L"EDIT", L"0",
        editStyle, 0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_EDIT_SECONDS, m_hInstance, nullptr);

    SendMessage(m_editHours,   WM_SETFONT, (WPARAM)m_editFont, FALSE);
    SendMessage(m_editMinutes, WM_SETFONT, (WPARAM)m_editFont, FALSE);
    SendMessage(m_editSeconds, WM_SETFONT, (WPARAM)m_editFont, FALSE);

    // owner-draw 按钮（初始可见）
    DWORD btnStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP;

    m_btnMinusH = CreateWindowExW(0, L"BUTTON", L"−", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_MINUS_H, m_hInstance, nullptr);
    m_btnPlusH  = CreateWindowExW(0, L"BUTTON", L"+", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PLUS_H,  m_hInstance, nullptr);
    m_btnMinusM = CreateWindowExW(0, L"BUTTON", L"−", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_MINUS_M, m_hInstance, nullptr);
    m_btnPlusM  = CreateWindowExW(0, L"BUTTON", L"+", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PLUS_M,  m_hInstance, nullptr);
    m_btnMinusS = CreateWindowExW(0, L"BUTTON", L"−", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_MINUS_S, m_hInstance, nullptr);
    m_btnPlusS  = CreateWindowExW(0, L"BUTTON", L"+", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PLUS_S,  m_hInstance, nullptr);

    m_btnTabAdjust = CreateWindowExW(0, L"BUTTON", L"调节", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_TAB_ADJUST, m_hInstance, nullptr);
    m_btnTabInput  = CreateWindowExW(0, L"BUTTON", L"输入", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_TAB_INPUT,  m_hInstance, nullptr);

    m_btnPreset1  = CreateWindowExW(0, L"BUTTON", L"1分", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PRESET_1,  m_hInstance, nullptr);
    m_btnPreset5  = CreateWindowExW(0, L"BUTTON", L"5分", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PRESET_5,  m_hInstance, nullptr);
    m_btnPreset10 = CreateWindowExW(0, L"BUTTON", L"10分", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PRESET_10, m_hInstance, nullptr);

    m_btnStart = CreateWindowExW(0, L"BUTTON", L"开始", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_START, m_hInstance, nullptr);
    m_btnReset = CreateWindowExW(0, L"BUTTON", L"重置", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_RESET, m_hInstance, nullptr);

    m_btnPreview = CreateWindowExW(0, L"BUTTON", L"试听", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_PREVIEW, m_hInstance, nullptr);
    m_btnMusic   = CreateWindowExW(0, L"BUTTON", L"选择音乐", btnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_MUSIC, m_hInstance, nullptr);

    // 结束界面"停止"按钮（初始隐藏，倒计时结束时才显示）
    DWORD stopBtnStyle = WS_CHILD | BS_OWNERDRAW | WS_TABSTOP;
    m_btnStop = CreateWindowExW(0, L"BUTTON", L"停止", stopBtnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_STOP, m_hInstance, nullptr);

    // 倒计时模式"回到主界面"按钮（初始隐藏，Running/Paused 时才显示）
    m_btnBackToMain = CreateWindowExW(0, L"BUTTON", L"回到主界面", stopBtnStyle,
        0, 0, 1, 1, m_hwnd, (HMENU)(INT_PTR)ID_BTN_BACK_TO_MAIN, m_hInstance, nullptr);

    // 为所有 owner-draw 按钮注册子类化过程，以捕获鼠标悬停事件
    // subclassId 统一用 1（每个 HWND 不同，ID 可以相同）
    HWND allBtns[] = {
        m_btnMinusH, m_btnPlusH, m_btnMinusM, m_btnPlusM, m_btnMinusS, m_btnPlusS,
        m_btnTabAdjust, m_btnTabInput,
        m_btnPreset1, m_btnPreset5, m_btnPreset10,
        m_btnStart, m_btnReset,
        m_btnPreview, m_btnMusic,
        m_btnStop, m_btnBackToMain
    };
    for (HWND h : allBtns)
        SetWindowSubclass(h, ButtonSubclassProc_, 1, reinterpret_cast<DWORD_PTR>(this));
}

// ─── RepositionControls_ ────────────────────────────────────────────────

void MainWindow::RepositionControls_(int cx, int cy)
{
    m_layout.footerY   = cy - FOOTER_H;
    int ch             = m_layout.footerY;

    m_layout.timeAreaH = std::max(90, ch * 20 / 100);
    m_layout.timeAreaY = ch * 4 / 100;

    m_layout.tabY = m_layout.timeAreaY + m_layout.timeAreaH + 16;

    m_layout.labelH = 22;
    m_layout.labelY = m_layout.tabY + TAB_BTN_H + 16;

    m_layout.ctrlY  = m_layout.labelY + m_layout.labelH + 4;

    m_layout.presetY = m_layout.ctrlY + GROUP_H + 18;

    // 开始/重置行：剩余空间三等分，取 1/3 处
    int remaining  = m_layout.footerY - (m_layout.presetY + PRESET_BTN_H);
    m_layout.btnY  = m_layout.presetY + PRESET_BTN_H + remaining / 3;

    // 三列组 X（均分水平空间）
    int margin = (cx - 3 * GROUP_W) / 4;
    if (margin < 4)
        margin = 4;

    for (int i = 0; i < 3; ++i)
    {
        m_layout.groupX[i]       = margin + i * (GROUP_W + margin);
        m_layout.groupCenterX[i] = m_layout.groupX[i] + GROUP_W / 2;
        m_layout.numBoxX[i]      = m_layout.groupX[i] + MINUS_PLUS_W + NUM_BOX_INNER_GAP;
    }

    // ── 标签按钮（水平居中）
    int tabTotalW = TAB_BTN_W * 2 + TAB_BTN_GAP;
    int tabStartX = (cx - tabTotalW) / 2;
    MoveWindow(m_btnTabAdjust, tabStartX,                           m_layout.tabY, TAB_BTN_W, TAB_BTN_H, TRUE);
    MoveWindow(m_btnTabInput,  tabStartX + TAB_BTN_W + TAB_BTN_GAP, m_layout.tabY, TAB_BTN_W, TAB_BTN_H, TRUE);

    // ── [−][+] 按钮（垂直居中于控件行）
    int plusXOff  = MINUS_PLUS_W + NUM_BOX_INNER_GAP + NUM_BOX_W + NUM_BOX_INNER_GAP;
    int btnVOff   = (GROUP_H - MINUS_PLUS_H) / 2;
    HWND minusBtns[3] = {m_btnMinusH, m_btnMinusM, m_btnMinusS};
    HWND plusBtns[3]  = {m_btnPlusH,  m_btnPlusM,  m_btnPlusS};

    for (int i = 0; i < 3; ++i)
    {
        int gx = m_layout.groupX[i];
        int gy = m_layout.ctrlY + btnVOff;
        MoveWindow(minusBtns[i], gx,            gy, MINUS_PLUS_W, MINUS_PLUS_H, TRUE);
        MoveWindow(plusBtns[i],  gx + plusXOff, gy, MINUS_PLUS_W, MINUS_PLUS_H, TRUE);
    }

    // ── EDIT 控件（比 DrawInputBox 的组框四边各缩进 EDIT_INSET，使圆角边框可见）
    HWND edits[3] = {m_editHours, m_editMinutes, m_editSeconds};
    for (int i = 0; i < 3; ++i)
    {
        MoveWindow(edits[i],
                   m_layout.groupX[i] + EDIT_INSET,
                   m_layout.ctrlY     + EDIT_INSET,
                   GROUP_W - 2 * EDIT_INSET,
                   GROUP_H - 2 * EDIT_INSET,
                   TRUE);
    }

    // ── 快捷预设按钮（水平居中）
    int presetWidths[3] = {PRESET_1_W, PRESET_5_W, PRESET_10_W};
    HWND presetBtns[3]  = {m_btnPreset1, m_btnPreset5, m_btnPreset10};
    int  presetTotalW   = PRESET_1_W + PRESET_GAP + PRESET_5_W + PRESET_GAP + PRESET_10_W;
    int  px             = (cx - presetTotalW) / 2;
    for (int i = 0; i < 3; ++i)
    {
        MoveWindow(presetBtns[i], px, m_layout.presetY, presetWidths[i], PRESET_BTN_H, TRUE);
        px += presetWidths[i] + PRESET_GAP;
    }

    // ── 开始 / 重置（水平居中）
    int srTotalW = START_BTN_W + START_RESET_GAP + RESET_BTN_W;
    int startX   = (cx - srTotalW) / 2;
    MoveWindow(m_btnStart, startX,                          m_layout.btnY, START_BTN_W, BTN_ROW_H, TRUE);
    MoveWindow(m_btnReset, startX + START_BTN_W + START_RESET_GAP, m_layout.btnY, RESET_BTN_W, BTN_ROW_H, TRUE);

    // ── 结束界面"停止"按钮（圆圈底部区域，与 DrawFinishedContent 使用相同公式定位）
    {
        float circleCY = m_layout.footerY * 0.43f;
        float circleR  = std::min(cx * 0.30f, m_layout.footerY * 0.35f);
        m_layout.stopBtnX = (cx - STOP_BTN_W) / 2;
        m_layout.stopBtnY = static_cast<int>(circleCY + circleR * 0.38f
                                              - static_cast<float>(STOP_BTN_H) / 2.0f);
        MoveWindow(m_btnStop, m_layout.stopBtnX, m_layout.stopBtnY, STOP_BTN_W, STOP_BTN_H, TRUE);
    }

    // ── 底部按钮（右对齐，垂直居中于底部区域）
    int footerBtnY  = m_layout.footerY + (FOOTER_H - MUSIC_BTN_H) / 2;
    int musicBtnX   = cx - FOOTER_BTN_R - MUSIC_BTN_W;
    int previewBtnX = musicBtnX - FOOTER_BTN_GAP - PREVIEW_BTN_W;
    MoveWindow(m_btnPreview, previewBtnX, footerBtnY, PREVIEW_BTN_W, PREVIEW_BTN_H, TRUE);
    MoveWindow(m_btnMusic,   musicBtnX,   footerBtnY, MUSIC_BTN_W,   MUSIC_BTN_H,   TRUE);
}

// ─── UpdateControlsVisibility_ ──────────────────────────────────────────

void MainWindow::UpdateControlsVisibility_()
{
    TimerState state = m_timerEngine.GetState();

    if (state == TimerState::Finished)
    {
        // 结束界面：隐藏所有正常控件，只显示"停止"按钮
        ShowWindow(m_btnMinusH,    SW_HIDE);
        ShowWindow(m_btnPlusH,     SW_HIDE);
        ShowWindow(m_btnMinusM,    SW_HIDE);
        ShowWindow(m_btnPlusM,     SW_HIDE);
        ShowWindow(m_btnMinusS,    SW_HIDE);
        ShowWindow(m_btnPlusS,     SW_HIDE);
        ShowWindow(m_editHours,    SW_HIDE);
        ShowWindow(m_editMinutes,  SW_HIDE);
        ShowWindow(m_editSeconds,  SW_HIDE);
        ShowWindow(m_btnTabAdjust, SW_HIDE);
        ShowWindow(m_btnTabInput,  SW_HIDE);
        ShowWindow(m_btnPreset1,   SW_HIDE);
        ShowWindow(m_btnPreset5,   SW_HIDE);
        ShowWindow(m_btnPreset10,  SW_HIDE);
        ShowWindow(m_btnStart,      SW_HIDE);
        ShowWindow(m_btnReset,      SW_HIDE);
        ShowWindow(m_btnBackToMain, SW_HIDE);
        ShowWindow(m_btnStop,       SW_SHOW);
        EnableWindow(m_btnStop, TRUE);
        InvalidateRect(m_btnStop, nullptr, FALSE);
        return;
    }

    // ── 倒计时进行中（Running / Paused）：隐藏设置控件；底部试听/选音乐栏保留
    if (state == TimerState::Running || state == TimerState::Paused)
    {
        ShowWindow(m_btnMinusH,    SW_HIDE);
        ShowWindow(m_btnPlusH,     SW_HIDE);
        ShowWindow(m_btnMinusM,    SW_HIDE);
        ShowWindow(m_btnPlusM,     SW_HIDE);
        ShowWindow(m_btnMinusS,    SW_HIDE);
        ShowWindow(m_btnPlusS,     SW_HIDE);
        ShowWindow(m_editHours,    SW_HIDE);
        ShowWindow(m_editMinutes,  SW_HIDE);
        ShowWindow(m_editSeconds,  SW_HIDE);
        ShowWindow(m_btnTabAdjust, SW_HIDE);
        ShowWindow(m_btnTabInput,  SW_HIDE);
        ShowWindow(m_btnPreset1,   SW_HIDE);
        ShowWindow(m_btnPreset5,   SW_HIDE);
        ShowWindow(m_btnPreset10,  SW_HIDE);
        ShowWindow(m_btnReset,     SW_HIDE);
        ShowWindow(m_btnStop,      SW_HIDE);

        // 底部试听 / 选音乐按钮在倒计时模式下保留可见
        ShowWindow(m_btnPreview, SW_SHOW);
        ShowWindow(m_btnMusic,   SW_SHOW);
        EnableWindow(m_btnPreview, TRUE);
        EnableWindow(m_btnMusic,   TRUE);

        // 计算两个按钮（暂停/继续 + 回到主界面）并排居中的位置
        // 以内容区高度（footerY）的 70% 处为按钮顶边
        RECT rcForBtn = {};
        GetClientRect(m_hwnd, &rcForBtn);
        int cxW      = rcForBtn.right;
        int contentH = m_layout.footerY > 0 ? m_layout.footerY : (rcForBtn.bottom - FOOTER_H);
        int groupW    = COUNTDOWN_PAUSE_BTN_W + COUNTDOWN_BTNS_GAP + COUNTDOWN_BACK_BTN_W;
        int groupX    = (cxW - groupW) / 2;
        int btnY      = contentH * COUNTDOWN_BTN_Y_NUM / COUNTDOWN_BTN_Y_DEN;

        MoveWindow(m_btnStart,      groupX,
                   btnY, COUNTDOWN_PAUSE_BTN_W, COUNTDOWN_BTN_H, TRUE);
        MoveWindow(m_btnBackToMain, groupX + COUNTDOWN_PAUSE_BTN_W + COUNTDOWN_BTNS_GAP,
                   btnY, COUNTDOWN_BACK_BTN_W,  COUNTDOWN_BTN_H, TRUE);

        const wchar_t *pauseText = (state == TimerState::Running) ? L"暂停" : L"继续";
        SetWindowTextW(m_btnStart, pauseText);
        ShowWindow(m_btnStart,      SW_SHOW);
        ShowWindow(m_btnBackToMain, SW_SHOW);
        EnableWindow(m_btnStart,      TRUE);
        EnableWindow(m_btnBackToMain, TRUE);
        InvalidateRect(m_btnStart,      nullptr, FALSE);
        InvalidateRect(m_btnBackToMain, nullptr, FALSE);
        spdlog::debug("MainWindow::UpdateControlsVisibility_: 倒计时模式，显示\"{}\" + \"回到主界面\"",
                      state == TimerState::Running ? "暂停" : "继续");
        return;
    }

    // ── Idle 状态：恢复完整主界面，隐藏停止和回到主界面按钮
    ShowWindow(m_btnStop,       SW_HIDE);
    ShowWindow(m_btnBackToMain, SW_HIDE);

    // 将开始/重置按钮恢复到主界面正常位置（从倒计时模式返回时位置可能已被移动）
    {
        RECT rcClient = {};
        GetClientRect(m_hwnd, &rcClient);
        int cxW      = rcClient.right;
        int srTotalW = START_BTN_W + START_RESET_GAP + RESET_BTN_W;
        int startX   = (cxW - srTotalW) / 2;
        MoveWindow(m_btnStart, startX, m_layout.btnY, START_BTN_W, BTN_ROW_H, TRUE);
        MoveWindow(m_btnReset, startX + START_BTN_W + START_RESET_GAP,
                   m_layout.btnY, RESET_BTN_W, BTN_ROW_H, TRUE);
    }

    // 结束界面会隐藏这些控件，回到主界面时需显式恢复
    ShowWindow(m_btnTabAdjust, SW_SHOW);
    ShowWindow(m_btnTabInput,  SW_SHOW);
    ShowWindow(m_btnPreset1,   SW_SHOW);
    ShowWindow(m_btnPreset5,   SW_SHOW);
    ShowWindow(m_btnPreset10,  SW_SHOW);

    int adjustShow = (m_tabMode == TabMode::Adjust) ? SW_SHOW : SW_HIDE;
    ShowWindow(m_btnMinusH, adjustShow);
    ShowWindow(m_btnPlusH,  adjustShow);
    ShowWindow(m_btnMinusM, adjustShow);
    ShowWindow(m_btnPlusM,  adjustShow);
    ShowWindow(m_btnMinusS, adjustShow);
    ShowWindow(m_btnPlusS,  adjustShow);

    int inputShow = (m_tabMode == TabMode::Input) ? SW_SHOW : SW_HIDE;
    ShowWindow(m_editHours,   inputShow);
    ShowWindow(m_editMinutes, inputShow);
    ShowWindow(m_editSeconds, inputShow);

    ShowWindow(m_btnStart, SW_SHOW);
    ShowWindow(m_btnReset, SW_SHOW);

    // Idle 状态下所有时间设置控件均可用
    EnableWindow(m_btnMinusH,    TRUE);
    EnableWindow(m_btnPlusH,     TRUE);
    EnableWindow(m_btnMinusM,    TRUE);
    EnableWindow(m_btnPlusM,     TRUE);
    EnableWindow(m_btnMinusS,    TRUE);
    EnableWindow(m_btnPlusS,     TRUE);
    EnableWindow(m_editHours,    TRUE);
    EnableWindow(m_editMinutes,  TRUE);
    EnableWindow(m_editSeconds,  TRUE);
    EnableWindow(m_btnTabAdjust, TRUE);
    EnableWindow(m_btnTabInput,  TRUE);
    EnableWindow(m_btnPreset1,   TRUE);
    EnableWindow(m_btnPreset5,   TRUE);
    EnableWindow(m_btnPreset10,  TRUE);

    SetWindowTextW(m_btnStart, L"开始");

    // 强制重绘所有可见 owner-draw 按钮
    HWND allBtns[] = {
        m_btnStart, m_btnReset,
        m_btnTabAdjust, m_btnTabInput,
        m_btnPreset1, m_btnPreset5, m_btnPreset10,
        m_btnMinusH, m_btnPlusH,
        m_btnMinusM, m_btnPlusM,
        m_btnMinusS, m_btnPlusS,
        m_btnPreview, m_btnMusic
    };
    for (HWND h : allBtns)
        InvalidateRect(h, nullptr, FALSE);
}

// ─── AdjustTimeField_ ───────────────────────────────────────────────────

void MainWindow::AdjustTimeField_(int field, bool increment)
{
    int *ptr    = nullptr;
    int  maxVal = 59;

    if (field == 0)
    {
        ptr    = &m_hours;
        maxVal = 99;
    }
    else if (field == 1)
    {
        ptr    = &m_minutes;
    }
    else
    {
        ptr    = &m_seconds;
    }

    if (increment)
    {
        if (*ptr < maxVal)
            (*ptr)++;
    }
    else
    {
        if (*ptr > 0)
            (*ptr)--;
    }

    spdlog::debug("MainWindow::AdjustTimeField_: 字段={} 新值={}", field, *ptr);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

// ─── ReadEditValues_ / WriteEditValues_ ─────────────────────────────────

void MainWindow::ReadEditValues_()
{
    int h = GetEditInt_(m_editHours);
    int m = GetEditInt_(m_editMinutes);
    int s = GetEditInt_(m_editSeconds);

    m_hours   = std::min(h, 99);
    m_minutes = std::min(m, 59);
    m_seconds = std::min(s, 59);

    spdlog::debug("MainWindow::ReadEditValues_: H={} M={} S={}", m_hours, m_minutes, m_seconds);
}

void MainWindow::WriteEditValues_()
{
    wchar_t buf[8] = {};

    swprintf_s(buf, 8, L"%d", m_hours);
    SetWindowTextW(m_editHours, buf);

    swprintf_s(buf, 8, L"%d", m_minutes);
    SetWindowTextW(m_editMinutes, buf);

    swprintf_s(buf, 8, L"%d", m_seconds);
    SetWindowTextW(m_editSeconds, buf);
}

int MainWindow::GetEditInt_(HWND editHwnd) const
{
    wchar_t buf[16] = {};
    GetWindowTextW(editHwnd, buf, 16);
    if (buf[0] == L'\0')
        return 0;
    return static_cast<int>(wcstol(buf, nullptr, 10));
}

// ─── 倒计时逻辑 ──────────────────────────────────────────────────────────

void MainWindow::OnStartClick_()
{
    TimerState state = m_timerEngine.GetState();

    if (state == TimerState::Idle || state == TimerState::Finished)
    {
        if (m_tabMode == TabMode::Input)
            ReadEditValues_();

        int total = m_hours * 3600 + m_minutes * 60 + m_seconds;
        if (total <= 0)
        {
            spdlog::warn("MainWindow::OnStartClick_: 时长为 0，忽略开始");
            return;
        }

        m_timerEngine.SetDuration(m_hours, m_minutes, m_seconds);
        m_timerEngine.Start();
        m_lastDisplayedSeconds = -1; // 强制下一次 OnTimer_ 立即刷新
        spdlog::info("MainWindow::OnStartClick_: 倒计时开始，总秒数={}", total);

        if (!m_timerRunning)
        {
            SetTimer(m_hwnd, ID_TIMER, TIMER_INTERVAL, nullptr);
            m_timerRunning = true;
        }
    }
    else if (state == TimerState::Running)
    {
        m_timerEngine.Pause();
        StopPreviewIfPlaying_(); // 暂停时一并停止正在试听的音乐
        spdlog::info("MainWindow::OnStartClick_: 倒计时暂停");
    }
    else if (state == TimerState::Paused)
    {
        m_timerEngine.Start();
        spdlog::info("MainWindow::OnStartClick_: 倒计时继续");
    }

    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnResetClick_()
{
    m_timerEngine.Reset();
    m_lastDisplayedSeconds = -1; // 强制立即刷新显示
    m_animPhase            = 0.0f;
    m_finishedMusicStopped = false;
    spdlog::info("MainWindow::OnResetClick_: 重置倒计时");

    if (m_timerRunning)
    {
        KillTimer(m_hwnd, ID_TIMER);
        m_timerRunning = false;
    }

    m_audioPlayer.Stop();
    PlaySound(nullptr, nullptr, 0); // 确保停止系统提示音的 SND_LOOP 循环

    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnStopClick_()
{
    // 停止音乐
    if (m_audioPlayer.IsLoaded())
        m_audioPlayer.Stop();
    else
        PlaySound(nullptr, nullptr, 0); // 停止 SND_LOOP 系统提示音循环

    // 停止动画定时器
    if (m_timerRunning)
    {
        KillTimer(m_hwnd, ID_TIMER);
        m_timerRunning = false;
    }

    // 重置计时引擎
    m_timerEngine.Reset();

    // 恢复默认：5 分钟 + 调节模式
    m_hours   = 0;
    m_minutes = 5;
    m_seconds = 0;
    m_tabMode = TabMode::Adjust;

    m_lastDisplayedSeconds = -1;
    m_animPhase            = 0.0f;
    m_finishedMusicStopped = false;

    spdlog::info("MainWindow::OnStopClick_: 停止提醒，回到主界面，默认 5 分钟");

    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnBackToMainClick_()
{
    // 停止正在试听的音乐，避免返回主界面后仍有音乐在播
    StopPreviewIfPlaying_();

    // 停止倒计时定时器
    if (m_timerRunning)
    {
        KillTimer(m_hwnd, ID_TIMER);
        m_timerRunning = false;
    }

    // 重置倒计时状态，时间回到用户原先设置的值
    m_timerEngine.Reset();
    m_lastDisplayedSeconds = -1;
    m_animPhase            = 0.0f;
    m_finishedMusicStopped = false;

    spdlog::info("MainWindow::OnBackToMainClick_: 回到主界面，倒计时已重置");

    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::StopPreviewIfPlaying_()
{
    if (!m_isPreviewing)
        return;

    if (m_audioPlayer.IsLoaded())
        m_audioPlayer.Stop();
    else
        PlaySound(nullptr, nullptr, 0);

    m_isPreviewing = false;
    SetWindowTextW(m_btnPreview, L"试听");
    InvalidateRect(m_btnPreview, nullptr, FALSE);
    spdlog::info("MainWindow::StopPreviewIfPlaying_: 试听音乐已停止");
}

void MainWindow::OnCountdownFinished_()
{
    spdlog::info("MainWindow::OnCountdownFinished_: 倒计时结束，触发提醒");

    // 不停定时器：复用为结束界面的动画驱动 + 音乐循环检测
    m_animPhase            = 0.0f;
    m_finishedMusicStopped = false;

    const std::wstring &musicPath = m_settings.GetMusicPath();
    if (!musicPath.empty() && m_audioPlayer.IsLoaded())
    {
        m_audioPlayer.Play();
        spdlog::info("MainWindow::OnCountdownFinished_: 开始循环播放音乐文件");
    }
    else
    {
        // 无自定义文件：系统提示音循环播放，SND_LOOP 持续响铃直到 PlaySound(nullptr) 停止
        PlaySound(L"SystemExclamation", nullptr, SND_ALIAS | SND_ASYNC | SND_LOOP);
        spdlog::info("MainWindow::OnCountdownFinished_: 开始循环播放系统提示音");
    }

    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

// ─── 音乐 ────────────────────────────────────────────────────────────────

void MainWindow::SelectMusicFile_()
{
    wchar_t filePath[MAX_PATH] = {};

    OPENFILENAMEW ofn = {};
    ofn.lStructSize   = sizeof(ofn);
    ofn.hwndOwner     = m_hwnd;
    ofn.lpstrFile     = filePath;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrFilter   = L"音频文件\0*.mp3;*.wav;*.wma;*.aac;*.flac\0所有文件\0*.*\0\0";
    ofn.Flags         = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle    = L"选择提醒音乐";

    if (!GetOpenFileNameW(&ofn))
    {
        spdlog::debug("MainWindow::SelectMusicFile_: 用户取消选择");
        return;
    }

    spdlog::info("MainWindow::SelectMusicFile_: 文件名长度={}字符", wcslen(filePath));

    m_settings.SetMusicPath(filePath);

    if (m_audioPlayer.LoadFile(filePath))
        spdlog::info("MainWindow::SelectMusicFile_: 音乐文件加载成功");
    else
        spdlog::warn("MainWindow::SelectMusicFile_: 音乐文件加载失败");

    m_settings.Save();
    InvalidateRect(m_hwnd, nullptr, FALSE); // 刷新底部文件名显示
}

void MainWindow::PreviewMusic_()
{
    if (m_isPreviewing)
    {
        // 正在试听 → 用户再次点击，停止播放
        if (m_audioPlayer.IsLoaded())
            m_audioPlayer.Stop();
        else
            PlaySound(nullptr, nullptr, 0); // 停止系统提示音

        m_isPreviewing = false;
        SetWindowTextW(m_btnPreview, L"试听");
        InvalidateRect(m_btnPreview, nullptr, FALSE);
        spdlog::info("MainWindow::PreviewMusic_: 用户手动停止试听");
        return;
    }

    // 未在试听 → 开始播放
    const std::wstring &musicPath = m_settings.GetMusicPath();
    if (!musicPath.empty() && m_audioPlayer.IsLoaded())
    {
        m_audioPlayer.Play();
        spdlog::info("MainWindow::PreviewMusic_: 开始试听音乐文件");
    }
    else
    {
        PlaySound(L"SystemExclamation", nullptr, SND_ALIAS | SND_ASYNC);
        spdlog::info("MainWindow::PreviewMusic_: 开始试听系统提示音");
    }

    m_isPreviewing = true;
    SetWindowTextW(m_btnPreview, L"停止");
    InvalidateRect(m_btnPreview, nullptr, FALSE);
}

// ─── ComputeDisplaySeconds_ ──────────────────────────────────────────────

int MainWindow::ComputeDisplaySeconds_() const
{
    TimerState state = m_timerEngine.GetState();
    if (state == TimerState::Running || state == TimerState::Paused)
        return m_timerEngine.GetRemainingSeconds();

    return m_hours * 3600 + m_minutes * 60 + m_seconds;
}

// ─── 消息处理 ────────────────────────────────────────────────────────────

void MainWindow::OnPaint_()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    int cx = rcClient.right;
    int cy = rcClient.bottom;

    // 双缓冲：所有内容先画到内存位图，再一次性 BitBlt 到屏幕。
    // 这样屏幕始终看到完整帧，彻底消除"背景色→文字"两步之间的空白帧闪烁。
    HDC     memDC  = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, cx, cy);
    HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, memBmp));

    {
        Gdiplus::Graphics g(memDC);
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        // 离屏缓冲区不支持次像素 ClearType，改用灰度抗锯齿，BitBlt 后文字边缘平滑
        g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

        TimerState state = m_timerEngine.GetState();

        if (state == TimerState::Running || state == TimerState::Paused)
        {
            // 倒计时全屏模式：主内容区超大数字居中；底部试听/选音乐栏保持正常
            m_painter.DrawMainBackground(g, cx, m_layout.footerY);
            m_painter.DrawFooterBackground(g, cx, cy, m_layout.footerY);
            int totalSec = ComputeDisplaySeconds_();
            m_painter.DrawCountdownModeTime(g, cx, m_layout.footerY,
                                            totalSec / 3600,
                                            (totalSec % 3600) / 60,
                                            totalSec % 60);
            m_painter.DrawMusicInfo(g, cx, m_layout.footerY, FOOTER_H, m_settings.GetMusicPath());
        }
        else
        {
            m_painter.DrawMainBackground(g, cx, m_layout.footerY);
            m_painter.DrawFooterBackground(g, cx, cy, m_layout.footerY);

            if (state == TimerState::Finished)
            {
                // 结束界面：脉冲圆圈 + "时间到了" + 柔和"00:00:00"（"停止"/"重置"由 WM_DRAWITEM 绘制）
                m_painter.DrawFinishedContent(g, cx, m_layout.footerY, m_animPhase);
            }
            else
            {
                // Idle 状态：时间显示 + 列标题 + 调节/输入控件
                int totalSec = ComputeDisplaySeconds_();
                int dh = totalSec / 3600;
                int dm = (totalSec % 3600) / 60;
                int ds = totalSec % 60;
                m_painter.DrawBigTimeText(g, cx, m_layout.timeAreaY, m_layout.timeAreaH, dh, dm, ds);

                m_painter.DrawColumnLabel(g, L"时", m_layout.groupCenterX[0], m_layout.labelY, m_layout.labelH);
                m_painter.DrawColumnLabel(g, L"分", m_layout.groupCenterX[1], m_layout.labelY, m_layout.labelH);
                m_painter.DrawColumnLabel(g, L"秒", m_layout.groupCenterX[2], m_layout.labelY, m_layout.labelH);

                if (m_tabMode == TabMode::Adjust)
                {
                    m_painter.DrawAdjustNumberBox(g,
                        m_layout.numBoxX[0], m_layout.ctrlY, NUM_BOX_W, GROUP_H, m_hours);
                    m_painter.DrawAdjustNumberBox(g,
                        m_layout.numBoxX[1], m_layout.ctrlY, NUM_BOX_W, GROUP_H, m_minutes);
                    m_painter.DrawAdjustNumberBox(g,
                        m_layout.numBoxX[2], m_layout.ctrlY, NUM_BOX_W, GROUP_H, m_seconds);
                }
                else
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        m_painter.DrawInputBox(g,
                            m_layout.groupX[i], m_layout.ctrlY, GROUP_W, GROUP_H);
                    }
                }
            }

            m_painter.DrawMusicInfo(g, cx, m_layout.footerY, FOOTER_H, m_settings.GetMusicPath());
        }
    } // Graphics 析构后内容提交到 memDC

    // 将内存帧一次性贴到屏幕（WS_CLIPCHILDREN 保证不覆盖子控件区域）
    BitBlt(hdc, 0, 0, cx, cy, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnTimer_()
{
    TimerState state = m_timerEngine.GetState();

    if (state == TimerState::Finished)
    {
        // 推进呼吸动画相位
        static constexpr float TWO_PI_F = 6.28318f;
        m_animPhase += ANIM_PHASE_STEP;
        if (m_animPhase >= TWO_PI_F)
            m_animPhase -= TWO_PI_F;

        // 自定义音乐循环：若用户未点停止且音乐播完，重新开始
        if (!m_finishedMusicStopped && m_audioPlayer.IsLoaded()
            && !m_audioPlayer.IsPlaying())
        {
            m_audioPlayer.Play();
            spdlog::debug("MainWindow::OnTimer_: 音乐自然播完，重新开始循环");
        }

        // 结束状态每 100ms 都要刷新界面（驱动动画）
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return;
    }

    // 检测试听音乐是否自然播完（仅对文件音乐有效；系统提示音极短，无需额外处理）
    if (m_isPreviewing && m_audioPlayer.IsLoaded() && !m_audioPlayer.IsPlaying())
    {
        m_isPreviewing = false;
        SetWindowTextW(m_btnPreview, L"试听");
        InvalidateRect(m_btnPreview, nullptr, FALSE);
        spdlog::debug("MainWindow::OnTimer_: 试听音乐自然播完，按钮恢复为\"试听\"");
    }

    // 正常倒计时 Tick
    bool finished = m_timerEngine.Tick(TIMER_INTERVAL);

    // 秒数不变就不触发重绘（定时器 100ms 触发，但显示只需整秒更新一次）
    int currentSec = ComputeDisplaySeconds_();
    if (currentSec != m_lastDisplayedSeconds)
    {
        m_lastDisplayedSeconds = currentSec;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }

    if (finished)
        OnCountdownFinished_();
}

void MainWindow::OnSize_(int cx, int cy)
{
    if (cx <= 0 || cy <= 0)
        return;

    RepositionControls_(cx, cy);
    // 倒计时模式下，暂停按钮的位置随窗口尺寸变化而需要同步更新
    UpdateControlsVisibility_();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void MainWindow::OnCommand_(int id, int notifyCode)
{
    (void)notifyCode;

    switch (id)
    {
    case ID_BTN_START:
        OnStartClick_();
        break;

    case ID_BTN_RESET:
        OnResetClick_();
        break;

    case ID_BTN_STOP:
        OnStopClick_();
        break;

    case ID_BTN_BACK_TO_MAIN:
        OnBackToMainClick_();
        break;

    case ID_BTN_MUSIC:
        SelectMusicFile_();
        break;

    case ID_BTN_PREVIEW:
        PreviewMusic_();
        break;

    case ID_BTN_TAB_ADJUST:
        if (m_tabMode != TabMode::Adjust)
        {
            ReadEditValues_();
            m_tabMode = TabMode::Adjust;
            UpdateControlsVisibility_();
            InvalidateRect(m_hwnd, nullptr, FALSE);
            spdlog::debug("MainWindow::OnCommand_: 切换到调节模式");
        }
        break;

    case ID_BTN_TAB_INPUT:
        if (m_tabMode != TabMode::Input)
        {
            WriteEditValues_();
            m_tabMode = TabMode::Input;
            UpdateControlsVisibility_();
            InvalidateRect(m_hwnd, nullptr, FALSE);
            spdlog::debug("MainWindow::OnCommand_: 切换到输入模式");
        }
        break;

    // 预设只设置时长，不自动开始
    case ID_BTN_PRESET_1:
        m_hours = 0; m_minutes = 1; m_seconds = 0;
        WriteEditValues_();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        spdlog::debug("MainWindow::OnCommand_: 预设 1 分钟");
        break;

    case ID_BTN_PRESET_5:
        m_hours = 0; m_minutes = 5; m_seconds = 0;
        WriteEditValues_();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        spdlog::debug("MainWindow::OnCommand_: 预设 5 分钟");
        break;

    case ID_BTN_PRESET_10:
        m_hours = 0; m_minutes = 10; m_seconds = 0;
        WriteEditValues_();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        spdlog::debug("MainWindow::OnCommand_: 预设 10 分钟");
        break;

    case ID_BTN_MINUS_H:
        AdjustTimeField_(0, false);
        break;
    case ID_BTN_PLUS_H:
        AdjustTimeField_(0, true);
        break;
    case ID_BTN_MINUS_M:
        AdjustTimeField_(1, false);
        break;
    case ID_BTN_PLUS_M:
        AdjustTimeField_(1, true);
        break;
    case ID_BTN_MINUS_S:
        AdjustTimeField_(2, false);
        break;
    case ID_BTN_PLUS_S:
        AdjustTimeField_(2, true);
        break;
    }
}

void MainWindow::OnDrawItem_(DRAWITEMSTRUCT *dis)
{
    if (dis->CtlType != ODT_BUTTON)
        return;

    bool isPressed  = (dis->itemState & ODS_SELECTED) != 0;
    bool isDisabled = (dis->itemState & ODS_DISABLED)  != 0;
    bool isHovered  = (dis->hwndItem  == m_hoveredBtn);

    // 先用父区域背景色填满整个矩形，消除圆角路径之外的角落残留色
    // 试听/选音乐在底部栏（棕色背景）；其余按钮在主内容区（奶油色背景）
    bool inFooter = (dis->CtlID == ID_BTN_PREVIEW || dis->CtlID == ID_BTN_MUSIC);
    COLORREF parentBg = inFooter ? RGB(0xF0, 0xDB, 0xC8) : RGB(0xFF, 0xFA, 0xF3);
    HBRUSH bgBrush = CreateSolidBrush(parentBg);
    FillRect(dis->hDC, &dis->rcItem, bgBrush);
    DeleteObject(bgBrush);

    wchar_t textBuf[64] = {};
    GetWindowTextW(dis->hwndItem, textBuf, 64);
    std::wstring text = textBuf;

    ButtonStyle style      = ButtonStyle::Secondary;
    bool        isSelected = false;

    switch (dis->CtlID)
    {
    // 开始/暂停：橙色主按钮
    case ID_BTN_START:
        style = ButtonStyle::Primary;
        break;

    // 重置：浅色副按钮
    case ID_BTN_RESET:
        style = ButtonStyle::Secondary;
        break;

    // 选择音乐：橙色按钮
    case ID_BTN_MUSIC:
        style = ButtonStyle::Primary;
        break;

    // 试听：描边按钮
    case ID_BTN_PREVIEW:
        style = ButtonStyle::Preview;
        break;

    // 结束界面"停止"：橙色主按钮
    case ID_BTN_STOP:
        style = ButtonStyle::Primary;
        break;

    // 倒计时模式"回到主界面"：浅色副按钮（低优先级操作）
    case ID_BTN_BACK_TO_MAIN:
        style = ButtonStyle::Secondary;
        break;

    // "调节"标签：分段控件左侧，激活时内嵌橙色圆角矩形
    case ID_BTN_TAB_ADJUST:
        style      = ButtonStyle::TabLeft;
        isSelected = (m_tabMode == TabMode::Adjust);
        break;

    // "输入"标签：分段控件右侧，激活时内嵌橙色圆角矩形
    case ID_BTN_TAB_INPUT:
        style      = ButtonStyle::TabRight;
        isSelected = (m_tabMode == TabMode::Input);
        break;

    // 快捷预设：胶囊形浅色按钮
    case ID_BTN_PRESET_1:
    case ID_BTN_PRESET_5:
    case ID_BTN_PRESET_10:
        style = ButtonStyle::Preset;
        break;

    // [−][+]：小号浅色按钮
    case ID_BTN_MINUS_H:
    case ID_BTN_MINUS_M:
    case ID_BTN_MINUS_S:
    case ID_BTN_PLUS_H:
    case ID_BTN_PLUS_M:
    case ID_BTN_PLUS_S:
        style = ButtonStyle::Secondary;
        break;
    }

    m_painter.DrawButton(dis->hDC, dis->rcItem, text, style, isPressed, isDisabled, isSelected, isHovered);
}

void MainWindow::OnGetMinMaxInfo_(MINMAXINFO *mmi)
{
    RECT rc = {0, 0, MIN_CLIENT_W, MIN_CLIENT_H};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    mmi->ptMinTrackSize.x = rc.right  - rc.left;
    mmi->ptMinTrackSize.y = rc.bottom - rc.top;
}

void MainWindow::OnDestroy_()
{
    spdlog::info("MainWindow::OnDestroy_: 保存配置并退出");

    if (m_tabMode == TabMode::Input)
        ReadEditValues_();

    m_settings.SetHours(m_hours);
    m_settings.SetMinutes(m_minutes);
    m_settings.SetSeconds(m_seconds);
    m_settings.Save();

    if (m_timerRunning)
    {
        KillTimer(m_hwnd, ID_TIMER);
        m_timerRunning = false;
    }

    m_audioPlayer.Stop();
    PlaySound(nullptr, nullptr, 0); // 停止可能仍在循环的系统提示音

    if (m_editFont)
    {
        DeleteObject(m_editFont);
        m_editFont = nullptr;
    }
    if (m_editBrush)
    {
        DeleteObject(m_editBrush);
        m_editBrush = nullptr;
    }

    PostQuitMessage(0);
}

} // namespace TickTock
