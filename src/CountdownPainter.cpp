/**
 * @file    CountdownPainter.cpp
 * @brief   GDI+ 自绘实现：主区背景、大时间字体、调节数字框、底部音乐区、各类按钮。
 * @author  balloonwj@qq.com
 * @date    2026-06-25
 */
#include "CountdownPainter.h"

#include <gdiplus.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace TickTock {

// ─── 颜色常量（ARGB，A=FF 不透明）────────────────────────────────────────

static constexpr Gdiplus::ARGB COLOR_BG               = 0xFFFFFAF3; ///< 主区背景  #fffaf3
static constexpr Gdiplus::ARGB COLOR_FOOTER_BG        = 0xFFF0DBC8; ///< 底部背景  #f0dbc8 （暖棕）
static constexpr Gdiplus::ARGB COLOR_TIME_TEXT        = 0xFF3C332B; ///< 大时间文字 #3c332b
static constexpr Gdiplus::ARGB COLOR_SECONDARY_TEXT   = 0xFF9A8771; ///< 次要文字   #9a8771
static constexpr Gdiplus::ARGB COLOR_NUM_BOX_BG       = 0xFFFBF3E8; ///< 调节数字框背景 #fbf3e8
static constexpr Gdiplus::ARGB COLOR_NUM_BOX_BORDER   = 0xFFE7D8C4; ///< 调节数字框边框
static constexpr Gdiplus::ARGB COLOR_ORANGE           = 0xFFD98A5F; ///< 橙色主色   #d98a5f
static constexpr Gdiplus::ARGB COLOR_ORANGE_HOVER     = 0xFFE59B72; ///< 橙色悬停（略亮约 8%）
static constexpr Gdiplus::ARGB COLOR_ORANGE_PRESSED   = 0xFFBD6F40; ///< 橙色按下（明显加深约 20%）
static constexpr Gdiplus::ARGB COLOR_WHITE            = 0xFFFFFFFF;
static constexpr Gdiplus::ARGB COLOR_SEC_BTN_BG       = 0xFFFBF3E8; ///< 副按钮背景 #fbf3e8
static constexpr Gdiplus::ARGB COLOR_SEC_BTN_HOVER    = 0xFFF0E5D4; ///< 副按钮悬停（略深）
static constexpr Gdiplus::ARGB COLOR_SEC_BTN_PRESSED  = 0xFFDAC9B2; ///< 副按钮按下（明显加深）
static constexpr Gdiplus::ARGB COLOR_SEC_BTN_BORDER   = 0xFFE0D0BE; ///< 副按钮边框
static constexpr Gdiplus::ARGB COLOR_DISABLED_BG      = 0xFFDDD5CB; ///< 禁用背景
static constexpr Gdiplus::ARGB COLOR_DISABLED_TXT     = 0xFFB0A898; ///< 禁用文字
static constexpr Gdiplus::ARGB COLOR_TAB_HOVER        = 0xFFE4CFBC; ///< 未选中 Tab 悬停（#f0dbc8 略暗 8%）
static constexpr Gdiplus::ARGB COLOR_TAB_PRESSED      = 0xFFCFB9A6; ///< 未选中 Tab 按下（#f0dbc8 明显加深 15%）

// ─── 字号（点）──────────────────────────────────────────────────────────

static constexpr float BIG_TIME_PT   = 60.0f; ///< 主界面大时间显示
static constexpr float COUNTDOWN_PT  = 96.0f; ///< 倒计时全屏模式字号，远大于主界面时间区以突出倒计时
static constexpr float ADJUST_PT     = 26.0f; ///< 调节模式数字框（更大更易读）
static constexpr float BTN_PT        = 13.0f; ///< 按钮文字
static constexpr float LABEL_PT      = 10.0f; ///< 列标题
static constexpr float SMALL_PT      =  9.0f; ///< 音乐副标题

// ─── 圆角参数 ────────────────────────────────────────────────────────────

static constexpr float RADIUS_BTN_PRIMARY  = 14.0f; ///< 开始/选择音乐按钮圆角（更饱满）
static constexpr float RADIUS_BTN_SMALL    = 10.0f; ///< 重置/试听按钮圆角
static constexpr float RADIUS_NUM_BOX      =  9.0f; ///< 数字框圆角
static constexpr float RADIUS_ORANGE_ICON  = 14.0f; ///< 音乐区橙色圆圈半径
static constexpr float RADIUS_TAB_SELECTED = 10.0f; ///< Tab 选中态内嵌橙色圆角矩形圆角

