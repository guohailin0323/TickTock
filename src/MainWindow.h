/**
 * @file    MainWindow.h
 * @brief   TickTock 主窗口：包含所有控件创建、布局计算、消息分派、倒计时逻辑。
 *          UI 遵循 design/image/主界面.png，客户区默认 480×620，支持缩放。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

#include "stdafx.h"
#include "TimerEngine.h"
#include "AppSettings.h"
#include "AudioPlayer.h"
#include "CountdownPainter.h"
#include <string>

namespace TickTock {

// ─── 控件 ID ────────────────────────────────────────────────────────────

static constexpr int ID_EDIT_HOURS     = 101; ///< 小时 EDIT（输入模式）
static constexpr int ID_EDIT_MINUTES   = 102; ///< 分钟 EDIT（输入模式）
static constexpr int ID_EDIT_SECONDS   = 103; ///< 秒数 EDIT（输入模式）
static constexpr int ID_BTN_START      = 107; ///< 开始/暂停
static constexpr int ID_BTN_RESET      = 108; ///< 重置
static constexpr int ID_BTN_MUSIC      = 109; ///< 选择音乐
static constexpr int ID_BTN_MINUS_H    = 110; ///< 小时 [−]
static constexpr int ID_BTN_PLUS_H     = 111; ///< 小时 [+]
static constexpr int ID_BTN_MINUS_M    = 112; ///< 分钟 [−]
static constexpr int ID_BTN_PLUS_M     = 113; ///< 分钟 [+]
static constexpr int ID_BTN_MINUS_S    = 114; ///< 秒数 [−]
static constexpr int ID_BTN_PLUS_S     = 115; ///< 秒数 [+]
static constexpr int ID_BTN_TAB_ADJUST = 116; ///< "调节" 标签
static constexpr int ID_BTN_TAB_INPUT  = 117; ///< "输入" 标签
static constexpr int ID_BTN_PRESET_1   = 118; ///< 快捷预设 1 分钟
static constexpr int ID_BTN_PRESET_5   = 119; ///< 快捷预设 5 分钟
static constexpr int ID_BTN_PRESET_10  = 120; ///< 快捷预设 10 分钟
static constexpr int ID_BTN_PREVIEW    = 121; ///< 试听
static constexpr int ID_BTN_STOP          = 122; ///< 结束界面的"停止"按钮（停止音乐，保留结束界面）
static constexpr int ID_BTN_BACK_TO_MAIN  = 123; ///< 倒计时模式"回到主界面"按钮（停止并重置）
static constexpr int ID_TIMER             = 201; ///< WM_TIMER 定时器 ID

// ─── 尺寸常量（以默认 480×620 为参考，部分随窗口缩放）────────────────

static constexpr int DEFAULT_CLIENT_W = 630; ///< 默认客户区宽（px）
static constexpr int DEFAULT_CLIENT_H = 620; ///< 默认客户区高（px）
static constexpr int MIN_CLIENT_W     = 440; ///< 最小客户区宽
static constexpr int MIN_CLIENT_H     = 520; ///< 最小客户区高（90px footer + 430px 内容）
static constexpr int FOOTER_H         =  90; ///< 底部音乐区固定高度（px）
static constexpr int TIMER_INTERVAL   = 100; ///< 定时器驱动间隔（ms）

// 调节区每列尺寸（固定，不随缩放变化）
static constexpr int GROUP_W           = 140; ///< [−][数字][+] 总宽
static constexpr int GROUP_H           =  52; ///< [−][数字][+] 总高
static constexpr int MINUS_PLUS_W      =  38; ///< [−][+] 宽度（增大以更显眼）
static constexpr int MINUS_PLUS_H      =  38; ///< [−][+] 高度
static constexpr int NUM_BOX_INNER_GAP =   4; ///< [−]/[+] 与数字框之间的间距（px）
///< 数字框宽 = GROUP_W − 2×MINUS_PLUS_W − 2×GAP = 56
static constexpr int NUM_BOX_W         =  56;
///< 输入模式下 EDIT 控件在 DrawInputBox 内的四边缩进量（px）；使 GDI+ 绘制的圆角边框可见
static constexpr int EDIT_INSET        =   3;

// 标签按钮
static constexpr int TAB_BTN_W  = 80; ///< 标签按钮宽
static constexpr int TAB_BTN_H  = 36; ///< 标签按钮高
static constexpr int TAB_BTN_GAP = 0; ///< 两个标签紧贴，视觉上形成统一的分段控件容器

// 快捷预设
static constexpr int PRESET_BTN_H  = 34; ///< 预设按钮高
static constexpr int PRESET_1_W    = 68; ///< "1分"宽
static constexpr int PRESET_5_W    = 68; ///< "5分"宽
static constexpr int PRESET_10_W   = 78; ///< "10分"宽
static constexpr int PRESET_GAP    = 12; ///< 预设按钮间距

// 开始/重置按钮
static constexpr int BTN_ROW_H     = 56; ///< 按钮行高
static constexpr int START_BTN_W   = 296; ///< 开始按钮宽
static constexpr int RESET_BTN_W   = 108; ///< 重置按钮宽
static constexpr int START_RESET_GAP = 16; ///< 开始与重置之间的间距

// 结束界面"停止"按钮与动画
static constexpr int   STOP_BTN_W      = 180;      ///< "停止"按钮宽（px）
static constexpr int   STOP_BTN_H      =  52;      ///< "停止"按钮高（px）
static constexpr float ANIM_PHASE_STEP = 0.31416f; ///< 每 100ms 推进的呼吸相位（2π/20，2s 一次完整呼吸）

// 倒计时全屏模式：暂停/继续 + 回到主界面，两个按钮并排居中显示在大数字下方
static constexpr int COUNTDOWN_PAUSE_BTN_W  = 160; ///< 暂停/继续按钮宽（px）
static constexpr int COUNTDOWN_BACK_BTN_W   = 160; ///< 回到主界面按钮宽（px）
static constexpr int COUNTDOWN_BTN_H        =  52; ///< 两个按钮高度（px）
static constexpr int COUNTDOWN_BTNS_GAP     =  16; ///< 两按钮之间的间距（px）
/// 两个按钮组顶边相对内容区高度（footerY）的比例，位于大数字正下方
static constexpr int COUNTDOWN_BTN_Y_NUM    =  70;
static constexpr int COUNTDOWN_BTN_Y_DEN    = 100;

// 底部按钮
static constexpr int PREVIEW_BTN_W  =  68; ///< 试听按钮宽
static constexpr int PREVIEW_BTN_H  =  36; ///< 试听按钮高
static constexpr int MUSIC_BTN_W    = 100; ///< 选择音乐按钮宽
static constexpr int MUSIC_BTN_H    =  36; ///< 选择音乐按钮高
static constexpr int FOOTER_BTN_GAP =   8; ///< 底部两按钮间距
static constexpr int FOOTER_BTN_R   =  16; ///< 距右边距

// ─── 枚举 ───────────────────────────────────────────────────────────────

/**
 * @brief 时间设置区的标签模式。
 */
