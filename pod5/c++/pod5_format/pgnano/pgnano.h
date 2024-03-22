#pragma once

#include "pod5_format/types.h"
#include "pod5_format/read_table_utils.h"
#include "compressor.h"
#include "pgnano_reader_state.h"
#include "pgnano_writer_state.h"
#include "BAM_handler.h"

// TODO: Use SampleType instead of int16_t
namespace pgnano {

pod5::Status decompress_signal(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state);

arrow::Result<std::shared_ptr<arrow::Buffer>> compress_signal(
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    pod5::ReadData const & read_data,
    bool is_last_batch);

static inline CompressionStats get_compression_stats() { return Compressor::get_compression_stats(); }

void pgnano_init();
void pgnano_terminate();
};