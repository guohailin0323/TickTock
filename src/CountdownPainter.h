/**
 * @file    CountdownPainter.h
 * @brief   GDI+ 自绘辅助类：负责主窗口所有纯绘制操作。
 *          不持有任何 Win32 句柄，只接收 GDI+ Graphics 或 HDC 并在上面绘制。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#pragma once

#include "stdafx.h"
#include <string>

namespace TickTock {

/**
 * @brief 自定义按钮的绘制风格。
 */
enum class ButtonStyle
{
    Primary,   ///< 橙色填充，白色文字：开始/暂停、选择音乐
    Secondary, ///< 浅色填充、细边框、深色文字：重置、[−][+]
    TabLeft,   ///< 分段控件左侧标签（左侧圆角、右侧直角）；isSelected=true 时内嵌橙色圆角矩形
    TabRight,  ///< 分段控件右侧标签（左侧直角、右侧圆角）；isSelected=true 时内嵌橙色圆角矩形
    Preset,    ///< 胶囊形，浅色填充、细边框：快捷时长预设
    Preview,   ///< 描边按钮，橙色边框和文字：试听
};

/**
 * @class   CountdownPainter
 * @brief   纯 GDI+ 绘制工具类，供 MainWindow 在 WM_PAINT / WM_DRAWITEM 中调用。
 *
 *          所有方法均无状态副作用，内部仅缓存字体对象。
 *          使用前提：GdiplusStartup 必须已在调用方完成。
 */
class CountdownPainter
{
public:
    /**
     * @brief   绘制主内容区背景（y=[0, footerY)），纯色 #fffaf3。
     * @param   g        GDI+ Graphics
     * @param   cx       客户区宽度（px）
     * @param   footerY  底部音乐区顶边 y（px）
     */
    void DrawMainBackground(Gdiplus::Graphics &g, int cx, int footerY);

    /**
     * @brief   绘制底部音乐区背景（y=[footerY, cy)），暖棕色 #f0dbc8。
     * @param   g        GDI+ Graphics
     * @param   cx       客户区宽度
     * @param   cy       客户区高度
     * @param   footerY  底部音乐区顶边 y
     */
    void DrawFooterBackground(Gdiplus::Graphics &g, int cx, int cy, int footerY);

    /**
     * @brief   绘制始终可见的大字体倒计时。格式 HH:MM:SS，Segoe UI 56pt Bold。
     * @param   g         GDI+ Graphics
     * @param   cx        客户区宽度
     * @param   timeAreaY 绘制区域顶边 y
     * @param   timeAreaH 绘制区域高度
     * @param   hours     显示的小时数 [0, 99]
     * @param   minutes   显示的分钟数 [0, 59]
     * @param   seconds   显示的秒数   [0, 59]
     */
    void DrawBigTimeText(Gdiplus::Graphics &g, int cx,
                         int timeAreaY, int timeAreaH,
                         int hours, int minutes, int seconds);

    /**
     * @brief   绘制时/分/秒列标题文字（在调节与输入两种模式下均显示）。
     * @param   g           GDI+ Graphics
     * @param   text        标题文字（"时"/"分"/"秒"）
     * @param   centerX     文字水平中心 x（px）
     * @param   labelY      标签顶边 y
     * @param   labelH      标签高度
     */
    void DrawColumnLabel(Gdiplus::Graphics &g, const wchar_t *text,
                         int centerX, int labelY, int labelH);

    /**
     * @brief   绘制调节模式下单组数字显示框（圆角矩形背景 + 两位数字）。
     *          数字框位于 [−] 和 [+] 按钮之间，[−][+] 本身由 WM_DRAWITEM 绘制。
     * @param   g       GDI+ Graphics
     * @param   boxX    数字框左边 x
     * @param   boxY    数字框顶边 y
     * @param   boxW    数字框宽度
     * @param   boxH    数字框高度
     * @param   value   要显示的数值 [0, 99]
     */
    void DrawAdjustNumberBox(Gdiplus::Graphics &g,
                              int boxX, int boxY, int boxW, int boxH,
                              int value);