enum class TabMode
{
    Adjust, ///< 调节模式：[−] 数字 [+] 微调
    Input,  ///< 输入模式：EDIT 控件直接键入
};

// ─── 布局缓存 ────────────────────────────────────────────────────────────

/**
 * @brief RepositionControls_ 计算后缓存的各区域坐标，供 WM_PAINT 读取。
 */
struct LayoutCache
{
    int footerY;         ///< 底部区域顶边 y
    int timeAreaY;       ///< 大时间显示顶边 y
    int timeAreaH;       ///< 大时间显示区高度
    int tabY;            ///< 标签栏顶边 y
    int labelY;          ///< 列标题（时/分/秒）顶边 y
    int labelH;          ///< 列标题高度
    int ctrlY;           ///< 调节/输入控件区顶边 y
    int presetY;         ///< 快捷预设行顶边 y
    int btnY;            ///< 开始/重置按钮行顶边 y
    int groupX[3];       ///< 三列组左边 x（H/M/S 顺序）
    int groupCenterX[3]; ///< 三列组中心 x（绘制列标题用）
    int numBoxX[3];      ///< 三列数字框左边 x
    int stopBtnX;        ///< 结束界面"停止"按钮左边 x
    int stopBtnY;        ///< 结束界面"停止"按钮顶边 y
};

// ─── MainWindow ──────────────────────────────────────────────────────────

/**
 * @class   MainWindow
 * @brief   TickTock 主窗口，封装所有 Win32 消息处理和 UI 状态。
 *
 *          调用顺序：
 *          1. Create(hInstance) → 注册类 + 创建窗口 + 所有子控件
 *          2. ShowAndRun()      → ShowWindow + 消息循环，关闭时返回
 */
class MainWindow
{
public:
    MainWindow()  = default;
    ~MainWindow() = default;

