// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#ifndef PARQUET_READER_H
#define PARQUET_READER_H

#include <string>
#include <vector>
#include <memory>

struct ParquetData
{
    std::vector<double> raw_idx;
    std::vector<std::vector<double>> signal_values;
    std::vector<int> target;
};

class ParquetReader
{
public:
    static std::vector<std::string> columnNames(const std::string &filename);
    static std::unique_ptr<ParquetData> read(const std::string &filename,
                                             const std::string &x_column,
                                             const std::vector<std::string> &signal_columns,
                                             const std::string &trigger_column);
};

#endif // PARQUET_READER_H
