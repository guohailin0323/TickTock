# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

TickTock 是一个 Windows 桌面倒计时软件，使用 C++17 + Win32 开发。主要功能：

- 支持小时 / 分钟 / 秒自定义输入
- 开始 / 重置控制
- 倒计时结束时播放用户自定义音乐

UI 设计稿：`design/TickTock.html`（HTML 导出，用浏览器打开预览）。

**平台要求**：Windows 7+，x86/x64；编译器 Visual Studio 2022。

## 构建

项目使用 CMake，C++17，Visual Studio 2022，目标 Windows 7+（x64）。

```cmake
cmake_minimum_required(VERSION 3.16)
project(TickTock LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third-party/spdlog-1.17.0)   # 静态库，无 DLL

add_executable(TickTock WIN32 src/main.cpp ...)
target_link_libraries(TickTock PRIVATE spdlog::spdlog gdiplus winmm comctl32)
```

最终产物只有 `TickTock.exe`，无第三方 DLL 依赖。

## 关键依赖

### UI 技术栈：原生 Win32 + GDI+

界面直接用 Win32 API 实现，不依赖任何第三方 UI 框架：

- 窗口创建：`CreateWindowExW`（`WS_POPUP` 无边框 + DWM 圆角）
- 自绘：GDI+（`Gdiplus`）做抗锯齿文字、圆角矩形、渐变填充
- 输入控件：系统原生 `EDIT`（H/M/S 输入）、`BUTTON`（开始/重置/选音乐）
- 消息处理：标准 `WndProc` / `WNDCLASSEXW`
- 链接库：`gdiplus.lib`、`winmm.lib`（MCI 音频）、`comctl32.lib`

`third-party/balloonui/` 保留但**不参与编译**，仅作参考。

### spdlog（`third-party/spdlog-1.17.0/`）

项目唯一日志库（禁止自造 logger）。头文件在 `include/spdlog/`，用法示例见 `example/example.cpp`。

---

## 编码规范（适用范围：TickTock 自有代码；`third-party/` 跳过）

以下规则对 TickTock 自己编写的所有 C++ 源码强制生效。`third-party/` 下的 vendored 库保留各自风格，不要按这些规则修改它们。

1. **花括号布局**
   - `if / for / while / switch / 函数 / 类 / 结构体` 的左花括号 `{` 单独占一行。
   - **唯一例外是 `namespace`**：`{` 必须与 `namespace` 关键字同一行；多层 namespace 写在同一行（例：`namespace TickTock { namespace net {`）；namespace 体内**不缩进**。
2. **禁止魔数**。所有字面量数字（除了 `0`、`1`、`-1` 这类显而易见的边界）必须命名为 `constexpr` 常量或宏，并写注释说明含义、单位、出处。
3. **enum 每个枚举值都要逐条中文注释**，写清楚什么场景下取该值。
4. **类成员变量以 `m_` 开头**，每个成员必须有中文注释说明用途、单位、生命周期。
5. **新建类必须有类用途注释**；所有 `public` 方法必须有中文注释，逐个说明：方法用途、每个参数的含义/单位/取值范围、返回值含义、可能的副作用。
6. **每个 `.h` / `.cpp` 文件头**必须包含：开发者 `balloonwj@qq.com`、开发时间（创建日期，`YYYY-MM-DD`）、本文件用途与典型用法描述。
7. **注释量目标 ≥ 30%**，但**严禁废话注释**（`i++ // 自增`、`return x; // 返回 x` 这类不要写）。判断标准是"删掉这条注释后，未来读代码的人会不会变难理解" —— 答案为"会"则保留，否则删。
8. **`if / for / while` 的语句体即便只有一行也必须独立成行**（不允许 `if (x) foo();` 写在一行）。**不强制加大括号** —— 允许 `if (x)\n    foo();`，但若加大括号则按规则 1 走。
9. **所有注释一律使用中文。**
10. **关键路径必须打印日志**。覆盖范围：
    - 网络收发的每条消息（消息类型、来源/目的、关键字段）
    - 回调入口与异步任务边界（哪个回调被触发、入参关键值）
    - 对象/会话的生命周期跃迁（创建、销毁、状态机迁移）
    - 所有错误分支（异常、非预期返回值、超时、断连）
    - 配置加载、启动、优雅关闭

    **目的：出问题时优先靠日志输出快速定位，再回到代码看实现。** 判定标准是"打开 INFO 级日志，能否不看代码就复盘一次完整请求路径" —— 答案为"不能"则补日志。

    统一日志库：**spdlog**（C++ 项目）。级别按 TRACE / DEBUG / INFO / WARN / ERROR 五档划分；INFO 是部署默认级别，TRACE/DEBUG 仅排障时开启。**禁止自己造 logger 轮子。** 与 测试规范 §3 互为补充：本条管"代码里要埋哪些日志"，§3 管"上线前要验证日志够不够还原路径"。