    MainWindow(const MainWindow &)            = delete;
    MainWindow &operator=(const MainWindow &) = delete;

    /**
     * @brief   注册窗口类、创建主窗口（含所有子控件）、加载配置。
     * @param   hInstance  应用实例句柄（来自 WinMain）
     * @return  true=成功；false=失败（详情已记录日志）
     */
    bool Create(HINSTANCE hInstance);

    /**
     * @brief   显示窗口并运行消息循环，直到窗口关闭后返回退出码。
     * @return  消息循环退出码（通常为 0）
     */
    int ShowAndRun();

private:
    // ─── Win32 消息入口

    static LRESULT CALLBACK WndProc_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    /**
     * @brief   所有 owner-draw 按钮共享的子类化过程，用于捕获 WM_MOUSEMOVE / WM_MOUSELEAVE
     *          以实现 hover 状态跟踪；触发 InvalidateRect 使按钮重绘为悬停颜色。
     */
    static LRESULT CALLBACK ButtonSubclassProc_(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                                                  UINT_PTR subclassId, DWORD_PTR refData);
    LRESULT HandleMessage_(UINT msg, WPARAM wp, LPARAM lp);

    // ─── 消息处理方法

    /** @brief WM_CREATE：创建子控件、加载配置、启动定时器。 */
    void OnCreate_();

    /** @brief WM_PAINT：GDI+ 绘制背景、时间、标签、数字框、音乐区。 */
    void OnPaint_();

    /** @brief WM_TIMER：驱动 TimerEngine::Tick，处理倒计时结束。 */
    void OnTimer_();

    /** @brief WM_SIZE：重算布局并移动所有子控件。 */
    void OnSize_(int cx, int cy);

    /** @brief WM_COMMAND：处理所有按钮点击。 */
    void OnCommand_(int id, int notifyCode);

    /** @brief WM_DRAWITEM：owner-draw 按钮绘制，转发到 CountdownPainter。 */
    void OnDrawItem_(DRAWITEMSTRUCT *dis);

    /** @brief WM_GETMINMAXINFO：限制最小窗口尺寸。 */
    void OnGetMinMaxInfo_(MINMAXINFO *mmi);

    /** @brief WM_DESTROY：保存配置、释放资源、PostQuitMessage。 */
    void OnDestroy_();

    // ─── 控件创建 / 布局

    /** @brief 在 WM_CREATE 中一次性创建所有子控件。 */
    void CreateControls_();

    /**
     * @brief   根据客户区大小计算 m_layout，并 MoveWindow 移动所有子控件。
     * @param   cx  客户区宽度（px）
     * @param   cy  客户区高度（px）
     */
    void RepositionControls_(int cx, int cy);

    /** @brief 按当前 m_tabMode 和定时器状态切换子控件的显隐与禁用。 */
    void UpdateControlsVisibility_();

    // ─── 调节模式

    /**
     * @brief   处理 [−] 或 [+] 点击：递增/递减字段并刷新显示。
     * @param   field      0=时, 1=分, 2=秒
     * @param   increment  true=加一, false=减一
     */
    void AdjustTimeField_(int field, bool increment);

    // ─── 输入模式

    /** @brief 读取三个 EDIT 控件中的数值，写入 m_hours/m_minutes/m_seconds。 */
    void ReadEditValues_();

    /** @brief 将 m_hours/m_minutes/m_seconds 写入三个 EDIT 控件。 */
    void WriteEditValues_();

    // ─── 倒计时逻辑

    /** @brief "开始/暂停"按钮点击处理。 */
    void OnStartClick_();

    /** @brief "重置"按钮点击处理。 */
    void OnResetClick_();

    /** @brief 结束界面"停止"按钮点击：停止音乐，界面保留在结束状态。 */
    void OnStopClick_();

    /** @brief 倒计时模式"回到主界面"按钮点击：停止试听音乐、重置倒计时、恢复主界面控件。 */
    void OnBackToMainClick_();

    /**
     * @brief   若当前正在试听音乐，则停止并重置试听状态。
     *          在暂停、回到主界面等操作中调用，避免试听音乐与倒计时状态脱节。
     */
    void StopPreviewIfPlaying_();

    /** @brief 倒计时结束时调用（在 OnTimer_ 中触发）。 */
    void OnCountdownFinished_();

    // ─── 音乐

    /** @brief 弹出文件对话框选择音乐文件，成功后保存路径并刷新。 */
    void SelectMusicFile_();

    /** @brief 试听：有文件则播放文件，无文件则播放系统提示音。 */
    void PreviewMusic_();

