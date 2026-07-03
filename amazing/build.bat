@echo off
chcp 65001 > nul
echo 正在编译所有 .c 文件...

:: cmd 会自动把 *.c 展开为当前目录所有 C 源文件，无需手动拼接
gcc -g -Wall -I. -o amazing.exe *.c

if %errorlevel% equ 0 (
    echo 编译成功，生成 amazing.exe
) else (
    echo 编译失败
)