11. **少用 `auto`**。默认写明类型；仅以下场景允许 `auto`：
    - 迭代器（`auto it = container.begin()`）
    - lambda 对象（虽然按规则 §12 lambda 本身也要少用）
    - 范围 for（`for (auto &x : container)`）
    - 结构化绑定（`auto [k, v] = pair`）
    - `std::make_shared` / `std::make_unique` 的返回值（左边写出来等于复读）

    其它场景请写出完整类型。**理由**：review 时一眼能看出"这是哪个类型"，省去对着函数签名反查的成本；也避免隐式类型推导带来的微妙坑（如 `auto x = container[0]` 不知是值还是引用）。

12. **尽量不用 lambda 表达式**。优先把回调 / 谓词 / 信号槽函数写成具名函数或成员函数。**例外**仅限：
    - 一两行的极短回调，且没有 capture，写具名函数反而拖沓的（如 `std::sort(v.begin(), v.end(), [](const A &a, const A &b) { return a.id < b.id; })`）。
    - Qt `QObject::connect` 把信号转发到当前对象的私有方法时的转发壳。

    **理由**：lambda 容易藏 capture-by-reference 生命周期 bug（`[&]` 捕获到栈对象、回调晚于对象析构触发）；崩溃堆栈里 lambda 没名字，难定位；review 时也很难快速判断它"被谁调"。具名函数 + 显式参数，所有这些问题都没有。

13. **命名优先用通俗、自解释的英文词汇**。**禁止凭空发明缩写或代号**充当类型名/枚举值/字段名/方法名/变量名。
    **遇到无合适英文词的场景：先问用户，不要自己拍。**

    **理由**：代码是给人读的，包括从未参与设计讨论的未来 reviewer。`L1Direct` 这种命名把"哪条路径"和"为什么叫 L1"两层语义都藏进了一个名字里 —— 读者得先去翻 plan / 设计文档才看得懂代码。直接命名 `Direct` 则代码本身自解释。代号类命名是"作者偷懒、读者付费"的反模式。

14. **统一用 C++17 标准开发**。所有 TickTock 自有 C++ 工程一律以 C++17 为目标标准：CMake 里写 `set(CMAKE_CXX_STANDARD 17)` + `set(CMAKE_CXX_STANDARD_REQUIRED ON)`，不要降到 C++14 以下，也不要随手用 C++20/23 特性（如 concepts、`std::format`、`<ranges>`、协程）。**理由**：固定一个标准基线，避免不同模块各用各的标准导致 ABI / 编译器兼容问题；C++17 的 `std::optional` / `std::variant` / `std::string_view` / 结构化绑定 / `if constexpr` 已够用，新特性收益不足以打破基线统一。确有 C++20 特性强需求时**先和用户确认**再升基线。`third-party/` 跳过此条（各库保留自己的标准设置）。

