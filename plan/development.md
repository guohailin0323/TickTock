# TickTock 开发计划

## 总览

| 阶段 | 内容 | 依赖 | 状态 |
|------|------|------|------|
| 0 | 构建系统（Win32 + spdlog 静态） | — | ✅ 部分完成（TimerEngine 已验证） |
| 1 | TimerEngine（倒计时状态机） | 阶段0 | ✅ 完成（25/25 测试通过） |
| 2 | AppSettings（配置读写） | 阶段1 | ⏳ 待开始 |
| 3 | AudioPlayer（音频播放） | 阶段1 | ⏳ 待开始 |
| 4+5 | 主窗口 UI（原生 Win32 + GDI+） | 阶段1-3 | ⏳ 待开始 |

## 执行原则

1. **测试先行**：每个阶段先写测试，跑通后才写实现
2. **测试用例先与用户确认**：不擅自改测试范围
3. **推测先实证**：结论性表述必须附证据（文件路径:行号 / 命令输出）

---

## UI 技术选型：原生 Win32 + GDI+

**不使用** BalloonUI（已删除依赖）。界面全部通过 Win32 API + GDI+ 自绘。

### 窗口方案

```
主窗口：WS_POPUP（无标题栏）+ WS_EX_LAYERED（透明/圆角）
  ├─ 自绘区（WM_PAINT，GDI+ 渲染背景 + 大字体时间）
  ├─ 标题栏区（自绘：标题文字 + 关闭按钮，WM_NCHITTEST 拖动）
  ├─ H/M/S 输入（三个子窗口 EDIT 控件，停止时可见）
  ├─ 开始/暂停按钮（子窗口 BUTTON，owner-draw 样式）
  ├─ 重置按钮（子窗口 BUTTON，owner-draw 样式）
  └─ 选择音乐按钮（子窗口 BUTTON）
```

### 颜色常量

| 用途 | 颜色 |
|------|------|
| 窗口背景 | `#fffaf3` (255,250,243) |
| 头部区背景 | `#f7ecde` (247,236,222) |
| 主按钮（开始/暂停） | `#d98a5f` (217,138,95) |
| 主按钮 hover | `#c87a4e` |
| 副按钮背景 | `#fbf3e8` (251,243,232) |
| 副按钮边框 | `#e7d8c4` (231,216,196) |
| 大字体时间 | `#3c332b` (60,51,43) |
| 副文字 | `#9a8771` (154,135,113) |

### 窗口尺寸与布局

```
窗口：480 × 360 px，居中显示，圆角 18px
┌─────────────────────────────────────────────┐  ← 头部 60px，#f7ecde
│  ● TickTock                            ✕   │
├─────────────────────────────────────────────┤
│                                             │
│   [时间停止时]  00 ↑  :  05 ↑  :  00 ↑     │  ← SpinBox/Edit，h=40px
│   [运行时]          00 : 05 : 00            │  ← GDI+ 大字体，#3c332b
│                                             │
│   ┌──────── 开 始 ────────┐  ┌─ 重 置 ─┐   │  ← #d98a5f / #fbf3e8
│   └───────────────────────┘  └─────────┘  │
│                🎵 选择音乐                  │
└─────────────────────────────────────────────┘
```

---

## 阶段 0：构建系统

### 已完成
- `CMakeLists.txt` 根文件（spdlog 静态链接）
- `TimerEngineTest.exe` 可独立编译运行

### 待修改（切换到 Win32）
- 移除 CMakeLists.txt 中的 `balloonui_static` 相关内容
- 移除 `src/stdafx.h` 中的 ATL/WTL 依赖
- `src/main.cpp` 改为纯 Win32 入口

---

## 阶段 1：TimerEngine ✅

### 已通过测试（25/25）

| # | 用例 | 结果 |
|---|------|------|
| 1 | 初始状态 Idle，剩余 = 0 | ✅ |
| 2 | SetDuration(0,5,0) 后剩余 = 300 | ✅ |
| 3 | Start() 后状态 = Running | ✅ |
| 4 | 累计 Tick(300000ms) → Finished | ✅ |
| 5 | Tick(1000ms) 减少 1 秒 | ✅ |
| 6 | Pause() 后 Tick 不扣时 | ✅ |
| 7 | Paused 下 Start() 恢复 | ✅ |
| 8 | 任意状态 Reset() → Idle | ✅ |
| 9 | Finished 下 Reset() → Idle | ✅ |
| 10 | Tick 超时后剩余 = 0，不为负 | ✅ |

