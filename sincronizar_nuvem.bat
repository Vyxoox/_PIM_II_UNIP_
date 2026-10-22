@echo off
title Sincronizar Projeto com GitHub
cd /d "%~dp0"

echo =========================================================
echo    ENVIANDO PROJETO E DECISOES PARA O GITHUB (NUVEM)
echo =========================================================
echo.

git add .
set "msg=update: %date% %time%"
git commit -m "%msg%"
git push origin main

echo.
if %errorlevel% equ 0 (
    echo [OK] Sincronizacao com a nuvem concluida com sucesso!
) else (
    echo [!] Verifique se o repositorio remoto esta configurado no GitHub.
)
echo.
pause