    /**
     * @brief   绘制倒计时结束界面：脉冲圆圈 + "时间到了"图标与文字 + 柔和的 "00:00:00"。
     *          "停止"按钮由 MainWindow 作为 owner-draw 控件独立绘制，不在此方法内。
     * @param   g          GDI+ Graphics
     * @param   cx         客户区宽度（px）
     * @param   footerY    底部音乐区顶边 y（px）
     * @param   animPhase  呼吸动画相位 [0, 2π)，每 100ms 推进一步
     */
    void DrawFinishedContent(Gdiplus::Graphics &g, int cx, int footerY, float animPhase);

    /**
     * @brief   绘制输入模式下单组 EDIT 的圆角边框背景（EDIT 控件本身用 WS_CLIPCHILDREN 覆盖其上）。
     *          EDIT 需比此框四边各缩进 EDIT_INSET 像素，使边框可见。
     * @param   g     GDI+ Graphics
     * @param   boxX  边框左边 x
     * @param   boxY  边框顶边 y
     * @param   boxW  边框宽度（通常等于 GROUP_W）
     * @param   boxH  边框高度（通常等于 GROUP_H）
     */
    void DrawInputBox(Gdiplus::Graphics &g, int boxX, int boxY, int boxW, int boxH);

    /**
     * @brief   绘制底部音乐区的图标和文字标签（不含按钮）。
     * @param   g          GDI+ Graphics
     * @param   cx         客户区宽度
     * @param   footerY    底部音乐区顶边 y
     * @param   footerH    底部音乐区高度
     * @param   musicPath  已选音乐文件完整路径，空字符串表示使用默认系统声音
     */
    void DrawMusicInfo(Gdiplus::Graphics &g, int cx,
                       int footerY, int footerH,
                       const std::wstring &musicPath);

    /**
     * @brief   倒计时全屏运行模式：用超大字体将 HH:MM:SS 居中绘制于窗口上半区域。
     *          运行/暂停时所有设置控件隐藏，只剩此文字与底部的暂停/继续按钮。
     * @param   g        GDI+ Graphics
     * @param   cx       客户区宽度（px）
     * @param   cy       客户区高度（px）
     * @param   hours    显示的小时数 [0, 99]
     * @param   minutes  显示的分钟数 [0, 59]
     * @param   seconds  显示的秒数   [0, 59]
     */
    void DrawCountdownModeTime(Gdiplus::Graphics &g, int cx, int cy,
                               int hours, int minutes, int seconds);

    /**
     * @brief   绘制自定义按钮（在 WM_DRAWITEM 中调用）。
     * @param   hdc        目标 HDC（来自 DRAWITEMSTRUCT::hDC）
     * @param   rc         按钮客户矩形，原点 (0, 0)（来自 DRAWITEMSTRUCT::rcItem）
     * @param   text       按钮文字
     * @param   style      按钮风格
     * @param   isPressed  true 表示按下状态（ODS_SELECTED）
     * @param   isDisabled true 表示禁用状态（ODS_DISABLED）
     * @param   isSelected 仅对 Tab 风格有效：true 表示此标签当前处于激活状态
     */
    void DrawButton(HDC hdc, const RECT &rc, const std::wstring &text,
                    ButtonStyle style, bool isPressed, bool isDisabled,
                    bool isSelected = false, bool isHovered = false);

private:
    /**
     * @brief   懒加载：首次绘制前创建所有 GDI+ 字体对象。
     */
    void EnsureResources_();

    bool m_ready = false; ///< 字体资源是否已完成懒加载

    std::unique_ptr<Gdiplus::Font> m_bigTimeFont;    ///< 大时间字体       Segoe UI 60pt Bold
    std::unique_ptr<Gdiplus::Font> m_countdownFont; ///< 倒计时全屏字体   Segoe UI 96pt Bold（全屏模式专用）
    std::unique_ptr<Gdiplus::Font> m_adjustNumFont; ///< 调节数字字体     Segoe UI 26pt Bold
    std::unique_ptr<Gdiplus::Font> m_btnFont;       ///< 按钮/标签字体   Segoe UI 13pt Bold
    std::unique_ptr<Gdiplus::Font> m_labelFont;     ///< 列标题字体       Segoe UI 10pt Regular
    std::unique_ptr<Gdiplus::Font> m_smallFont;     ///< 小号文字          Segoe UI  9pt Regular
};

} // namespace TickTock