---

## 阶段 2：AppSettings（配置读写）

### 文件
- `tests/test_app_settings.cpp`（先写，待用户确认）
- `src/AppSettings.h` / `src/AppSettings.cpp`

### 存储
`%APPDATA%\TickTock\settings.ini`，使用 `GetPrivateProfileStringW`

### 待确认测试用例（5 条）

| # | 用例 |
|---|------|
| 1 | 保存 `{hours=1,minutes=30,seconds=0,musicPath="C:/test.mp3"}` 后文件存在 |
| 2 | 读取刚保存的文件，所有字段与保存值一致 |
| 3 | 文件不存在时读取，返回默认值 `{0,5,0,""}` |
| 4 | 保存空 musicPath，读取后为空字符串，不崩溃 |
| 5 | 连续保存两次，第二次正确覆盖第一次 |

---

## 阶段 3：AudioPlayer（音频播放）

### 文件
- `tests/test_audio_player.cpp`（先写，待用户确认）
- `src/AudioPlayer.h` / `src/AudioPlayer.cpp`

### 技术
Windows MCI（`mciSendString`），链接 `winmm.lib`，支持 WAV/MP3/WMA

### 待确认测试用例（6 条）

| # | 用例 |
|---|------|
| 1 | `LoadFile("")` 返回 false，不崩溃 |
| 2 | `LoadFile("不存在路径.mp3")` 返回 false |
| 3 | `Stop()` 未加载文件时调用，不崩溃 |
| 4 | `Play()` 未加载文件时调用，不崩溃 |
| 5 | 真实 `.wav`：`LoadFile()` 返回 true，`Play()` 可播放（人工验证） |
| 6 | `Play()` 后 `Stop()`，再 `Play()` 可重播，不崩溃 |

---

## 阶段 4+5：主窗口 UI（原生 Win32）

### 文件
- `src/MainWindow.h` / `src/MainWindow.cpp`（主窗口 WndProc）
- `src/CountdownPainter.h` / `src/CountdownPainter.cpp`（GDI+ 绘制逻辑）
- `src/res/app.rc`（图标资源）

### 技术细节

| 问题 | 方案 |
|------|------|
| 无边框 + 可拖动 | `WS_POPUP` + `WM_NCHITTEST` 返回 `HTCAPTION` |
| 圆角窗口 | `DwmSetWindowAttribute(DWMWA_WINDOW_CORNER_PREFERENCE, DWMWCP_ROUND)` |
| 背景自绘 | `WM_ERASEBKGND` 返回 TRUE + `WM_PAINT` 用 GDI+ 填色 |
| 输入控件 | `CreateWindowEx(WC_EDIT)` 子窗口，停止时可见，运行时隐藏 |
| 按钮自绘 | `BS_OWNERDRAW` + `WM_DRAWITEM` 自绘圆角填色 |
| 定时器 | `SetTimer(100ms)` → `WM_TIMER` → `TimerEngine::Tick` |
| 音频触发 | `TimerEngine::Tick()` 返回 true → 调 `AudioPlayer::Play()` |

### 待确认测试用例（8 条，人工验证）

| # | 用例 |
|---|------|
| 1 | 启动 → 窗口居中，背景色 `#fffaf3`，头部/按钮可见 |
| 2 | 修改 H/M/S 输入框 → 时间显示同步更新 |
| 3 | 点击"开始" → 按钮变"暂停"，时间开始倒数 |
| 4 | 点击"暂停" → 倒计时停止，按钮变回"开始" |
| 5 | 点击"重置" → 时间恢复初始值 |
| 6 | 倒计时归零 → 自动播放已选音乐 |
| 7 | 点击"选择音乐" → 文件对话框弹出，选择后文件名显示 |
| 8 | 关闭再打开 → 上次设置的时间和音乐路径被恢复 |