// ─── 圆角矩形路径辅助 ────────────────────────────────────────────────────

/**
 * @brief   向已有 GraphicsPath 追加圆角矩形（左上角原点在 0,0）。
 *          GDI+ 不支持 GraphicsPath 按值返回（拷贝构造为 protected），
 *          因此改为传引用并就地修改。
 */
static void BuildRoundRectPath(Gdiplus::GraphicsPath &path,
                                float w, float h, float radius)
{
    float d = 2.0f * radius;
    path.AddArc(0.0f,  0.0f,  d, d, 180.0f, 90.0f);
    path.AddArc(w - d, 0.0f,  d, d, 270.0f, 90.0f);
    path.AddArc(w - d, h - d, d, d,   0.0f, 90.0f);
    path.AddArc(0.0f,  h - d, d, d,  90.0f, 90.0f);
    path.CloseFigure();
}

/**
 * @brief   向已有 GraphicsPath 追加四角可独立设置圆角半径的矩形。
 *          顺时针方向绘制，从左上角起点 (0, tlR) 出发。
 *          任意角传 0 表示直角。
 * @param   tlR/trR/brR/blR  左上/右上/右下/左下圆角半径（px）
 */
static void BuildCornerRectPath(Gdiplus::GraphicsPath &path,
                                 float w, float h,
                                 float tlR, float trR, float brR, float blR)
{
    path.StartFigure();

    // 左上圆角（结束点：(tlR, 0)）
    if (tlR > 0)
        path.AddArc(0.0f, 0.0f, tlR * 2, tlR * 2, 180.0f, 90.0f);

    // 顶边
    path.AddLine(tlR, 0.0f, w - trR, 0.0f);

    // 右上圆角（结束点：(w, trR)）
    if (trR > 0)
        path.AddArc(w - trR * 2, 0.0f, trR * 2, trR * 2, 270.0f, 90.0f);

    // 右边
    path.AddLine(w, trR, w, h - brR);

    // 右下圆角（结束点：(w-brR, h)）
    if (brR > 0)
        path.AddArc(w - brR * 2, h - brR * 2, brR * 2, brR * 2, 0.0f, 90.0f);

    // 底边
    path.AddLine(w - brR, h, blR, h);

    // 左下圆角（结束点：(0, h-blR)）
    if (blR > 0)
        path.AddArc(0.0f, h - blR * 2, blR * 2, blR * 2, 90.0f, 90.0f);

    // 左边（回到起点）
    path.AddLine(0.0f, h - blR, 0.0f, tlR);

    path.CloseFigure();
}

// ─── EnsureResources_ ────────────────────────────────────────────────────

void CountdownPainter::EnsureResources_()
{
    if (m_ready)
        return;

    m_bigTimeFont   = std::make_unique<Gdiplus::Font>(L"Segoe UI", BIG_TIME_PT,  Gdiplus::FontStyleBold);
    m_countdownFont = std::make_unique<Gdiplus::Font>(L"Segoe UI", COUNTDOWN_PT, Gdiplus::FontStyleBold);
    m_adjustNumFont = std::make_unique<Gdiplus::Font>(L"Segoe UI", ADJUST_PT,    Gdiplus::FontStyleBold);
    m_btnFont       = std::make_unique<Gdiplus::Font>(L"Segoe UI", BTN_PT,       Gdiplus::FontStyleBold);
    m_labelFont     = std::make_unique<Gdiplus::Font>(L"Segoe UI", LABEL_PT,     Gdiplus::FontStyleRegular);
    m_smallFont     = std::make_unique<Gdiplus::Font>(L"Segoe UI", SMALL_PT,     Gdiplus::FontStyleRegular);

    m_ready = true;
}

// ─── 背景 ────────────────────────────────────────────────────────────────

void CountdownPainter::DrawMainBackground(Gdiplus::Graphics &g, int cx, int footerY)
{
    Gdiplus::Color c;
    c.SetValue(COLOR_BG);
    Gdiplus::SolidBrush brush(c);
    g.FillRectangle(&brush, 0, 0, cx, footerY);
}

void CountdownPainter::DrawFooterBackground(Gdiplus::Graphics &g, int cx, int cy, int footerY)
{
    Gdiplus::Color c;
    c.SetValue(COLOR_FOOTER_BG);
    Gdiplus::SolidBrush brush(c);
    g.FillRectangle(&brush, 0, footerY, cx, cy - footerY);
}

