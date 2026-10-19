# ScreenOff - install desktop shortcut with a global hotkey.
# Usage: install-hotkey.ps1 [-Hotkey "CTRL+ALT+Q"]
param(
    [string]$Hotkey = 'CTRL+ALT+Q'
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe  = Join-Path $root 'ScreenOff.exe'

if (-not (Test-Path $exe)) {
    Write-Host "ScreenOff.exe not found next to this script: $exe"
    exit 1
}

# Copy to LOCALAPPDATA so the hotkey keeps working if this folder is moved.
$dest = Join-Path $env:LOCALAPPDATA 'ScreenOff'
New-Item -ItemType Directory -Force -Path $dest | Out-Null
Copy-Item $exe (Join-Path $dest 'ScreenOff.exe') -Force

$desktop = [Environment]::GetFolderPath('Desktop')
$lnk     = Join-Path $desktop 'ScreenOff.lnk'

$ws = New-Object -ComObject WScript.Shell
$sc = $ws.CreateShortcut($lnk)
$sc.TargetPath       = Join-Path $dest 'ScreenOff.exe'
$sc.WorkingDirectory = $dest
$sc.Description      = 'Turn off the display'
$sc.Hotkey           = $Hotkey
$sc.Save()

Write-Host ''
Write-Host "Installed."
Write-Host "  Hotkey  : $Hotkey"
Write-Host "  Program : $(Join-Path $dest 'ScreenOff.exe')"
Write-Host "  Shortcut: $lnk"
Write-Host ''
Write-Host 'Note: Windows only honours shortcut hotkeys placed on the Desktop or in the Start Menu.'

exit 0
