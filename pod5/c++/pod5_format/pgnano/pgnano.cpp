#define COMPRESSOR_C5

#include <arrow/buffer.h>

#include "compressor.h"
#include "decompressor.h"
#include "macros.h"
#include "header.h"
#include "BAM_handler.h"

#include "pod5_format/types.h"
#include "pod5_format/read_table_utils.h"
#include "pgnano_writer_state.h"

#include "pod5_format/signal_compression.h"

#include "pod5_format/pgnano/svb16/decode.hpp"
#include "pod5_format/pgnano/svb16/encode.hpp"

#include <arrow/buffer.h>
#include <zstd.h>

#include "pod5_format/pgnano/svb16/C1.hpp"
#include "pod5_format/pgnano/svb16/VBZ_0.hpp"
#include "pod5_format/pgnano/svb16/C3.hpp"
#include "pod5_format/pgnano/svb16/C2.hpp"
#include "pod5_format/pgnano/svb16/C5.hpp"
#include "pod5_format/pgnano/svb16/C4.hpp"

extern long full_size_keys;
extern long full_size_S;
extern long full_size_M;
extern long full_size_Llow;
extern long full_size_Lhigh;
extern long full_size_data;

extern long comp_size_keys;
extern long comp_size_S;
extern long comp_size_M;
extern long comp_size_Llow;
extern long comp_size_Lhigh;
extern long comp_size_data;

extern long total_samples;

extern double compression_time;
extern double decompression_time;

namespace pgnano {

std::size_t compressed_signal_max_size(std::size_t sample_count)
{
    auto const max_svb_size = svb16_max_encoded_length(sample_count);
    auto const zstd_compressed_max_size = ZSTD_compressBound(max_svb_size);
    return zstd_compressed_max_size;
}


arrow::Result<std::shared_ptr<arrow::Buffer>> compress_signal(  
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{

    ARROW_ASSIGN_OR_RAISE(
        std::shared_ptr<arrow::ResizableBuffer> out,
        arrow::AllocateResizableBuffer(pgnano::Compressor::compressed_signal_max_size(samples.size(), read_data), pool));

    ARROW_ASSIGN_OR_RAISE( 
        auto final_size,
        #ifdef COMPRESSOR_VBZ1 
            pod5::compress_signal(samples, pool, gsl::make_span(out->mutable_data(), out->size())));
        #endif
        #ifdef COMPRESSOR_VBZ0
            pgnano::compress_signal_VBZ1(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        #ifdef COMPRESSOR_C1
            pgnano::compress_signal_KD(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        #ifdef COMPRESSOR_C2
            pgnano::compress_signal_lh(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        #ifdef COMPRESSOR_C3
            pgnano::compress_signal_ll_lh(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        #ifdef COMPRESSOR_C5
            pgnano::compress_signal_N01(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        #ifdef COMPRESSOR_C4
            pgnano::compress_signal_N02(samples, pool, gsl::make_span(out->mutable_data(), out->size()), read_data, is_last_batch));
        #endif
        
    ARROW_RETURN_NOT_OK(out->Resize(final_size));
    return out;
}

pod5::Status decompress_signal(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state)
{

    #ifdef COMPRESSOR_VBZ1
        return pod5::decompress_signal(compressed_bytes, pool, destination);
    #endif
    #ifdef COMPRESSOR_VBZ0
        return pgnano::decompress_signal_VBZ1(compressed_bytes, pool, destination, state);
    #endif
    #ifdef COMPRESSOR_C1
        return pgnano::decompress_signal_KD(compressed_bytes, pool, destination, state);
    #endif
    #ifdef COMPRESSOR_C2
        return pgnano::decompress_signal_lh(compressed_bytes, pool, destination, state);
    #endif
    #ifdef COMPRESSOR_C3
        return pgnano::decompress_signal_ll_lh(compressed_bytes, pool, destination, state);
    #endif
    #ifdef COMPRESSOR_C5
        return pgnano::decompress_signal_N01(compressed_bytes, pool, destination, state);
    #endif
    #ifdef COMPRESSOR_C4
        return pgnano::decompress_signal_N02(compressed_bytes, pool, destination, state);
    #endif
}

};

