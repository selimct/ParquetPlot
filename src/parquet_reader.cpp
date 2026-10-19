// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include "parquet_reader.h"
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/properties.h>
#include <arrow/io/file.h>
#include <arrow/array.h>
#include <arrow/array/util.h>
#include <arrow/buffer.h>
#include <arrow/table.h>
#include <arrow/type.h>
#include <arrow/util/bit_util.h>
#include <arrow/util/config.h>
#include <algorithm>
#include <cstring>
#include <map>
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

static std::shared_ptr<arrow::Table> read_table(parquet::arrow::FileReader &reader,
                                               const char *error_message)
{
#if ARROW_VERSION_MAJOR >= 24
    auto result = reader.ReadTable();
    if (!result.ok())
        throw std::runtime_error(std::string(error_message) + result.status().ToString());
    return std::move(result).ValueOrDie();
#else
    // The Result-returning overload was introduced in Arrow 24.
    std::shared_ptr<arrow::Table> table;
    auto status = reader.ReadTable(&table);
    if (!status.ok())
        throw std::runtime_error(std::string(error_message) + status.ToString());
    return table;
#endif
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
                                                 const std::vector<std::string> &trigger_columns)
{
    auto data = std::make_unique<ParquetData>();
    auto reader = open_reader(filename);

    auto table = read_table(*reader, "Failed to read table: ");

    data->raw_idx = get_double_column(table, x_column);
    data->signal_values.reserve(signal_columns.size());
    for (const std::string &signal_column : signal_columns)
    {
        data->signal_values.push_back(get_double_column(table, signal_column));
    }
    data->target_values.reserve(trigger_columns.size());
    for (const std::string &trigger_column : trigger_columns)
    {
        data->target_values.push_back(get_int_column(table, trigger_column));
    }

    if (data->raw_idx.empty())
    {
        throw std::runtime_error("The selected columns contain no rows");
    }

    return data;
}

static void set_binary_target_value(arrow::Type::type type, uint8_t *values,
                                    int64_t index, bool value)
{
    switch (type)
    {
    case arrow::Type::BOOL:
        arrow::bit_util::SetBitTo(values, index, value);
        break;
    case arrow::Type::INT8:
        reinterpret_cast<int8_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::INT16:
        reinterpret_cast<int16_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::INT32:
        reinterpret_cast<int32_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::INT64:
        reinterpret_cast<int64_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::UINT8:
        reinterpret_cast<uint8_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::UINT16:
        reinterpret_cast<uint16_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::UINT32:
        reinterpret_cast<uint32_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::UINT64:
        reinterpret_cast<uint64_t *>(values)[index] = value ? 1 : 0;
        break;
    case arrow::Type::FLOAT:
        reinterpret_cast<float *>(values)[index] = value ? 1.0F : 0.0F;
        break;
    case arrow::Type::DOUBLE:
        reinterpret_cast<double *>(values)[index] = value ? 1.0 : 0.0;
        break;
    default:
        throw std::runtime_error("Edited target columns must be numeric or boolean");
    }
}

static std::shared_ptr<arrow::ChunkedArray> edit_target_column(
    const std::shared_ptr<arrow::ChunkedArray> &source,
    const std::map<int64_t, bool> &row_edits,
    const std::string &column_name)
{
    std::vector<std::shared_ptr<arrow::Array>> chunks = source->chunks();
    auto edit = row_edits.cbegin();
    int64_t chunk_start = 0;

    for (size_t chunk_index = 0; chunk_index < chunks.size() && edit != row_edits.cend();
         ++chunk_index)
    {
        const std::shared_ptr<arrow::Array> &source_chunk = chunks[chunk_index];
        const int64_t chunk_end = chunk_start + source_chunk->length();
        if (edit->first >= chunk_end)
        {
            chunk_start = chunk_end;
            continue;
        }

        const auto first_edit = edit;
        while (edit != row_edits.cend() && edit->first < chunk_end)
            ++edit;

        const std::shared_ptr<arrow::ArrayData> source_data = source_chunk->data();
        if (source_data->buffers.size() < 2 || !source_data->buffers[1])
            throw std::runtime_error("Target column " + column_name + " has no value buffer");

        auto buffer_result = arrow::AllocateBuffer(source_data->buffers[1]->size());
        if (!buffer_result.ok())
            throw std::runtime_error("Failed to allocate an edited target buffer: " +
                                     buffer_result.status().ToString());
        std::shared_ptr<arrow::Buffer> edited_values(std::move(buffer_result).ValueOrDie());
        std::memcpy(edited_values->mutable_data(), source_data->buffers[1]->data(),
                    static_cast<size_t>(source_data->buffers[1]->size()));

        for (auto current = first_edit; current != edit; ++current)
        {
            const int64_t local_row = current->first - chunk_start;
            if (source_chunk->IsNull(local_row))
            {
                throw std::runtime_error("Cannot apply an FP/FN marker to null cell in column " +
                                         column_name + " at row " +
                                         std::to_string(current->first));
            }
            set_binary_target_value(source->type()->id(), edited_values->mutable_data(),
                                    source_data->offset + local_row, current->second);
        }

        std::shared_ptr<arrow::ArrayData> edited_data = source_data->Copy();
        edited_data->buffers[1] = std::move(edited_values);
        chunks[chunk_index] = arrow::MakeArray(edited_data);
        chunk_start = chunk_end;
    }

    return std::make_shared<arrow::ChunkedArray>(std::move(chunks), source->type());
}