// ─── 大时间显示 ──────────────────────────────────────────────────────────

void CountdownPainter::DrawBigTimeText(Gdiplus::Graphics &g, int cx,
                                        int timeAreaY, int timeAreaH,
                                        int hours, int minutes, int seconds)
{
    EnsureResources_();

    wchar_t buf[16] = {};
    swprintf_s(buf, 16, L"%02d:%02d:%02d", hours, minutes, seconds);

    Gdiplus::RectF rect(
        0.0f,
        static_cast<float>(timeAreaY),
        static_cast<float>(cx),
        static_cast<float>(timeAreaH));

    Gdiplus::Color txtColor;
    txtColor.SetValue(COLOR_TIME_TEXT);
    Gdiplus::SolidBrush brush(txtColor);

    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    g.DrawString(buf, -1, m_bigTimeFont.get(), rect, &sf, &brush);
}

// ─── 倒计时全屏模式 ──────────────────────────────────────────────────────

void CountdownPainter::DrawCountdownModeTime(Gdiplus::Graphics &g, int cx, int cy,
                                              int hours, int minutes, int seconds)
{
    EnsureResources_();

    wchar_t buf[16] = {};
    swprintf_s(buf, 16, L"%02d:%02d:%02d", hours, minutes, seconds);

    // 绘制区域为整个内容区（cy = footerY），GDI+ 居中对齐自动垂直居中
    // 按钮固定在 70% 处，与此区域不重叠
    Gdiplus::RectF rect(
        0.0f,
        0.0f,
        static_cast<float>(cx),
        static_cast<float>(cy));

    Gdiplus::Color txtColor;
    txtColor.SetValue(COLOR_TIME_TEXT);
    Gdiplus::SolidBrush brush(txtColor);

    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    g.DrawString(buf, -1, m_countdownFont.get(), rect, &sf, &brush);
}

// ─── 列标题 ──────────────────────────────────────────────────────────────

void CountdownPainter::DrawColumnLabel(Gdiplus::Graphics &g, const wchar_t *text,
                                        int centerX, int labelY, int labelH)
{
    EnsureResources_();

    // 宽度给够，让居中对齐自然生效
    constexpr float LABEL_W = 80.0f;
    Gdiplus::RectF rect(
        static_cast<float>(centerX) - LABEL_W / 2.0f,
        static_cast<float>(labelY),
        LABEL_W,
        static_cast<float>(labelH));

    Gdiplus::Color c;
    c.SetValue(COLOR_SECONDARY_TEXT);
    Gdiplus::SolidBrush brush(c);

    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    g.DrawString(text, -1, m_labelFont.get(), rect, &sf, &brush);
}

// ─── 调节模式数字框 ──────────────────────────────────────────────────────

void CountdownPainter::DrawAdjustNumberBox(Gdiplus::Graphics &g,
                                            int boxX, int boxY, int boxW, int boxH,
                                            int value)
{
    EnsureResources_();

    float w = static_cast<float>(boxW);
    float h = static_cast<float>(boxH);
    float x = static_cast<float>(boxX);
    float y = static_cast<float>(boxY);

    // 先平移 Graphics 原点到数字框左上角，以便用 BuildRoundRectPath(w, h, ...) 绘制
    Gdiplus::Matrix origMatrix;
    g.GetTransform(&origMatrix);
    g.TranslateTransform(x, y);

    // 绘制圆角矩形背景
    Gdiplus::GraphicsPath bgPath;
    BuildRoundRectPath(bgPath, w, h, RADIUS_NUM_BOX);

    Gdiplus::Color bgColor;
    bgColor.SetValue(COLOR_NUM_BOX_BG);
    Gdiplus::SolidBrush bgBrush(bgColor);
    g.FillPath(&bgBrush, &bgPath);
    // 不绘制描边：依靠背景色差异与主区背景区分，避免视觉上的硬边框感

    // 在框内居中绘制两位数字
    wchar_t buf[8] = {};
    swprintf_s(buf, 8, L"%02d", value);

    Gdiplus::Color txtColor;
    txtColor.SetValue(COLOR_TIME_TEXT);
    Gdiplus::SolidBrush txtBrush(txtColor);

    Gdiplus::RectF textRect(0, 0, w, h);
    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(buf, -1, m_adjustNumFont.get(), textRect, &sf, &txtBrush);

    // 恢复 Graphics 变换
    g.SetTransform(&origMatrix);
}

