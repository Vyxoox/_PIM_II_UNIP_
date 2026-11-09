@echo off
title CyberHardware E-commerce - Sistema Integrado em C (PIM II)
cd /d "%~dp0"

set "GCC_PATH=C:\Program Files\JetBrains\CLion 2026.2\bin\mingw\bin\gcc.exe"

if exist "%GCC_PATH%" (
    echo [Compilando sistema_loja.c com GCC e Winsock2...]
    "%GCC_PATH%" -Wall -O2 sistema_loja.c -o sistema_loja.exe -lws2_32 -lshell32
    if %errorlevel% neq 0 (
        echo [!] Falha na compilacao.
        pause
        exit /b %errorlevel%
    )
)

echo.
cls
sistema_loja.exe
pause