15. **`switch / case` 排版**：
    - **每个 `case` 的语句体必须独立成行**，禁止 `case X: return y;` 这种 case 标签与语句挤在同一行（这是 §8"语句体独立成行"在 switch 上的延伸）。`return` / 赋值 / 函数调用都另起一行：

      ```cpp
      case PinnedRole:
          // 是否置顶: 行置顶徽章 + devicesInGroup 把它排到本组最前
          return m_pinned.contains(d.uuid);
      ```

    - **每个 `case`（含 `default`）都要有中文注释说明该分支的用途 / 取该值的场景**（与 §3"枚举逐条注释"同源）。注释写"这个分支在什么场景命中、为什么这么处理"，不要写复述标签名的废话（§7）；role→字段 这类 dispatch switch，注释落到"该字段给谁用 / 什么场景读"。
    - 多个 `case` 共享同一段逻辑（穿透 fall-through）时，在第一个 `case` 上注释清楚"这几个值走同一处理"，并保留显式 `// fall through` 标记。

    **理由**：switch 往往是状态机迁移、消息分发、role 取值这类关键路径的落点，一行挤死的写法在 review 和加断点时都难拆；逐 case 注释能让读者不跳转定义就看懂每个分支的业务含义。

文件头注释样板：

```cpp
/**
 * @file    DeviceManager.cpp
 * @brief   设备列表管理：维护在线/离线 agent 集合、心跳、改名/分组等
 *          状态变更，向 UI 层广播 DUIN_DEVICE_CHANGED 事件。
 * @author  hailin@qq.com
 * @date    2026-05-20
 */
```

## 测试规范

1. **不确定的功能必须先跑测试**。任何"我以为它是这样工作的"假设，在动代码前先用测试或 probe 验证现状；测试跑通确认基线后，才允许改实现。这一条与 `plans/features.md` §24 的"任何推测先 probe 实证再写主程序"同源，强制执行。
2. **每个新功能必须配测试用例**。测试用例设计先与用户确认，确认通过后再写实现——不要直接埋头实现完才补测试。
3. **关键路径必须有日志**（信号链路、回调入口、生命周期跃迁、跨进程消息、错误分支）。日志要能让人不看代码就大致复盘问题路径。
4. **"据推测"与证据规则——同时约束代码注释与对话回答**：
   - 任何推测性表述（包括对用户的回答）必须以"据推测"开头，并在随后给出验证步骤或承诺验证。
   - 任何结论性表述必须附证据：commit hash、文件路径:行号、命令输出、文档链接、运行截图任择其一。
   - 这条规则对未来 Claude 实例同样生效——写代码注释、写 PR 描述、回答用户问题都要遵守。
   
## 交互规范（Claude ↔ 用户对话）

1. **和用户交互时尽量用通俗易懂的语句，不要使用自造的术语或者缩写**。回答问题、解释方案、写 commit / PR 描述、问澄清问题时，用大白话把事情讲清楚：
   - **不要**自创缩写 / 代号（如 "L1/L2/L3"、"H1/D2"、"P5.3.1" 这类内部编号当名词直接甩给用户时要先解释它指什么）。
   - **不要**堆砌行话黑话；非通用术语（KCP、NAT、hole-punch 等）第一次出现时用一句话解释含义。
   - 优先用"直连 / P2P 打洞 / 中转"这种用户一眼能懂的说法，而不是项目内部代号。
   - 这一条是 代码规范 §13（命名通俗化）在"对话层"的延伸：§13 管代码里的标识符，本条管我对你说话的措辞。**理由**：沟通成本最低的表达才是好表达；让用户为了看懂回答还得反查内部编号，是把作者的偷懒转嫁给读者。


## 日志规范

> **核心原则：关键路径一定要打印日志。** 在生产环境出现问题时，第一手信息来源是日志；出了问题要能通过日志快速定位，而不是靠猜。日志覆盖不到的地方，问题就无从复盘。

**日志框架**：项目统一使用 `spdlog`（`third-party/spdlog-1.17.0/`），CMake target 为 `spdlog::spdlog`，以静态库方式链接，不产生额外 DLL。


### 日志级别

| 级别 | 使用场景 |
|------|---------|
| `spdlog::info` | 正常业务事件（连接建立、注册成功、打洞完成） |
| `spdlog::warn` | 可恢复的异常（重复注册、打洞超时、速率限制触发、对端提前断开） |
| `spdlog::error` | 不可恢复的错误（socket 错误、协议解析失败、资源分配失败） |
| `spdlog::debug` | 开发期详细追踪（帧内容、状态机每步跃迁），Release 构建可关闭 |

### 日志内容规范
每条日志必须包含：**操作类型标签 + 关键标识符**（设备ID / UUID / 地址）**+ 结果或原因**。