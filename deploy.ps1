<#
.SYNOPSIS
  Deploy WindroseSchema C++ mod into a UE4SS Mods tree.

.DESCRIPTION
  - Copies the built WindroseSchema.dll to:
      <Ue4ssModsRoot>\WindroseSchema\dlls\main.dll
  - Creates Mods\WindroseSchema\config and Mods\WindroseSchema\mods if missing.
  - Writes default config.json only if the file does not exist (unless -ForceConfig).
  - Optionally creates enabled.txt so UE4SS starts the mod without editing mods.txt.

  UE4SS loads C++ mods from Mods\<Name>\dlls\main.dll (see RE-UE4SS CppMod.cpp).

  You still need a valid Mappings.usmap next to UE4SS.dll for this game build (not deployed here).

.PARAMETER Ue4ssModsRoot
  Path to the UE4SS "Mods" folder (contains per-mod subfolders).

.PARAMETER BuiltDll
  Full path to WindroseSchema.dll. If omitted, the script searches common CMake outputs under the repo.

.PARAMETER BuildRoot
  Repo-relative build directory to search first (default: build).

.PARAMETER SkipEnabledTxt
  If set, does not create WindroseSchema\enabled.txt (use when you enable the mod only via mods.txt).

.PARAMETER ForceConfig
  Overwrites config\config.json with defaults even if it already exists.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $false)]
    [string] $Ue4ssModsRoot = 'G:\Steam\steamapps\common\Windrose\R5\Binaries\Win64\ue4ss\Mods',

    [Parameter(Mandatory = $false)]
    [string] $BuiltDll = '',

    [Parameter(Mandatory = $false)]
    [string] $BuildRoot = 'build',

    [Parameter(Mandatory = $false)]
    [switch] $SkipEnabledTxt,

    [Parameter(Mandatory = $false)]
    [switch] $ForceConfig
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
if (-not $repoRoot) { $repoRoot = (Get-Location).Path }

function Resolve-BuiltDll {
    param([string] $Explicit, [string] $Root, [string] $BuildRootName)

    if ($Explicit -and (Test-Path -LiteralPath $Explicit)) {
        return (Resolve-Path -LiteralPath $Explicit).Path
    }

    $candidates = @(
        (Join-Path $Root (Join-Path $BuildRootName 'Release\WindroseSchema.dll'))
        (Join-Path $Root (Join-Path $BuildRootName 'Debug\WindroseSchema.dll'))
        (Join-Path $Root (Join-Path $BuildRootName 'RelWithDebInfo\WindroseSchema.dll'))
        (Join-Path $Root (Join-Path $BuildRootName 'Game__Shipping__Win64\WindroseSchema.dll'))
        (Join-Path $Root 'cmake-build-debug-visual-studio\WindroseSchema.dll')
        (Join-Path $Root 'out\build\x64-Release\bin\WindroseSchema.dll')
    ) | Select-Object -Unique

    foreach ($p in $candidates) {
        if (Test-Path -LiteralPath $p) { return (Resolve-Path -LiteralPath $p).Path }
    }

    return $null
}

$dllSource = Resolve-BuiltDll -Explicit $BuiltDll -Root $repoRoot -BuildRootName $BuildRoot
if (-not $dllSource) {
    throw @"
Could not find WindroseSchema.dll.
  Pass -BuiltDll 'C:\path\to\WindroseSchema.dll', or build the target first.
  Searched under: $(Join-Path $repoRoot $BuildRoot) (Release/Debug/RelWithDebInfo/Game__Shipping__Win64),
  cmake-build-debug-visual-studio, out\build\x64-Release\bin.
"@
}

if (-not (Test-Path -LiteralPath $Ue4ssModsRoot)) {
    throw "UE4SS Mods folder does not exist: $Ue4ssModsRoot"
}

$modRoot = Join-Path $Ue4ssModsRoot 'WindroseSchema'
$dllsDir = Join-Path $modRoot 'dlls'
$configDir = Join-Path $modRoot 'config'
$modsDir = Join-Path $modRoot 'mods'
$mainDll = Join-Path $dllsDir 'main.dll'
$configFile = Join-Path $configDir 'config.json'
$enabledFile = Join-Path $modRoot 'enabled.txt'

New-Item -ItemType Directory -Path $dllsDir -Force | Out-Null
New-Item -ItemType Directory -Path $configDir -Force | Out-Null
New-Item -ItemType Directory -Path $modsDir -Force | Out-Null

Copy-Item -LiteralPath $dllSource -Destination $mainDll -Force

$defaultConfig = @'
{
  "languageOverride": "",
  "enableAutoReload": false,
  "enableDebugLogging": false
}
'@

if ($ForceConfig -or -not (Test-Path -LiteralPath $configFile)) {
    # UTF-8 without BOM — glaze JSON rejects a leading BOM (shows as "expected_brace" at index 0).
    $utf8NoBom = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($configFile, $defaultConfig.TrimEnd(), $utf8NoBom)
    Write-Host "Wrote config: $configFile"
} else {
    Write-Host "Left existing config unchanged: $configFile"
}

if (-not $SkipEnabledTxt) {
    if (-not (Test-Path -LiteralPath $enabledFile)) {
        New-Item -ItemType File -Path $enabledFile -Force | Out-Null
        Write-Host "Created enabled.txt: $enabledFile"
    } else {
        Write-Host "enabled.txt already exists: $enabledFile"
    }
}

$modsKeep = Join-Path $modsDir '.gitkeep'
if (-not (Test-Path -LiteralPath $modsKeep)) {
    New-Item -ItemType File -Path $modsKeep -Force | Out-Null
}

Write-Host ""
Write-Host "Deploy complete."
Write-Host "  Source DLL: $dllSource"
Write-Host "  Target:     $mainDll"
Write-Host "  Config:     $configFile"
Write-Host "  Packs dir:  $modsDir  (add mods/<PackName>/raw|blueprints|enums|resources|paks)"
Write-Host ""
Write-Host "Reminder: Mappings.usmap for this Windrose build must sit next to UE4SS.dll (not in Mods)."
Write-Host "If you manage load order via mods.txt only, pass -SkipEnabledTxt to avoid creating enabled.txt."
