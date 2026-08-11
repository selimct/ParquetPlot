// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include "parquet_reader.h"
#include <parquet/arrow/reader.h>
#include <arrow/io/file.h>
#include <arrow/array.h>
#include <arrow/table.h>
#include <arrow/type.h>
#include <stdexcept>

static std::unique_ptr<parquet::arrow::FileReader> open_reader(const std::string &filename)
{
    auto infile_result = arrow::io::ReadableFile::Open(filename);
    if (!infile_result.ok())
    {
        throw std::runtime_error("Failed to open file: " + infile_result.status().ToString());
    }

    auto reader_result = parquet::arrow::OpenFile(infile_result.ValueOrDie(), arrow::default_memory_pool());
    if (!reader_result.ok())
    {
        throw std::runtime_error("Failed to open parquet file: " + reader_result.status().ToString());
    }

    return std::move(reader_result).ValueOrDie();
}

static std::vector<double> get_double_column(const std::shared_ptr<arrow::Table> &table, const std::string &name)
{
    auto column = table->GetColumnByName(name);
    if (!column)
        throw std::runtime_error("Column " + name + " not found");

    std::vector<double> result;
    for (int i = 0; i < column->num_chunks(); ++i)
    {
        auto chunk = column->chunk(i);

        // Try different numeric types
        if (auto arr = std::dynamic_pointer_cast<arrow::DoubleArray>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(arr->Value(j));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::FloatArray>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int64Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int32Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int16Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int8Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt64Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt32Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt16Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt8Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<double>(arr->Value(j)));
            }
        }
        else
        {
            throw std::runtime_error("Column " + name + " has unsupported numeric type");
        }
    }
    return result;
}

static std::vector<int> get_int_column(const std::shared_ptr<arrow::Table> &table, const std::string &name)
{
    auto column = table->GetColumnByName(name);
    if (!column)
        throw std::runtime_error("Column " + name + " not found");

    std::vector<int> result;
    for (int i = 0; i < column->num_chunks(); ++i)
    {
        auto chunk = column->chunk(i);

        // Try different integer types
        if (auto arr = std::dynamic_pointer_cast<arrow::Int64Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int32Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int16Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt32Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt64Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt16Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::UInt8Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::Int8Array>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::BooleanArray>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(arr->Value(j) ? 1 : 0);
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::DoubleArray>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else if (auto arr = std::dynamic_pointer_cast<arrow::FloatArray>(chunk))
        {
            for (int64_t j = 0; j < arr->length(); ++j)
            {
                result.push_back(static_cast<int>(arr->Value(j)));
            }
        }
        else
        {
            throw std::runtime_error("Trigger column " + name + " must be numeric or boolean");
        }
    }
    return result;
}

std::vector<std::string> ParquetReader::columnNames(const std::string &filename)
{
    auto reader = open_reader(filename);
    std::shared_ptr<arrow::Schema> schema;
    auto status = reader->GetSchema(&schema);
    if (!status.ok())
    {
        throw std::runtime_error("Failed to read schema: " + status.ToString());
    }

    std::vector<std::string> names;
    names.reserve(schema->num_fields());
    for (const auto &field : schema->fields())
    {
        names.push_back(field->name());
    }
    return names;
}

std::unique_ptr<ParquetData> ParquetReader::read(const std::string &filename,
                                                 const std::string &x_column,
                                                 const std::vector<std::string> &signal_columns,
                                                 const std::string &trigger_column)
{
    auto data = std::make_unique<ParquetData>();
    auto reader = open_reader(filename);

    std::shared_ptr<arrow::Table> table;
    auto status = reader->ReadTable(&table);
    if (!status.ok())
    {
        throw std::runtime_error("Failed to read table: " + status.ToString());
    }

    data->raw_idx = get_double_column(table, x_column);
    data->signal_values.reserve(signal_columns.size());
    for (const std::string &signal_column : signal_columns)
    {
        data->signal_values.push_back(get_double_column(table, signal_column));
    }
    data->target = get_int_column(table, trigger_column);

    if (data->raw_idx.empty())
    {
        throw std::runtime_error("The selected columns contain no rows");
    }

    return data;
}
