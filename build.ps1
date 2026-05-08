#Requires -Version 5.1
<#
.SYNOPSIS
  Fresh-configure and build WindroseSchema with MSVC (VsDevCmd) + Ninja.

.EXAMPLE
  .\build.ps1
#>
$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$BuildDir = Join-Path $RepoRoot 'cmake-build-debug-visual-studio'
$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat'

if (-not (Test-Path -LiteralPath $VsDevCmd)) {
    Write-Error "VsDevCmd.bat not found: $VsDevCmd`nEdit this script if VS is installed elsewhere."
}

# Mirrors: cmd /c "call ""...VsDevCmd.bat"" -arch=x64 ... && cmake ... && cmake --build ..."
# PowerShell: "" inside a double-quoted string becomes one " for cmd.
$CmdLine = "call ""$VsDevCmd"" -arch=x64 -host_arch=x64 && cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE=Game__Shipping__Win64 -S ""$RepoRoot"" -B ""$BuildDir"" && cmake --build ""$BuildDir"" --target WindroseSchema"

# Pass one argument to cmd /c — do not wrap $CmdLine in extra `"...`" or the line starts with a stray quote and CMD mis-parses paths.
cmd.exe /c "$CmdLine"
exit $LASTEXITCODE