// ─── 底部音乐区内容 ──────────────────────────────────────────────────────

void CountdownPainter::DrawMusicInfo(Gdiplus::Graphics &g, int cx,
                                      int footerY, int footerH,
                                      const std::wstring &musicPath)
{
    EnsureResources_();

    // 垂直居中基准
    float cy = static_cast<float>(footerY) + static_cast<float>(footerH) / 2.0f;

    // --- 橙色圆圈图标 ---
    constexpr float ICON_R  = RADIUS_ORANGE_ICON; // 半径 14px
    constexpr float ICON_CX = 24.0f;              // 图标水平中心

    Gdiplus::Color orangeColor;
    orangeColor.SetValue(COLOR_ORANGE);
    Gdiplus::SolidBrush orangeBrush(orangeColor);
    g.FillEllipse(&orangeBrush,
                  ICON_CX - ICON_R, cy - ICON_R,
                  ICON_R * 2.0f, ICON_R * 2.0f);

    // 图标内"♪"符号（白色）
    Gdiplus::Color whiteColor;
    whiteColor.SetValue(COLOR_WHITE);
    Gdiplus::SolidBrush whiteBrush(whiteColor);
    Gdiplus::RectF iconRect(ICON_CX - ICON_R, cy - ICON_R, ICON_R * 2.0f, ICON_R * 2.0f);
    Gdiplus::StringFormat iconSf;
    iconSf.SetAlignment(Gdiplus::StringAlignmentCenter);
    iconSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"♪", -1, m_smallFont.get(), iconRect, &iconSf, &whiteBrush);

    // --- "提醒音乐" 主标题 ---
    constexpr float TEXT_LEFT = 48.0f;   // 文字区左边
    constexpr float LINE_H    = 20.0f;   // 单行高度

    Gdiplus::Color labelColor;
    labelColor.SetValue(COLOR_TIME_TEXT);
    Gdiplus::SolidBrush labelBrush(labelColor);

    // 两行文字整体居中：上行中心在 cy-10，下行中心在 cy+10
    Gdiplus::RectF titleRect(TEXT_LEFT, cy - LINE_H, 200.0f, LINE_H);
    Gdiplus::StringFormat titleSf;
    titleSf.SetAlignment(Gdiplus::StringAlignmentNear);
    titleSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"提醒音乐", -1, m_btnFont.get(), titleRect, &titleSf, &labelBrush);

    // --- 副标题：文件名或"默认提示音" ---
    std::wstring subtitle;
    if (musicPath.empty())
    {
        subtitle = L"默认提示音";
    }
    else
    {
        // 只取文件名部分（最后一个 \ 之后）
        size_t pos = musicPath.rfind(L'\\');
        subtitle = (pos == std::wstring::npos) ? musicPath : musicPath.substr(pos + 1);
    }

    Gdiplus::Color secColor;
    secColor.SetValue(COLOR_SECONDARY_TEXT);
    Gdiplus::SolidBrush secBrush(secColor);

    Gdiplus::RectF subtitleRect(TEXT_LEFT, cy, 200.0f, LINE_H);
    Gdiplus::StringFormat subSf;
    subSf.SetAlignment(Gdiplus::StringAlignmentNear);
    subSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    subSf.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
    g.DrawString(subtitle.c_str(), -1, m_smallFont.get(), subtitleRect, &subSf, &secBrush);
}

// ─── DrawButton ──────────────────────────────────────────────────────────

