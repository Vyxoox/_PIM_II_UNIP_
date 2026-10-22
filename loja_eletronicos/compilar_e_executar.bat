@echo off
title Compilando e Executando E-commerce em C
cd /d "%~dp0"

echo =========================================================
echo    COMPILADOR MSVC (C/C++) - LOJA ONLINE NATIVA
echo =========================================================
echo.

set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VS_PATH%" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_DIR=%%i"
    )
    if defined VS_DIR (
        set "VS_PATH=%VS_DIR%\VC\Auxiliary\Build\vcvars64.bat"
    )
)

echo [+] Carregando ambiente do compilador MSVC...
call "%VS_PATH%" >nul 2>&1

echo [+] Compilando main.c com otimizacao /O2 e Winsock2...
cl /W3 /O2 /Fe:loja.exe main.c ws2_32.lib shell32.lib

if %errorlevel% neq 0 (
    echo.
    echo [-] ERRO: Falha na compilacao do codigo C!
    pause
    exit /b %errorlevel%
)

echo.
echo [OK] Compilacao concluida com sucesso: loja.exe gerado!
echo.
echo =========================================================
echo Iniciando servidor e abrindo loja no navegador...
echo =========================================================
echo.
loja.exe
pause
