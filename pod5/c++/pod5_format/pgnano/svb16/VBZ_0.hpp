#ifndef VBZ1_HPP
#define VBZ1_HPP


#include <arrow/buffer.h>

#include "compressor.h"
#include "decompressor.h"
#include "macros.h"
#include "header.h"
#include "BAM_handler.h"

#include "pod5_format/types.h"
#include "pod5_format/read_table_utils.h"
#include "pod5_format/pgnano/pgnano_writer_state.h"

#include "pod5_format/signal_compression.h"

#include <arrow/buffer.h>
#include <zstd.h>

#include "common.hpp"
#include "svb16.h"  // svb16_key_length

#include "pod5_format/pgnano/pgnano_utils.hpp"
#include <chrono>

#endif // VBZ1_HPP

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

extern long number_small;
extern long number_medium;
extern long number_large;

extern double compression_time;
extern double decompression_time;

namespace svb16 {

struct buf_sizes_VBZ1 {
    size_t keys_sz, data_sz;
};


template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_VBZ1 encode_scalar_VBZ1(
    Int16T const * in,
    uint8_t * SVB_RESTRICT keys,
    uint8_t * data,
    uint32_t count,
    Int16T prev = 0)
{
    
    if (count == 0) {
        return {0,0};
    }
    uint8_t * keys_begin = keys;
    uint8_t * data_begin = data;

    uint8_t data_shift = 0; // cycles 0,4,8 then resets
    uint8_t shift = 0;  // cycles 0,2,4,6,8 then resets
    uint8_t key_byte = 0;
   
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keys++ = key_byte;
            key_byte = 0;
        }

        uint16_t value;
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            // need to do the arithmetic in unsigned space so it wraps
            value = static_cast<uint16_t>(in[c]) - static_cast<uint16_t>(prev);
            SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_encode(value); }
            prev = in[c];
        }
        else SVB16_IF_CONSTEXPR(UseZigzag) {
            value = detail::zigzag_encode(static_cast<uint16_t>(in[c]));
        }
        else {
            value = static_cast<uint16_t>(in[c]);
        }

        uint8_t code = 0;
        //0 byte
        if (value == 0){
            code =0;
        } else if (--value < 16){
        // 1/2 byte
            code = 1;
        } else if ((value -= 16) < 256) {
        // 1 byte
            code = 2;
        } else { 
        // 2 bytes
            value -= 256;
            code = 3;
        }

        size_t bits;
        // printf("code: %d\n",code);
        // printf("value: %d\n",value);
        switch (code){
            case 0:
                bits = 0;
                break;
            case 1:
                number_small ++;
                bits = 4;
                break;
            case 2:
                number_medium ++;
                bits = 8;
                break;
            case 3:
                number_large ++;
                bits = 16;
                break;
            default:
                assert(0 && "Unknown code");
                bits = 0;
                break;
        }

        for (std::size_t i = 0; i < bits/4; ++i)
        {
            auto val_masked = value & 0xf;
            value >>= 4;

            if (data_shift == 0)
            {
                *data = 0;
            }

            *data |= val_masked << data_shift;
            data_shift += 4;

            if (data_shift == 8)
            {
                data_shift = 0;
                data++;
            }
        }
        
        key_byte |= code << shift;
        shift += 2;
    }

    if (data_shift != 0){
        data += 1;
    }
    *keys++ = key_byte;  // write last key (no increment needed)

    return {(size_t) (keys - keys_begin),(size_t) (data - data_begin)};
}



template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_VBZ1 encode_VBZ1(Int16T const * in, uint8_t * SVB_RESTRICT out, uint32_t count, Int16T prev = 0)
    {
        auto const keys = out;
        auto const data = keys + ::svb16_key_length_2bit(count);
        return encode_scalar_VBZ1<Int16T, UseDelta, UseZigzag>(in, keys, data, count, prev);
    }


