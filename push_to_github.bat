@echo off
title Push to GitHub - Smart Bus Overcrowding System
cd /d "%~dp0"
echo ====================================================================
echo   PUSHING TO GITHUB: EdwinThomas-e/smart-bus-overcrowding-system
echo ====================================================================
echo.
git push -u origin main
echo.
if %ERRORLEVEL% equ 0 (
    echo ====================================================================
    echo   [SUCCESS] All files successfully uploaded to your GitHub repository!
    echo   Check it out at:
    echo   https://github.com/EdwinThomas-e/smart-bus-overcrowding-system
    echo ====================================================================
) else (
    echo [ERROR] Push encountered an issue. See message above.
)
echo.
pause
