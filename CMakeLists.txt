cmake_minimum_required(VERSION 3.16)
project(TickTock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── spdlog（静态库，不产生 DLL）─────────────────────────────────────
add_subdirectory(third-party/spdlog-1.17.0)

# ─── TimerEngine 单元测试（纯控制台，无 Windows GUI 依赖）────────────
enable_testing()

add_executable(TimerEngineTest
    tests/test_timer_engine.cpp
    src/TimerEngine.cpp)

target_include_directories(TimerEngineTest PRIVATE
    "${CMAKE_SOURCE_DIR}/src")

target_compile_definitions(TimerEngineTest PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE _UNICODE)

target_link_libraries(TimerEngineTest PRIVATE spdlog::spdlog)

if(MSVC)
    target_compile_options(TimerEngineTest PRIVATE /W3 /MP /utf-8)
endif()

add_test(NAME TimerEngine COMMAND TimerEngineTest)

# ─── AppSettings 单元测试────────────────────────────────────────────
add_executable(AppSettingsTest
    tests/test_app_settings.cpp
    src/AppSettings.cpp)

target_include_directories(AppSettingsTest PRIVATE
    "${CMAKE_SOURCE_DIR}/src")

target_compile_definitions(AppSettingsTest PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE _UNICODE
    WIN32 _WINDOWS)

target_link_libraries(AppSettingsTest PRIVATE spdlog::spdlog)

if(MSVC)
    target_compile_options(AppSettingsTest PRIVATE /W3 /MP /utf-8)
endif()

add_test(NAME AppSettings COMMAND AppSettingsTest)

# ─── AudioPlayer 单元测试───────────────────────────────────────────
add_executable(AudioPlayerTest
    tests/test_audio_player.cpp
    src/AudioPlayer.cpp)

target_include_directories(AudioPlayerTest PRIVATE
    "${CMAKE_SOURCE_DIR}/src")

target_compile_definitions(AudioPlayerTest PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE _UNICODE
    WIN32 _WINDOWS)

target_link_libraries(AudioPlayerTest PRIVATE spdlog::spdlog winmm)

if(MSVC)
    target_compile_options(AudioPlayerTest PRIVATE /W3 /MP /utf-8)
endif()

add_test(NAME AudioPlayer COMMAND AudioPlayerTest)

# ─── TickTock 主程序（WIN32，原生 Win32 + GDI+）─────────────────────
add_executable(TickTock WIN32
    src/main.cpp
    src/TimerEngine.cpp
    src/AppSettings.cpp
    src/AudioPlayer.cpp
    src/MainWindow.cpp
    src/CountdownPainter.cpp
    src/res/app.rc)

target_include_directories(TickTock PRIVATE
    "${CMAKE_SOURCE_DIR}/src")

target_compile_definitions(TickTock PRIVATE
    _CRT_SECURE_NO_WARNINGS
    WIN32 _WINDOWS UNICODE _UNICODE
    WINVER=0x0601 _WIN32_WINNT=0x0601)

target_link_libraries(TickTock PRIVATE
    spdlog::spdlog
    gdiplus
    winmm
    comctl32
    dwmapi)

if(MSVC)
    target_compile_options(TickTock PRIVATE /W3 /MP /utf-8)
endif()
