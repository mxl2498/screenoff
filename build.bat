@echo off
rem Rebuild ScreenOff.exe with TinyCC (downloaded on first run).
setlocal

set "ROOT=%~dp0"
set "TCCDIR=%ROOT%.build\tcc"
set "TCC=%TCCDIR%\tcc.exe"

if not exist "%TCC%" (
    echo Downloading TinyCC ...
    if not exist "%ROOT%.build" mkdir "%ROOT%.build"
    curl -L -o "%ROOT%.build\tcc.zip" https://download-mirror.savannah.gnu.org/releases/tinycc/tcc-0.9.27-win64-bin.zip || goto :fail
    tar -xf "%ROOT%.build\tcc.zip" -C "%ROOT%.build" || goto :fail
    del "%ROOT%.build\tcc.zip"
)

"%TCC%" -Wall "%ROOT%screenoff.c" -o "%ROOT%ScreenOff.exe" -Wl,-subsystem=windows -luser32 -lkernel32 -lmsvcrt
if errorlevel 1 goto :fail

echo Built: %ROOT%ScreenOff.exe
endlocal
exit /b 0

:fail
echo Build failed.
endlocal
exit /b 1
