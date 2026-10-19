# ParquetPlot

Desktop application for plotting, annotating, and browsing Parquet data on Linux and Windows.

## Features

- Plot any number of numeric signal and target columns
- Automatic high-contrast colors with per-series customization
- Stacked plot panels with a shared X-axis and independent Y-axis scaling
- Per-column panel assignment, line/step/point styles, multipliers, and offsets
- Zoom, pan, hover coordinates, and marker annotations
- Integrated ParquetViewer table browser with a file explorer and multiple file tabs
- Paged or all-rows tables, global search, quick filters, and a visual filter builder
- Switch from a plot to the table at the beginning or at the current X position
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
# Open directly as a table, or open multiple files in table tabs:
./build/ParquetPlot --table example.parquet
./build/ParquetPlot first.parquet second.parquet
```

## Build from source

Requirements:

- Qt 6
- Apache Arrow C++
- Parquet C++
- DuckDB CLI (`duckdb` in `PATH`, or set `-DDUCKDB_EXECUTABLE=/path/to/duckdb`)

The table browser uses the DuckDB CLI for Parquet queries. Source installation
copies it beside ParquetPlot; release packages include it and its license.

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

### Table view

Click **Table** in the toolbar to see the current file as a table, starting at
the left edge of the plot's current X range. Use the arrow beside **Table** to
choose **From beginning** or **From current plot X** explicitly. Click **Plot**
to return to the same zoom, panel ranges, and annotations.

The X jump uses the selected X column and opens at the row with the smallest X
value at or after the plot's left edge. If the view is beyond the data, it uses
the nearest X value. The table keeps source row order and shows all columns and
subsequent rows. Jumping from the plot or choosing **From beginning** clears
table search and filters so they cannot hide the requested row. The table shows
the source file; unsaved FP/FN annotations are applied only when saving an edited
Parquet copy.

The integrated ParquetViewer includes its file/folder explorer and multiple
closable file tabs. Open additional `.parquet` or `.parq` files from the table
toolbar or explorer. **Plot this file** opens the active table file in the plot
column picker. You can use the table browser before loading a plot, including
for files that contain only text or other nonnumeric columns.

Use the page arrows and **Rows** control to navigate; enter `-1` to show every
matching row. **Find** searches across all columns with **contains** or **exact**
matching, or accepts quick filters such as `target = 1`, `score > 10`, and
`active = true`. Press Enter to apply the search. **Add Filter** provides column,
operator, and value controls for numeric comparisons, text matching, booleans,
and empty values. Multiple filters are combined with AND; use **Apply** to run
them or **Clear** to remove them.

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

To run the plot and table interaction tests:

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
