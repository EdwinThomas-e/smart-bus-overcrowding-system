@echo off
title Smart Bus Overcrowding Simulation - MITS Kochi
echo ====================================================================
echo   SMART BUS OVERCROWDING DETECTION & ALERT SYSTEM
echo   MITS Kochi - Dept of Artificial Intelligence & Data Science
echo   Team: Edwin Thomas (29), Adwaith Vimal (12), 
echo         Bevan Bince (26), Joswin Thomas (40)
echo ====================================================================
echo.

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

set AGY_NODE="%APPDATA%\Antigravity\bin\agy-node.cmd"

if exist %AGY_NODE% (
    echo [INFO] Starting Node.js simulation server via Antigravity runtime...
    start "" cmd /c "%AGY_NODE% server.js"
    timeout /t 2 /nobreak > nul
    start http://localhost:3000
) else (
    where node >nul 2>nul
    if %ERRORLEVEL% equ 0 (
        echo [INFO] Starting Node.js simulation server...
        start "" cmd /c "node server.js"
        timeout /t 2 /nobreak > nul
        start http://localhost:3000
    ) else (
        echo [INFO] Opening standalone web simulation directly in default browser...
        start "" "%SCRIPT_DIR%public\index.html"
    )
)

echo.
echo [SUCCESS] Simulation is launching in your default web browser!
echo If it does not open automatically, browse to:
echo   http://localhost:3000  or  open public\index.html
echo.
pause
