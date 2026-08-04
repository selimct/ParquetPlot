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
    std::vector<double> ecg1_rotated;
    std::vector<double> puls_raw;
    std::vector<int> target;
};

class ParquetReader
{
public:
    static std::unique_ptr<ParquetData> read(const std::string &filename);
};

#endif // PARQUET_READER_H
