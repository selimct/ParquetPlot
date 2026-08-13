// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#ifndef PARQUET_READER_H
#define PARQUET_READER_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

struct ParquetData
{
    std::vector<double> raw_idx;
    std::vector<std::vector<double>> signal_values;
    std::vector<std::vector<int>> target_values;
};

struct ParquetCellEdit
{
    std::string column_name;
    int64_t row_index;
    bool value;
};

class ParquetReader
{
public:
    static std::vector<std::string> columnNames(const std::string &filename);
    static std::unique_ptr<ParquetData> read(const std::string &filename,
                                             const std::string &x_column,
                                             const std::vector<std::string> &signal_columns,
                                             const std::vector<std::string> &trigger_columns);
    static void writeEditedCopy(const std::string &input_filename,
                                const std::string &output_filename,
                                const std::vector<ParquetCellEdit> &edits);
};

#endif // PARQUET_READER_H