void CountdownPainter::DrawButton(HDC hdc, const RECT &rc, const std::wstring &text,
                                   ButtonStyle style, bool isPressed, bool isDisabled,
                                   bool isSelected, bool isHovered)
{
    EnsureResources_();

    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    float w = static_cast<float>(rc.right  - rc.left);
    float h = static_cast<float>(rc.bottom - rc.top);

    // ── TabLeft / TabRight：分段控件半胶囊风格，单独处理后直接返回 ──────────
    if (style == ButtonStyle::TabLeft || style == ButtonStyle::TabRight)
    {
        float capR = h / 2.0f; // 胶囊半径 = 高度一半
        float tlR  = (style == ButtonStyle::TabLeft)  ? capR : 0.0f;
        float trR  = (style == ButtonStyle::TabRight) ? capR : 0.0f;
        float brR  = (style == ButtonStyle::TabRight) ? capR : 0.0f;
        float blR  = (style == ButtonStyle::TabLeft)  ? capR : 0.0f;

        // 第一步：铺满主区背景色，消除半胶囊路径之外矩形角落的残留色
        Gdiplus::Color mainBg;
        mainBg.SetValue(COLOR_BG);
        Gdiplus::SolidBrush mainBgBrush(mainBg);
        g.FillRectangle(&mainBgBrush, 0.0f, 0.0f, w, h);

        // 第二步：用容器底色填充半胶囊路径
        Gdiplus::Color containerColor;
        if (isDisabled)
        {
            containerColor.SetValue(COLOR_DISABLED_BG);
        }
        else if (!isSelected)
        {
            // 未选中：悬停/按下时在容器底色上加深
            if (isPressed)
                containerColor.SetValue(COLOR_TAB_PRESSED);
            else if (isHovered)
                containerColor.SetValue(COLOR_TAB_HOVER);
            else
                containerColor.SetValue(COLOR_FOOTER_BG);
        }
        else
        {
            // 选中：容器底色不变，选中高亮用内嵌矩形表达
            containerColor.SetValue(COLOR_FOOTER_BG);
        }

        Gdiplus::GraphicsPath tabPath;
        BuildCornerRectPath(tabPath, w, h, tlR, trR, brR, blR);
        Gdiplus::SolidBrush containerBrush(containerColor);
        g.FillPath(&containerBrush, &tabPath);

        // 第三步：选中态 → 在容器内绘制内嵌橙色圆角矩形
        if (isSelected && !isDisabled)
        {
            static constexpr float INSET = 4.0f; // 距容器边缘缩进量（px）
            float iw = w - 2.0f * INSET;
            float ih = h - 2.0f * INSET;

            Gdiplus::Matrix origMatrix;
            g.GetTransform(&origMatrix);
            g.TranslateTransform(INSET, INSET);

            Gdiplus::GraphicsPath selPath;
            BuildRoundRectPath(selPath, iw, ih, RADIUS_TAB_SELECTED);

            Gdiplus::Color selColor;
            if (isPressed)
                selColor.SetValue(COLOR_ORANGE_PRESSED);
            else if (isHovered)
                selColor.SetValue(COLOR_ORANGE_HOVER);
            else
                selColor.SetValue(COLOR_ORANGE);

            Gdiplus::SolidBrush selBrush(selColor);
            g.FillPath(&selBrush, &selPath);

            g.SetTransform(&origMatrix);
        }

        // 第四步：绘制文字
        Gdiplus::Color txtColor;
        if (isDisabled)
            txtColor.SetValue(COLOR_DISABLED_TXT);
        else
            txtColor.SetValue(isSelected ? COLOR_WHITE : COLOR_TIME_TEXT);

        Gdiplus::SolidBrush txtBrush(txtColor);
        Gdiplus::RectF textRectF(0, 0, w, h);
        Gdiplus::StringFormat sfTab;
        sfTab.SetAlignment(Gdiplus::StringAlignmentCenter);
        sfTab.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        g.DrawString(text.c_str(), -1, m_btnFont.get(), textRectF, &sfTab, &txtBrush);

        return;
    }

    // 计算圆角半径
    float radius = RADIUS_BTN_SMALL;
    if (style == ButtonStyle::Primary)
        radius = RADIUS_BTN_PRIMARY;
    else if (style == ButtonStyle::Preset)
        radius = std::min(w, h) / 2.0f;  // 完整胶囊形

    // 构建路径
    Gdiplus::GraphicsPath path;
    BuildRoundRectPath(path, w, h, radius);

    // 解析各风格的颜色
    Gdiplus::ARGB fillArgb = 0;
    Gdiplus::ARGB textArgb = 0;
    bool drawBorder = false;
    bool isOrangeFill = false;

    if (isDisabled)
    {
        fillArgb = COLOR_DISABLED_BG;
        textArgb = COLOR_DISABLED_TXT;
    }
    else if (style == ButtonStyle::Primary)
    {
        // 优先级：按下 > 悬停 > 正常
        if (isPressed)
            fillArgb = COLOR_ORANGE_PRESSED;
        else if (isHovered)
            fillArgb = COLOR_ORANGE_HOVER;
        else
            fillArgb = COLOR_ORANGE;
        textArgb    = COLOR_WHITE;
        isOrangeFill = true;
    }
    else if (style == ButtonStyle::Preview)
    {
        // 描边样式：浅色底、橙色边框和文字
        if (isPressed)
            fillArgb = COLOR_SEC_BTN_PRESSED;
        else if (isHovered)
            fillArgb = COLOR_SEC_BTN_HOVER;
        else
            fillArgb = COLOR_SEC_BTN_BG;
        textArgb   = isPressed ? COLOR_ORANGE_PRESSED : COLOR_ORANGE;
        drawBorder = true;
    }
    else
    {
        // Secondary / Preset 统一用浅色填充+边框
        if (isPressed)
            fillArgb = COLOR_SEC_BTN_PRESSED;
        else if (isHovered)
            fillArgb = COLOR_SEC_BTN_HOVER;
        else
            fillArgb = COLOR_SEC_BTN_BG;
        textArgb   = COLOR_TIME_TEXT;
        drawBorder = true;
    }

    // 填充背景（有边框时先填边框色，再用填充色覆盖内缩路径，形成 1px 边框环；
    // 完全依赖 FillPath 而非 DrawPath，避免描边在 DC 右/下边界被裁半像素的问题）
    Gdiplus::Color fillColor;
    fillColor.SetValue(fillArgb);
    Gdiplus::SolidBrush fillBrush(fillColor);

    if (drawBorder)
    {
        // 第一步：用边框色填满整个圆角路径
        Gdiplus::Color borderColor;
        if (style == ButtonStyle::Preview)
            borderColor.SetValue(COLOR_ORANGE);
        else
            borderColor.SetValue(COLOR_SEC_BTN_BORDER);

        Gdiplus::SolidBrush borderBrush(borderColor);
        g.FillPath(&borderBrush, &path);

        // 第二步：用按钮填充色覆盖内缩 1px 的更小路径，边框环宽度恰好 1px
        static constexpr float BORDER_W  = 1.0f;
        float innerRadius = std::max(0.0f, radius - BORDER_W);
        Gdiplus::GraphicsPath innerPath;
        BuildRoundRectPath(innerPath,
                           w - BORDER_W * 2.0f,
                           h - BORDER_W * 2.0f,
                           innerRadius);

        Gdiplus::Matrix savedMatrix;
        g.GetTransform(&savedMatrix);
        g.TranslateTransform(BORDER_W, BORDER_W);
        g.FillPath(&fillBrush, &innerPath);
        g.SetTransform(&savedMatrix);
    }
    else
    {
        g.FillPath(&fillBrush, &path);
    }

    // 绘制文字
    Gdiplus::Color txtColor;
    txtColor.SetValue(textArgb);
    Gdiplus::SolidBrush txtBrush(txtColor);

    Gdiplus::RectF textRect(0, 0, w, h);
    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(text.c_str(), -1, m_btnFont.get(), textRect, &sf, &txtBrush);
}

