# ParquetPlot

Interactive desktop application for visualizing and annotating ECG data from parquet files.

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
./ParquetPlot
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
