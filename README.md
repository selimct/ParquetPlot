# ParquetPlot

Linux-native and Windows-available desktop application for visualizing and annotating (ECG) data from parquet files.

## Features

- **Single unified plot with:**
  - Any number of user-selected signal columns
  - A high-contrast, colorblind-conscious automatic color cycle with legend labels
  - Custom colors for every signal and target plot
  - Distinct dashed impulse lines for any number of target annotation columns
- **Interactive controls:**
  - Zoom: scroll wheel
  - Pan: drag mouse
  - Hover: see exact coordinates
- **Efficient**: handles up to 350k entries smoothly
- **One file at a time**: load and visualize parquet files sequentially

## Installation

### Download a finished application

Each published GitHub Release contains ready-to-run x86-64 packages:

- **Linux (Fedora and Ubuntu):** download `ParquetPlot-Linux-x86_64.AppImage`,
  make it executable, and run it:

  ```bash
  chmod +x ParquetPlot-Linux-x86_64.AppImage
  ./ParquetPlot-Linux-x86_64.AppImage
  ```

- **Windows:** download `ParquetPlot-Windows-x64.zip`, extract the whole
  `ParquetPlot` folder, and run `ParquetPlot.exe` from that folder.

The packages include Qt, Apache Arrow/Parquet, QCustomPlot, and their required
runtime libraries. Development packages are only needed when building from
source.

### Build from source

#### Dependencies

- Qt 6
- Apache Arrow C++
- Parquet C++

QCustomPlot 2.1.1 is downloaded from its official site at configure time,
verified with SHA-512, and compiled into the application. To build offline
against an installed Qt 6 QCustomPlot library, configure CMake with
`-DPARQUETPLOT_USE_SYSTEM_QCUSTOMPLOT=ON`.

#### Ubuntu/Debian

Enable the official [Apache Arrow APT repository](https://arrow.apache.org/install/),
then install:

```bash
sudo apt-get install qt6-base-dev libarrow-dev libparquet-dev
```

#### Fedora/RHEL

```bash
sudo dnf install qt6-qtbase-devel libarrow-devel parquet-libs-devel
```

#### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Usage

```bash
./build/ParquetPlot
```

1. Click **"Open Parquet File"** button
2. Select a `.parquet` file
3. Choose the X-axis column. Add or remove any number of plotted signal and target columns; ECG, pulse, and target are preselected when available.
4. View the selected signals and target annotations (dashed lines at flagged positions). Target amplitude defaults to 1500 and can be changed in the toolbar.
5. In the column chooser, click the color beside any signal or target to customize it. Select **Auto** to restore its palette color.
6. Use **Select Columns** to reopen the column chooser for the current file without changing the current zoom or pan ranges.
7. Use mouse wheel to zoom, drag to pan
8. Hover over any point to read coordinates

## Data Format

Choose any numeric columns when opening a file. These conventional names are preselected when present:

- `raw_idx` (numeric): sample index
- `ecg1_rotated` (numeric): ECG signal
- `puls_raw` (numeric): pulse/heart rate signal
- `target` (numeric or boolean): annotation flags (0 or 1)

Only target=1 markers are displayed as vertical lines. All selected target columns use the threshold amplitude shown in the toolbar.

## License

ParquetPlot is licensed under the GNU General Public License v3.0 only.
See [LICENSE](LICENSE) for the full license text.

Third-party notices for bundled release dependencies are listed in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Release automation

Publishing a GitHub Release triggers the release workflow. It builds and
smoke-tests both packages, then attaches them and a `SHA256SUMS` file to that
release. Pushes and pull requests do not run the packaging workflow. Create
releases from tags that contain the desired application source.
