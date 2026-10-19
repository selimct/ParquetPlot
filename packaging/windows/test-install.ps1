$ErrorActionPreference = 'Stop'

$installer = Join-Path $PSScriptRoot '../../ParquetPlot-Windows-x64-Setup.exe'
$installDir = Join-Path $env:ProgramFiles 'ParquetPlot'
$startMenu = Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'ParquetPlot.lnk'
$desktop = Join-Path ([Environment]::GetFolderPath('CommonDesktopDirectory')) 'ParquetPlot.lnk'
$uninstallKey = 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\{FEFE138C-03CA-4777-94C7-06CD60A57628}_is1'
$associationKey = 'HKLM:\Software\Classes\.parquet\OpenWithProgids'
$progIdKey = 'HKLM:\Software\Classes\ParquetPlot.Parquet'
$appPathKey = 'HKLM:\Software\Microsoft\Windows\CurrentVersion\App Paths\ParquetPlot.exe'

foreach ($path in @($installDir, $startMenu, $desktop, $uninstallKey, $progIdKey, $appPathKey)) {
    if (Test-Path -LiteralPath $path) {
        throw "Installer smoke test requires a clean machine; $path already exists"
    }
}

$setup = Start-Process -FilePath $installer -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/TASKS=desktopicon' -Wait -PassThru
if ($setup.ExitCode -ne 0) {
    throw "Installer failed with code $($setup.ExitCode)"
}

try {
    foreach ($path in @("$installDir\ParquetPlot.exe", "$installDir\LICENSE", "$installDir\THIRD_PARTY_NOTICES.md", "$installDir\platforms\qwindows.dll", $startMenu, $desktop, $uninstallKey, $progIdKey, $appPathKey)) {
        if (-not (Test-Path -LiteralPath $path)) {
            throw "Installation is missing $path"
        }
    }
    $entry = Get-ItemProperty -LiteralPath $uninstallKey
    if ($entry.DisplayName -ne 'ParquetPlot' -or -not $entry.UninstallString) {
        throw 'ParquetPlot is not correctly registered in installed apps'
    }
    $association = Get-Item -LiteralPath $associationKey
    if ('ParquetPlot.Parquet' -notin $association.GetValueNames()) {
        throw 'ParquetPlot is missing from Open With for .parquet files'
    }
    $command = (Get-Item -LiteralPath "$progIdKey\shell\open\command").GetValue('')
    if ($command -ne "`"$installDir\ParquetPlot.exe`" `"%1`"") {
        throw 'Open With command does not quote the executable and file paths correctly'
    }

    $shortcut = (New-Object -ComObject WScript.Shell).CreateShortcut($startMenu)
    if ($shortcut.TargetPath -ne "$installDir\ParquetPlot.exe") {
        throw 'Start menu shortcut points to the wrong executable'
    }

    # Launch away from the bundle and without build-tool DLLs on PATH.
    $originalPath = $env:PATH
    $process = $null
    try {
        $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
        $process = Start-Process -FilePath $shortcut.TargetPath -WorkingDirectory $env:TEMP -PassThru
        Start-Sleep -Seconds 5
        if ($process.HasExited) {
            throw "Installed ParquetPlot exited unexpectedly with code $($process.ExitCode)"
        }
    } finally {
        $env:PATH = $originalPath
        if ($process -and -not $process.HasExited) {
            Stop-Process -Id $process.Id
            $process.WaitForExit()
        }
    }
} finally {
    # Inno's original uninstaller waits for its temporary child with /SILENT.
    $uninstall = Start-Process -FilePath "$installDir\unins000.exe" -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART' -Wait -PassThru
    if ($uninstall.ExitCode -ne 0) {
        throw "Uninstaller failed with code $($uninstall.ExitCode)"
    }
}

foreach ($path in @("$installDir\ParquetPlot.exe", $startMenu, $desktop, $uninstallKey, $progIdKey, $appPathKey)) {
    if (Test-Path -LiteralPath $path) {
        throw "Uninstall left behind $path"
    }
}
if ((Test-Path -LiteralPath $associationKey) -and ('ParquetPlot.Parquet' -in (Get-Item -LiteralPath $associationKey).GetValueNames())) {
    throw 'Uninstall left behind the ParquetPlot Open With registration'
}
