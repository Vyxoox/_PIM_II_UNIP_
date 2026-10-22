@echo off
title CYBERSTORE - Servidor C E-commerce
cd /d "%~dp0"

echo =========================================================
echo    INICIANDO CYBERSTORE - E-COMMERCE NATIVO EM C
echo =========================================================
echo.

if not exist "loja.exe" (
    echo [!] Executavel loja.exe nao encontrado.
    echo Compilando codigo-fonte em C...
    call compilar_e_executar.bat
    exit /b
)

echo [+] Iniciando loja.exe na porta 8080...
echo [+] Seu navegador sera aberto automaticamente.
echo.
loja.exe
pause
