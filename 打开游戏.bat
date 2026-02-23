@echo off
chcp 65001 >nul
title Minecraft Clone

echo 🎮
echo 欢迎来到 Minecraft Clone!
echo 游戏启动中，请稍候...
echo.

:: 如果第一次运行，自动编译
if not exist "build\minecraft.exe" (
    echo 第一次运行，正在编译游戏...
    echo 这可能需要几分钟，请耐心等待...
    echo.

    if not exist "build" mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" >nul 2>nul
    cmake --build . >nul

    if errorlevel 1 (
        echo 编译失败，请检查是否安装了必要的工具。
        echo 需要: CMake, MinGW, GLFW
        pause
        exit
    )

    cd ..
    echo 编译完成!
    echo.
)

:: 启动游戏
echo 游戏启动!
echo 控制: WASD 移动, 鼠标视角, ESC 退出
echo 玩的开心! 😊
echo.

timeout /t 2 /nobreak >nul
start "" "build\minecraft.exe"

exit