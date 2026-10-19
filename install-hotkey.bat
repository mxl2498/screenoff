@echo off
rem Bind ScreenOff.exe to a global hotkey (default Ctrl+Alt+Q).
rem Example: install-hotkey.bat -Hotkey "CTRL+ALT+F12"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install-hotkey.ps1" %*
echo.
pause