// ─── DrawFinishedContent ─────────────────────────────────────────────────

void CountdownPainter::DrawFinishedContent(Gdiplus::Graphics &g,
                                             int cx, int footerY,
                                             float animPhase)
{
    EnsureResources_();

    float fCX = static_cast<float>(cx);
    float fFY = static_cast<float>(footerY);

    // 圆圈中心与基础半径（使圆覆盖主内容区的大部分区域）
    float circleCX = fCX / 2.0f;
    float circleCY = fFY * 0.43f;
    float baseR    = std::min(fCX * 0.30f, fFY * 0.35f);

    // 呼吸脉冲：±8px 正弦波
    float pulseR = baseR + 8.0f * sinf(animPhase);

    // ── 脉冲圆圈描边（暖棕色细环）
    Gdiplus::Color ringC;
    ringC.SetValue(0xFFCBB59C);
    Gdiplus::Pen ringPen(ringC, 1.5f);
    g.DrawEllipse(&ringPen,
                  circleCX - pulseR, circleCY - pulseR,
                  pulseR * 2.0f, pulseR * 2.0f);

    // ── "时间到了"图标 + 文字（位于圆圈内靠上位置）
    constexpr float ICON_W   = 20.0f;
    constexpr float ICON_H   = 18.0f;
    constexpr float ICON_R   =  5.0f;
    constexpr float NOTICE_W = 74.0f; ///< "时间到了"四字在 13pt Bold 下的估算宽度
    constexpr float NOTICE_H = 22.0f;
    constexpr float GAP      =  6.0f;

    float totalW  = ICON_W + GAP + NOTICE_W;
    float startX  = circleCX - totalW / 2.0f;
    float noticeY = circleCY - baseR * 0.52f - NOTICE_H / 2.0f;

    // 橙色小图标（圆角矩形，模拟通知气泡）
    Gdiplus::Matrix origMatrix;
    g.GetTransform(&origMatrix);
    g.TranslateTransform(startX, noticeY);

    Gdiplus::GraphicsPath iconPath;
    BuildRoundRectPath(iconPath, ICON_W, ICON_H, ICON_R);

    Gdiplus::Color orangeC;
    orangeC.SetValue(COLOR_ORANGE);
    Gdiplus::SolidBrush orangeBrush(orangeC);
    g.FillPath(&orangeBrush, &iconPath);
    g.SetTransform(&origMatrix);

    // "时间到了"文字（深棕色）
    Gdiplus::Color darkC;
    darkC.SetValue(COLOR_TIME_TEXT);
    Gdiplus::SolidBrush darkBrush(darkC);

    Gdiplus::RectF noticeRect(startX + ICON_W + GAP, noticeY, NOTICE_W + 12.0f, NOTICE_H + 4.0f);
    Gdiplus::StringFormat noticeSf;
    noticeSf.SetAlignment(Gdiplus::StringAlignmentNear);
    noticeSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"时间到了", -1, m_btnFont.get(), noticeRect, &noticeSf, &darkBrush);

    // ── "00:00:00" 时间显示（柔和暖棕色，位于圆圈中心区域）
    float timeH   = baseR * 0.58f;
    float timeY   = circleCY - timeH / 2.0f - 2.0f;

    Gdiplus::RectF timeRect(0.0f, timeY, fCX, timeH);

    Gdiplus::Color mutedC;
    mutedC.SetValue(0xFFBFA08A); ///< 柔和暖棕色，与正常深色区分，体现"结束"状态
    Gdiplus::SolidBrush mutedBrush(mutedC);

    Gdiplus::StringFormat timeSf;
    timeSf.SetAlignment(Gdiplus::StringAlignmentCenter);
    timeSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(L"00:00:00", -1, m_bigTimeFont.get(), timeRect, &timeSf, &mutedBrush);
}

