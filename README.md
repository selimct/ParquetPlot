# ParquetAnnotations

Interactive desktop application for visualizing and annotating ECG data from parquet files.

## Features

- **Single unified plot with:**
  - Any number of user-selected signal columns
  - Distinct colors and legend labels for each signal
  - Semi-transparent red dashed lines: Target annotations (1 only)
- **Interactive controls:**
  - Zoom: scroll wheel
  - Pan: drag mouse
  - Hover: see exact coordinates
- **Efficient**: handles up to 350k entries smoothly
- **One file at a time**: load and visualize parquet files sequentially

## Installation

### Dependencies

- Qt 6
- Apache Arrow C++
- Parquet C++
- QCustomPlot

### Ubuntu/Debian

```bash
sudo apt-get install qt6-base-dev libapache-arrow-dev libparquet-dev
```

### Fedora/RHEL

```bash
sudo dnf install qt6-qtbase-devel libarrow-devel parquet-libs-devel
```

### QCustomPlot Setup (if needed)

If QCustomPlot is not in your system packages:

1. Download from <https://www.qcustomplot.com/>
2. Copy `qcustomplot.h` and `qcustomplot.cpp` to `src/`
3. Uncomment lines in CMakeLists.txt to include source files

### Build

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./ParquetAnnotations
```

1. Click **"Open Parquet File"** button
2. Select a `.parquet` file
3. Choose the X-axis and trigger columns. Add or remove any number of plotted signal columns; ECG and pulse are preselected when available.
4. View the selected signals and target annotations (red dashed lines at flagged positions)
5. Use mouse wheel to zoom, drag to pan
6. Hover over any point to read coordinates

## Data Format

Choose any numeric columns when opening a file. These conventional names are preselected when present:

- `raw_idx` (numeric): sample index
- `ecg1_rotated` (numeric): ECG signal
- `puls_raw` (numeric): pulse/heart rate signal
- `target` (numeric or boolean): annotation flags (0 or 1)

Only target=1 markers are displayed as vertical lines, positioned slightly above the ECG value at each annotation point.

## License

ParquetAnnotations is licensed under the GNU General Public License v3.0 only.
See [LICENSE](LICENSE) for the full license text.