void ParquetReader::writeEditedCopy(const std::string &input_filename,
                                    const std::string &output_filename,
                                    const std::vector<ParquetCellEdit> &edits)
{
    if (edits.empty())
        throw std::runtime_error("No FP/FN changes were supplied");
    if (input_filename == output_filename)
        throw std::runtime_error("The source Parquet file cannot be overwritten");

    auto reader = open_reader(input_filename);
    const std::shared_ptr<parquet::FileMetaData> input_metadata =
        reader->parquet_reader()->metadata();

    auto original_table = read_table(*reader, "Failed to read the complete source table: ");

    std::map<std::string, std::map<int64_t, bool>> normalized_edits;
    for (const ParquetCellEdit &edit : edits)
    {
        if (edit.row_index < 0 || edit.row_index >= original_table->num_rows())
            throw std::runtime_error("FP/FN marker row is outside the source table");

        auto &column_edits = normalized_edits[edit.column_name];
        auto existing = column_edits.find(edit.row_index);
        if (existing != column_edits.end() && existing->second != edit.value)
        {
            throw std::runtime_error("Conflicting FP and FN markers target column " +
                                     edit.column_name + " at row " +
                                     std::to_string(edit.row_index));
        }
        column_edits[edit.row_index] = edit.value;
    }

    std::shared_ptr<arrow::Table> edited_table = original_table;
    for (const auto &column_entry : normalized_edits)
    {
        const std::vector<int> indices = original_table->schema()->GetAllFieldIndices(column_entry.first);
        if (indices.empty())
            throw std::runtime_error("Target column " + column_entry.first + " was not found");
        if (indices.size() != 1)
            throw std::runtime_error("Target column name " + column_entry.first +
                                     " is duplicated in the source schema");

        const int column_index = indices.front();
        std::shared_ptr<arrow::ChunkedArray> edited_column = edit_target_column(
            original_table->column(column_index), column_entry.second, column_entry.first);
        auto table_result = edited_table->SetColumn(
            column_index, original_table->schema()->field(column_index), edited_column);
        if (!table_result.ok())
            throw std::runtime_error("Failed to replace target column " + column_entry.first +
                                     ": " + table_result.status().ToString());
        edited_table = std::move(table_result).ValueOrDie();
    }

    auto output_result = arrow::io::FileOutputStream::Open(output_filename);
    if (!output_result.ok())
        throw std::runtime_error("Failed to open output file: " + output_result.status().ToString());
    std::shared_ptr<arrow::io::FileOutputStream> output =
        std::move(output_result).ValueOrDie();

    parquet::WriterProperties::Builder parquet_properties_builder;
    std::shared_ptr<parquet::WriterProperties> parquet_properties =
        parquet_properties_builder.compression(parquet::Compression::SNAPPY)->build();
    parquet::ArrowWriterProperties::Builder arrow_properties_builder;
    std::shared_ptr<parquet::ArrowWriterProperties> arrow_properties =
        arrow_properties_builder.store_schema()->build();

    auto writer_result = parquet::arrow::FileWriter::Open(
        *edited_table->schema(), arrow::default_memory_pool(), output,
        parquet_properties, arrow_properties);
    if (!writer_result.ok())
        throw std::runtime_error("Failed to create Parquet writer: " +
                                 writer_result.status().ToString());
    std::unique_ptr<parquet::arrow::FileWriter> writer =
        std::move(writer_result).ValueOrDie();

    int64_t row_offset = 0;
    for (int row_group = 0; row_group < input_metadata->num_row_groups(); ++row_group)
    {
        const int64_t row_count = input_metadata->RowGroup(row_group)->num_rows();
        auto status = writer->WriteTable(*edited_table->Slice(row_offset, row_count), row_count);
        if (!status.ok())
            throw std::runtime_error("Failed to write Parquet row group: " + status.ToString());
        row_offset += row_count;
    }
    if (row_offset != edited_table->num_rows())
        throw std::runtime_error("Source row-group metadata does not cover every table row");

    auto status = writer->Close();
    if (!status.ok())
        throw std::runtime_error("Failed to finalize Parquet data: " + status.ToString());
    status = output->Close();
    if (!status.ok())
        throw std::runtime_error("Failed to close output file: " + status.ToString());

    auto verification_reader = open_reader(output_filename);
    const std::shared_ptr<parquet::FileMetaData> output_metadata =
        verification_reader->parquet_reader()->metadata();
    auto verified_table = read_table(*verification_reader, "Failed to verify saved Parquet table: ");

    if (!verified_table->schema()->Equals(*edited_table->schema(), true))
        throw std::runtime_error("Saved Parquet schema or schema metadata differs from the source");

    const arrow::EqualOptions equal_options = arrow::EqualOptions::Defaults().nans_equal(true);
    if (!verified_table->Equals(*edited_table, equal_options))
        throw std::runtime_error("Saved Parquet values differ from the requested FP/FN edits");

    if (output_metadata->num_row_groups() != input_metadata->num_row_groups())
        throw std::runtime_error("Saved Parquet row-group count differs from the source");
    for (int row_group = 0; row_group < output_metadata->num_row_groups(); ++row_group)
    {
        const std::unique_ptr<parquet::RowGroupMetaData> output_row_group =
            output_metadata->RowGroup(row_group);
        if (output_row_group->num_rows() != input_metadata->RowGroup(row_group)->num_rows())
            throw std::runtime_error("Saved Parquet row-group boundaries differ from the source");
        for (int column = 0; column < output_row_group->num_columns(); ++column)
        {
            if (output_row_group->ColumnChunk(column)->compression() != parquet::Compression::SNAPPY)
                throw std::runtime_error("Saved Parquet file contains a column that is not Snappy-compressed");
        }
    }
}
