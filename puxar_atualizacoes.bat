@echo off
title Baixar Atualizacoes do GitHub
cd /d "%~dp0"

echo =========================================================
echo    PUXANDO ULTIMAS ATUALIZACOES DA NUVEM (GITHUB)
echo =========================================================
echo.

git pull origin main

echo.
if %errorlevel% equ 0 (
    echo [OK] Projeto atualizado com sucesso!
) else (
    echo [!] Verifique sua conexao ou configuracao do repositorio.
)
echo.
pause
