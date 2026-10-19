# ParquetPlot

Desktop application for visualizing and annotating Parquet signal data on Linux and Windows.

## Features

- Plot any number of numeric signal and target columns
- Automatic high-contrast colors with per-series customization
- Stacked plot panels with a shared X-axis and independent Y-axis scaling
- Per-column panel assignment, line/step/point styles, multipliers, and offsets
- Zoom, pan, hover coordinates, and marker annotations
- Smooth handling of files with up to 350k rows

## Installation

Each published GitHub Release contains ready-to-run x86-64 packages:

- **Linux:** download the AppImage, then run:

  ```bash
  chmod +x ParquetPlot-Linux-x86_64.AppImage
  ./ParquetPlot-Linux-x86_64.AppImage
  ```

- **Windows (installed app):** download and run
  `ParquetPlot-Windows-x64-Setup.exe`. Approve the administrator prompt to install
  for all users in Program Files. Launch **ParquetPlot** from the Start menu or
  Windows Search. The installer also offers a desktop shortcut. To uninstall,
  use **Settings > Apps > Installed apps > ParquetPlot > Uninstall**. Run a newer
  installer to update the existing installation.
- **Windows (portable):** extract `ParquetPlot-Windows-x64.zip` and run
  `ParquetPlot.exe`.

Runtime dependencies are included in all packages.

### Open Parquet files by double-clicking

- **Linux:** integrate the AppImage, then select ParquetPlot from the file's
  **Open With** dialog. Source installs include the desktop and MIME metadata.
- **Windows:** the installer registers ParquetPlot as an **Open with** option
  for `.parquet` files. Use **Open with > Choose another app**, select
  **ParquetPlot**, and enable **Always use this app** to make it the default.
  For the portable version, browse to `ParquetPlot.exe` in that dialog.

Files can also be opened from a terminal:

```bash
./build/ParquetPlot example.parquet
```

## Build from source

Requirements:

- Qt 6
- Apache Arrow C++
- Parquet C++

QCustomPlot 2.1.1 is downloaded and checksum-verified during configuration.
For an offline build, install QCustomPlot and use
`-DPARQUETPLOT_USE_SYSTEM_QCUSTOMPLOT=ON`.

Ubuntu/Debian, after enabling the [Apache Arrow repository](https://arrow.apache.org/install/):

```bash
sudo apt-get install qt6-base-dev libarrow-dev libparquet-dev
```

Fedora/RHEL:

```bash
sudo dnf install qt6-qtbase-devel libarrow-devel parquet-libs-devel
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Build the Windows installer

The release workflow bundles the Windows executable, Qt plugins, runtime DLLs,
and dependency licenses in `dist/ParquetPlot`, then builds both the installer
and portable ZIP. It tests installation, shortcuts, launching the installed app,
and uninstallation before uploading the packages to the release.

To compile the installer locally on Windows, prepare the same bundle and install
[Inno Setup 6](https://jrsoftware.org/isinfo.php), then run in PowerShell from the
repository root (adjust the version as needed):

```powershell
& "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" /DAppVersion=1.0.0 "/DAppSourceDir=$PWD\dist\ParquetPlot" "/O$PWD" packaging/windows/ParquetPlot.iss
```

## Usage

1. Open or double-click a `.parquet` file.
2. Choose the X-axis, signals, targets, and optional custom colors.
3. Scroll to zoom, drag to pan, or hover to inspect coordinates.
4. Use **Select Columns** to change the plot while keeping its horizontal view.

FP/FN markers, marker export, Parquet saving, and target-height controls share
an annotation toolbar row. Click **Annotations** in the plot toolbar to collapse
or expand it. The regular **Add Marker** tool stays in the plot toolbar.

### Stacked plots

In **Select Columns**, increase **Stacked panels** and assign each signal or
target to a **Panel** (numbered from top to bottom). Add the same column more
than once to show it in different panels or with different display settings.

Each panel has its own Y label, linear or logarithmic scale, and automatic or
fixed Y range. Set **Height weight** to give a panel more or less space relative
to the others. Only the bottom panel shows X-axis labels; horizontal zoom and
pan are synchronized across all panels.

For signals, choose **Line**, **Step**, or **Points** and optionally transform
displayed values with a multiplier and offset: `value × multiplier + offset`.
For targets, **Height ×** multiplies the toolbar's target height for that column.
These display settings do not modify the source data. Logarithmic axes display
positive values and require positive fixed bounds.

Box/Y zoom and mouse-wheel Y zoom use the panel under the pointer. The arrow
keys pan horizontally across all panels and vertically in the last active
panel. **Fit** restores automatic Y ranges or the configured fixed bounds;
**Undo** restores the shared X range and all panel Y ranges. Applying column
settings recalculates Y ranges and clears markers and zoom history.

To run the plot interaction tests:

```bash
cmake -S . -B build -DPARQUETPLOT_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Data Format

Any numeric columns can be selected. These names are preselected when present:

- `raw_idx` (numeric): sample index
- `ecg1_raw`: ECG signal
- `puls_raw`: pulse signal
- `target`: annotation flags

Target values equal to `1` are displayed as vertical lines.

## License

ParquetPlot is licensed under the GNU General Public License v3.0 only.
See [LICENSE](LICENSE) for the full license text.

Bundled dependency licenses are listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
