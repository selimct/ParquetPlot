# ParquetPlot

Desktop application for visualizing and annotating Parquet signal data on Linux and Windows.

## Features

- Plot any number of numeric signal and target columns
- Automatic high-contrast colors with per-series customization
- Zoom, pan, hover coordinates, and marker annotations
- Smooth handling of files with up to 350k rows

## Installation

Each published GitHub Release contains ready-to-run x86-64 packages:

- **Linux:** download the AppImage, then run:

  ```bash
  chmod +x ParquetPlot-Linux-x86_64.AppImage
  ./ParquetPlot-Linux-x86_64.AppImage
  ```

- **Windows:** extract `ParquetPlot-Windows-x64.zip` and run `ParquetPlot.exe`.

Runtime dependencies are included in both packages.

### Open Parquet files by double-clicking

- **Linux:** integrate the AppImage, then select ParquetPlot from the file's
  **Open With** dialog. Source installs include the desktop and MIME metadata.
- **Windows:** use **Open with > Choose another app**, select `ParquetPlot.exe`,
  and enable **Always use this app**.

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

## Usage

1. Open or double-click a `.parquet` file.
2. Choose the X-axis, signals, targets, and optional custom colors.
3. Scroll to zoom, drag to pan, or hover to inspect coordinates.
4. Use **Select Columns** to change the plot without resetting its view.

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
