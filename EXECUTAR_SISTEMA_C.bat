@echo off
title CyberHardware E-commerce - Sistema Integrado em C (PIM II)
cd /d "%~dp0"

set "GCC_BIN="
if exist "C:\Program Files\JetBrains\CLion 2026.1.4\bin\mingw\bin\gcc.exe" (
    set "GCC_BIN=C:\Program Files\JetBrains\CLion 2026.1.4\bin\mingw\bin"
) else if exist "C:\Program Files\JetBrains\CLion 2026.2\bin\mingw\bin\gcc.exe" (
    set "GCC_BIN=C:\Program Files\JetBrains\CLion 2026.2\bin\mingw\bin"
)

if defined GCC_BIN (
    set "PATH=%GCC_BIN%;%PATH%"
    echo [Compilando sistema_loja.c com GCC e Winsock2...]
    gcc -Wall -Wextra -O2 -static sistema_loja.c -o sistema_loja.exe -lws2_32 -lshell32
    if errorlevel 1 (
        echo [!] Falha na compilacao.
        pause
        exit /b 1
    )
) else (
    where gcc >nul 2>&1
    if not errorlevel 1 (
        echo [Compilando sistema_loja.c com GCC do PATH...]
        gcc -Wall -Wextra -O2 -static sistema_loja.c -o sistema_loja.exe -lws2_32 -lshell32
        if errorlevel 1 (
            echo [!] Falha na compilacao.
            pause
            exit /b 1
        )
    )
)

echo.
cls
sistema_loja.exe
pause