    // ─── 绘制辅助

    /**
     * @brief   计算当前应显示在大时间框中的秒数。
     *          Running/Paused 返回 TimerEngine 剩余秒，其他状态返回已配置总秒。
     */
    int ComputeDisplaySeconds_() const;

    /** @brief 读取 EDIT 控件中的整数（文本为空或非数字时返回 0）。 */
    int GetEditInt_(HWND editHwnd) const;

    // ─── 成员变量 ────────────────────────────────────────────────────────

    HWND      m_hwnd      = nullptr; ///< 主窗口句柄
    HINSTANCE m_hInstance = nullptr; ///< 应用实例句柄

    // 子控件句柄
    HWND m_editHours   = nullptr; ///< 小时 EDIT（输入模式可见）
    HWND m_editMinutes = nullptr; ///< 分钟 EDIT
    HWND m_editSeconds = nullptr; ///< 秒数 EDIT

    HWND m_btnMinusH = nullptr; ///< 小时 [−]（调节模式可见）
    HWND m_btnPlusH  = nullptr; ///< 小时 [+]
    HWND m_btnMinusM = nullptr; ///< 分钟 [−]
    HWND m_btnPlusM  = nullptr; ///< 分钟 [+]
    HWND m_btnMinusS = nullptr; ///< 秒数 [−]
    HWND m_btnPlusS  = nullptr; ///< 秒数 [+]

    HWND m_btnTabAdjust = nullptr; ///< "调节"标签按钮（owner-draw）
    HWND m_btnTabInput  = nullptr; ///< "输入"标签按钮（owner-draw）

    HWND m_btnPreset1  = nullptr; ///< "1分" 预设（owner-draw）
    HWND m_btnPreset5  = nullptr; ///< "5分" 预设（owner-draw）
    HWND m_btnPreset10 = nullptr; ///< "10分" 预设（owner-draw）

    HWND m_btnStart      = nullptr; ///< 开始/暂停/继续（owner-draw）
    HWND m_btnReset      = nullptr; ///< 重置（owner-draw）
    HWND m_btnPreview    = nullptr; ///< 试听（owner-draw）
    HWND m_btnMusic      = nullptr; ///< 选择音乐（owner-draw）
    HWND m_btnStop       = nullptr; ///< 结束界面"停止"（owner-draw，仅在 Finished 状态显示）
    HWND m_btnBackToMain = nullptr; ///< 倒计时模式"回到主界面"（owner-draw，仅 Running/Paused 状态显示）

    // GDI 资源
    HFONT  m_editFont  = nullptr; ///< EDIT 控件字体：Segoe UI Bold 24pt
    HBRUSH m_editBrush = nullptr; ///< EDIT 控件背景刷（#fbf3e8 浅奶油色）

    // 业务状态
    TabMode m_tabMode = TabMode::Adjust; ///< 当前激活的标签

    int m_hours   = 0; ///< 用户设置的小时 [0, 99]
    int m_minutes = 5; ///< 用户设置的分钟 [0, 59]，默认 5 分钟
    int m_seconds = 0; ///< 用户设置的秒数 [0, 59]

    // 业务对象
    TimerEngine      m_timerEngine; ///< 纯 C++ 倒计时状态机
    AppSettings      m_settings;   ///< INI 配置读写
    AudioPlayer      m_audioPlayer;///< MCI 音频播放
    CountdownPainter m_painter;    ///< GDI+ 绘制工具

    // 布局缓存
    LayoutCache m_layout = {}; ///< RepositionControls_ 更新，OnPaint_ 读取

    HWND  m_hoveredBtn            = nullptr; ///< 当前鼠标悬停的按钮句柄；nullptr 表示无悬停
    bool  m_timerRunning          = false;  ///< WM_TIMER 是否已在运行（防止重复 SetTimer）
    int   m_lastDisplayedSeconds  = -1;    ///< OnTimer_ 上次触发重绘时的秒数；-1 表示需要强制刷新
    float m_animPhase             = 0.0f;  ///< 结束界面呼吸动画当前相位 [0, 2π)
    bool  m_finishedMusicStopped  = false; ///< 用户已点击"停止"停止了音乐（不再循环播放）
    bool  m_isPreviewing          = false; ///< 底部"试听"正在播放中；true 时按钮文字为"停止"，false 时为"试听"

    static constexpr wchar_t WINDOW_CLASS[] = L"TickTockMainWnd"; ///< 窗口类注册名
};

} // namespace TickTock
