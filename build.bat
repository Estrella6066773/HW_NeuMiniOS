@echo off
REM Windows 构建脚本 - NeuMiniOS
REM 如果已安装 MinGW-w64 或 MSYS2，可以使用此脚本

setlocal

REM 检查 gcc 是否可用
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 gcc 编译器
    echo 请先安装 MinGW-w64 或 MSYS2
    echo.
    echo 安装方法:
    echo 1. 下载 MSYS2: https://www.msys2.org/
    echo 2. 在 MSYS2 终端运行: pacman -S mingw-w64-x86_64-gcc
    echo 3. 将 C:\msys64\mingw64\bin 添加到系统 PATH
    exit /b 1
)

REM 设置编译选项
set CC=gcc
set CFLAGS=-Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L
set INCLUDES=-I./include
set SRCDIR=src
set OBJDIR=obj
set BINDIR=.
set TARGET=neuminios

REM 创建 obj 目录
if not exist %OBJDIR% mkdir %OBJDIR%

REM 编译所有源文件
echo 正在编译源文件...
%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\main.c -o %OBJDIR%\main.o
if %errorlevel% neq 0 goto :error

%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\cli.c -o %OBJDIR%\cli.o
if %errorlevel% neq 0 goto :error

%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\process.c -o %OBJDIR%\process.o
if %errorlevel% neq 0 goto :error

%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\file_system.c -o %OBJDIR%\file_system.o
if %errorlevel% neq 0 goto :error

%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\commands.c -o %OBJDIR%\commands.o
if %errorlevel% neq 0 goto :error

%CC% %CFLAGS% %INCLUDES% -c %SRCDIR%\neuboot.c -o %OBJDIR%\neuboot.o
if %errorlevel% neq 0 goto :error

REM 链接生成可执行文件
echo 正在链接...
%CC% %OBJDIR%\*.o -o %BINDIR%\%TARGET%.exe
if %errorlevel% neq 0 goto :error

echo.
echo 构建完成: %TARGET%.exe
goto :end

:error
echo.
echo 构建失败！
exit /b 1

:end
endlocal