template <typename Int16T, bool UseDelta, bool UseZigzag>
uint8_t const * decode_scalar_VBZ1(
    gsl::span<Int16T> out_span,
    gsl::span<uint8_t const> keys_span,
    gsl::span<uint8_t const> data_span,
    Int16T prev = 0)
{
    
    auto const count = out_span.size();
    if (count == 0) {
        return data_span.begin();
    }

    auto out = out_span.begin();
    auto keys = keys_span.begin();
    auto data = data_span.begin();

    uint8_t data_shift = 0;
    uint8_t shift = 0;  // cycles 0,2,4,6,8 then resets
    uint8_t key_byte = *keys++; // read first key

    // need to do the arithmetic in unsigned space so it wraps
    auto u_prev = static_cast<uint16_t>(prev);
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        
        uint16_t value = 0;
        uint8_t code = (key_byte >> shift) & 0x3;

        size_t bits;
        switch (code){
            case 0:
                bits = 0;
                break;
            case 1:
                bits = 4;
                break;
            case 2:
                bits = 8;
                break;
            case 3:
                bits = 16;
                break;
            default:
                assert(0 && "Unknown code");
                bits = 0;
                break;
        }

        for (std::size_t i = 0; i < bits/4; ++i)
            {
                if (data_shift == 8)
                {
                    data++;
                    data_shift = 0;
                }
                
                auto val_bits = 0xf & (*data >> data_shift);
                value |= val_bits << (i*4);
                data_shift += 4;
            }
        switch (code)
        {
        case 0:
            value =0;
            break;
        case 1:
            value += 1;
            break;
        case 2:
            value += 17;
            break;
        case 3:
            value += 273;
            break;
        default:
            assert(0 && "Unknown code");
            value = 0;
            break;
        }
        

        SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_decode(value);}
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            value += u_prev;
            u_prev = value;
        }
        *out++ = static_cast<Int16T>(value);
        shift += 2;
    }

    if (data_shift != 0)
    {
        data += 1;
    }

    assert(out == out_span.end());
    assert(keys == keys_span.end());
    // printf("data: %p\tdata_span.end(): %p\n",data,data_span.end());
    assert(data <= data_span.end());    
    return data;
}



template <typename Int16T, bool UseDelta, bool UseZigzag>
size_t decode_VBZ1(gsl::span<Int16T> out, gsl::span<uint8_t const> in, uint32_t d, Int16T prev = 0)
{
    auto keys_length = ::svb16_key_length_2bit(out.size());
    auto const keys = in.subspan(0, keys_length);
    auto const data = in.subspan(keys_length);

#ifdef SVB16_X64
    //if (has_sse4_1()) {
    //    return decode_sse<Int16T, UseDelta, UseZigzag>(out, keys, data, prev) - in.begin();
    //}
#endif
    return decode_scalar_VBZ1<Int16T, UseDelta, UseZigzag>(out, keys,data, prev) - in.begin();
}



} // namespace svb16


namespace pgnano{

arrow::Result<std::size_t> compress_signal_VBZ1(  // COMPRESOR VBZ1 //
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{
    auto t1 = std::chrono::steady_clock::now();

    // --------------- First compress the samples using svb --------------- //
    auto const max_size = svb16_max_encoded_length_2bit(samples.size());
    ARROW_ASSIGN_OR_RAISE(auto intermediate, arrow::AllocateResizableBuffer(max_size, pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto const encoded_sz = svb16::encode_VBZ1<int16_t, UseDelta, UseZigzag>(
        samples.data(), intermediate->mutable_data(), samples.size());

    size_t encoded_count = encoded_sz.keys_sz + encoded_sz.data_sz; 
    
    ARROW_RETURN_NOT_OK(intermediate->Resize(encoded_count));

    // --------------- Now compress the svb data using zstd --------------- //
    size_t const zstd_compressed_max_size = ZSTD_compressBound(intermediate->size());
    if (ZSTD_isError(zstd_compressed_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for svb data");
    }

    size_t const compressed_size = ZSTD_compress(
        destination.data(), destination.size(),
        intermediate->data(), intermediate->size(), 1);
    if (ZSTD_isError(compressed_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    auto t2 = std::chrono::steady_clock::now();
    compression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

    full_size_keys += encoded_sz.keys_sz;
    full_size_M += encoded_sz.data_sz;
    // full_size_data += encoded_count;
    comp_size_data += compressed_size;
    total_samples += samples.size();

    return compressed_size;
}

    pod5::Status decompress_signal_VBZ1(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state)
{

    auto t1d = std::chrono::steady_clock::now();

    // First decompress data using zstd
    // Calculate decompressed size
    unsigned long long const decompressed_zstd_size =
        ZSTD_getFrameContentSize(compressed_bytes.data(), compressed_bytes.size());
    if (ZSTD_isError(decompressed_zstd_size)) {
        return pod5::Status::Invalid(
            "vbz1 Input data not compressed by zstd: (",
            decompressed_zstd_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_size),
            ")");
    }


    // Calculo el allocation_padding y creo el buffer intermediate
    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer(decompressed_zstd_size + 
                                    allocation_padding, pool));
    
    // Decompress
    size_t const decompress_res = ZSTD_decompress(
        intermediate->mutable_data(),
        decompressed_zstd_size,
        compressed_bytes.data(),
        compressed_bytes.size());
    if (ZSTD_isError(decompress_res)) {
        return pod5::Status::Invalid(
            "Input data failed to decompress using zstd: (",
            decompress_res,
            " ",
            ZSTD_getErrorName(decompress_res),
            ")");
    }    
    
    // Now decompress the data using svb:
    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto consumed_count = svb16::decode_VBZ1<int16_t, UseDelta, UseZigzag>(destination, gsl::make_span(intermediate->data(), 
        intermediate->size()), 0);
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }
    
    auto t2d = std::chrono::steady_clock::now();
    decompression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2d - t1d).count();

    return pod5::Status::OK();
}


} // namespace pgnano