// ─── DrawInputBox ────────────────────────────────────────────────────────

void CountdownPainter::DrawInputBox(Gdiplus::Graphics &g,
                                     int boxX, int boxY, int boxW, int boxH)
{
    EnsureResources_();

    float w = static_cast<float>(boxW);
    float h = static_cast<float>(boxH);
    float x = static_cast<float>(boxX);
    float y = static_cast<float>(boxY);

    Gdiplus::Matrix origMatrix;
    g.GetTransform(&origMatrix);
    g.TranslateTransform(x, y);

    // 先用边框色填满外层路径，再用背景色填充内缩 1px 的路径，形成 1px 边框环；
    // 同 DrawButton 的做法，避免 DrawPath 在右/下边界被 DC clip 裁剪
    static constexpr float INPUT_BORDER_W = 1.0f;

    Gdiplus::GraphicsPath bgPath;
    BuildRoundRectPath(bgPath, w, h, RADIUS_NUM_BOX);

    Gdiplus::Color borderColor;
    borderColor.SetValue(COLOR_NUM_BOX_BORDER);
    Gdiplus::SolidBrush borderBrush(borderColor);
    g.FillPath(&borderBrush, &bgPath);

    float innerR = std::max(0.0f, RADIUS_NUM_BOX - INPUT_BORDER_W);
    Gdiplus::GraphicsPath innerPath;
    BuildRoundRectPath(innerPath,
                       w - INPUT_BORDER_W * 2.0f,
                       h - INPUT_BORDER_W * 2.0f,
                       innerR);

    Gdiplus::Color bgColor;
    bgColor.SetValue(COLOR_NUM_BOX_BG);
    Gdiplus::SolidBrush bgBrush(bgColor);

    Gdiplus::Matrix savedMatrix;
    g.GetTransform(&savedMatrix);
    g.TranslateTransform(INPUT_BORDER_W, INPUT_BORDER_W);
    g.FillPath(&bgBrush, &innerPath);
    g.SetTransform(&savedMatrix);

    g.SetTransform(&origMatrix);
}

} // namespace TickTock
