#ifndef L_H_HPP
#define L_H_HPP

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

#include <chrono>

#include "pod5_format/pgnano/pgnano_utils.hpp"

#include <arrow/buffer.h>
#include <zstd.h>

#endif  // L_H_HPP

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
//extern double decompression_time;

namespace svb16 {

struct buf_sizes_lh {
    size_t keys_sz, data_l_sz, data_h_sz;
};

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_lh encode_scalar_lh(
    Int16T const * in,
    uint8_t * SVB_RESTRICT keys,
    uint8_t * SVB_RESTRICT data_low,
    uint8_t * SVB_RESTRICT data_high,
    uint32_t count,
    Int16T prev = 0)
{
    
    if (count == 0) {
        return {0,0,0};
    }

    uint8_t * keys_begin = keys;
    uint8_t * data_low_begin = data_low;
    uint8_t * data_high_begin = data_high;


    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = 0;
   
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keys++ = key_byte;
            key_byte = 0;
        }

        // Calcula VALUE:

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

        *data_low++ = static_cast<uint8_t>(value);

        // Almacena BYTES en BUFFER
        if (value >= (1 << 8)) {    // 2 bytes
            *data_high++ = static_cast<uint8_t>(value >> 8);  // assumes little endian
            key_byte |= 1 << shift;
        }
        shift += 1;
    }

    *keys++ = key_byte;  // write last key (no increment needed)
    return {(size_t) (keys - keys_begin), (size_t) (data_low - data_low_begin), (size_t) (data_high - data_high_begin)};
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
buf_sizes_lh encode_lh(Int16T const * in, uint8_t * SVB_RESTRICT out, uint32_t count, Int16T prev = 0)
{
    auto const keys = out;
    auto const data_low = keys + ::svb16_key_length(count);
    auto const data_high = data_low + count;

    return encode_scalar_lh<Int16T, UseDelta, UseZigzag>(in, keys, data_low, data_high, count, prev);
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
uint8_t const * decode_scalar_lh(
    gsl::span<Int16T> out_span,
    gsl::span<uint8_t const> keys_span,
    gsl::span<uint8_t const> data_l_span,
    gsl::span<uint8_t const> data_h_span,    
    Int16T prev = 0)
{
    auto const count = out_span.size();
    if (count == 0) {
        return data_l_span.begin();
    }

    auto out = out_span.begin();
    auto keys = keys_span.begin();
    auto data_l = data_l_span.begin();
    auto data_h = data_h_span.begin();

    uint8_t shift = 0;  // cycles 0 through 7 then resets
    uint8_t key_byte = *keys++;

    // need to do the arithmetic in unsigned space so it wraps
    auto u_prev = static_cast<uint16_t>(prev);
    
    for (uint32_t c = 0; c < count; c++, shift++) {
        if (shift == 8) {
            shift = 0;
            key_byte = *keys++;
        }
        
        uint16_t value = 0;
        if (((key_byte >> shift) & 0x01) == 1) {  // 2 bytes
            value = (uint16_t)*data_l++;
            value |= ((uint16_t)*data_h++ << 8);
        } 
        else {
            value = (uint16_t)*data_l++;
        }
        // uint16_t value = detail::decode_data(data_ll, data_lh, data_h, (key_byte >> shift) & 0x01);
        
        SVB16_IF_CONSTEXPR(UseZigzag) { value = detail::zigzag_decode(value); }
        
        SVB16_IF_CONSTEXPR(UseDelta)
        {
            value += u_prev;
            u_prev = value;
        }
        
        *out++ = static_cast<Int16T>(value);
    }

    assert(out == out_span.end());
    assert(keys == keys_span.end());
    assert(data_l <= data_l_span.end());   
    assert(data_h <= data_h_span.end());
    
    return data_h;
}

template <typename Int16T, bool UseDelta, bool UseZigzag>
size_t decode_lh(gsl::span<Int16T> out, gsl::span<uint8_t const> in, uint32_t dll, uint32_t dlh, Int16T prev = 0)
{
    auto keys_length = ::svb16_key_length(out.size());
    auto const keys = in.subspan(0, keys_length);
    auto const data_l = in.subspan(keys_length);
    auto const data_h = in.subspan(keys_length + dll);

    return decode_scalar_lh<Int16T, UseDelta, UseZigzag>(out, keys, data_l, data_h, prev) - in.begin();
}

} // namespace svb16

namespace pgnano {
    
    arrow::Result<std::size_t> compress_signal_lh(  // COMPRESOR LOW_HIGH //
    gsl::span<std::int16_t const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination,
    pod5::ReadData const & read_data,
    bool is_last_batch)
{

    auto t1 = std::chrono::steady_clock::now();

        // --------------- First compress the samples using svb --------------- //
    auto const max_size = svb16_max_encoded_length(samples.size());
    ARROW_ASSIGN_OR_RAISE(auto intermediate, arrow::AllocateResizableBuffer(max_size, pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto const encoded_sz = svb16::encode_lh<int16_t, UseDelta, UseZigzag>(
        samples.data(), intermediate->mutable_data(), samples.size());

    auto const encoded_count = encoded_sz.data_l_sz + encoded_sz.data_h_sz + encoded_sz.keys_sz;

    full_size_keys += encoded_sz.keys_sz;
    full_size_M += encoded_sz.data_l_sz;
    full_size_Lhigh += encoded_sz.data_h_sz;

    total_samples += samples.size();


    ARROW_RETURN_NOT_OK(intermediate->Resize(encoded_count));

    // --------------- Separate keys and data --------------- //
    auto const keys_size = ::svb16_key_length(samples.size());
    auto keys_span = gsl::span<uint8_t>(intermediate->mutable_data(), keys_size);
    auto data_low_span = gsl::span<uint8_t>(intermediate->mutable_data() + keys_size, samples.size());
    auto data_high_span = gsl::span<uint8_t>(intermediate->mutable_data() + keys_size + samples.size(), encoded_count - keys_size - samples.size());

    // --------------- Now compress the keys using zstd --------------- //
    size_t const zstd_keys_max_size = ZSTD_compressBound(keys_span.size());
    if (ZSTD_isError(zstd_keys_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for keys");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_keys_result = arrow::AllocateResizableBuffer(zstd_keys_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_keys, compressed_keys_result);

    size_t const compressed_keys_size = ZSTD_compress(
        compressed_keys->mutable_data(), compressed_keys->size(),
        keys_span.data(), keys_span.size(), 1);
    if (ZSTD_isError(compressed_keys_size)) {
        return pod5::Status::Invalid("Failed to compress keys");
    }

    // --------------- Now compress the data_low using zstd --------------- //
    size_t const zstd_data_low_max_size = ZSTD_compressBound(data_low_span.size());
    if (ZSTD_isError(zstd_data_low_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_low_result = arrow::AllocateResizableBuffer(zstd_data_low_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_low, compressed_data_low_result);
 
    size_t const compressed_data_low_size = ZSTD_compress(
        compressed_data_low->mutable_data(), compressed_data_low->size(),
        data_low_span.data(), data_low_span.size(), 1);
    if (ZSTD_isError(compressed_data_low_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

        // --------------- Now compress the data_high using zstd --------------- //
    size_t const zstd_data_high_max_size = ZSTD_compressBound(data_high_span.size());
    if (ZSTD_isError(zstd_data_high_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    // Get the raw pointer from the shared_ptr before calling mutable_data()
    auto compressed_data_high_result = arrow::AllocateResizableBuffer(zstd_data_high_max_size, pool);
    ARROW_ASSIGN_OR_RAISE(auto compressed_data_high, compressed_data_high_result);
 
    size_t const compressed_data_high_size = ZSTD_compress(
        compressed_data_high->mutable_data(), compressed_data_high->size(),
        data_high_span.data(), data_high_span.size(), 1);
    if (ZSTD_isError(compressed_data_high_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }

    auto write_ptr = destination.data();

    // Copy the size of compressed keys at the beginning of the destination buffer
    std::memcpy(write_ptr, &compressed_keys_size, sizeof(compressed_keys_size));
    write_ptr += sizeof(compressed_keys_size);

    // Copy compressed keys after the size in the destination buffer
    std::memcpy(write_ptr, compressed_keys->data(), compressed_keys_size);
    write_ptr += compressed_keys_size;

    // Copy the size of compressed data_low
    std::memcpy(write_ptr, &compressed_data_low_size, sizeof(compressed_data_low_size));
    write_ptr += sizeof(compressed_data_low_size);

    // Copy compressed data_low 
    std::memcpy(write_ptr,compressed_data_low->data(), compressed_data_low_size);
    write_ptr += compressed_data_low_size;

    // Copy compressed data_high 
    std::memcpy(write_ptr,compressed_data_high->data(), compressed_data_high_size);    
    write_ptr += compressed_data_high_size;

    auto t2 = std::chrono::steady_clock::now();

    compression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

    comp_size_keys += sizeof(compressed_keys_size) + compressed_keys_size;
    comp_size_M += sizeof(compressed_data_low_size) + compressed_data_low_size;
    comp_size_Lhigh += compressed_data_high_size;

    unsigned long compr_size = compressed_keys_size + compressed_data_low_size + compressed_data_high_size + sizeof(compressed_keys_size) + sizeof(compressed_data_low_size);

    return compr_size;
}

pod5::Status decompress_signal_lh(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination,
    pgnano::PGNanoReaderState & state)
{

    auto t1d = std::chrono::steady_clock::now();
    
    auto read_ptr = compressed_bytes.data();

    // Extract the size of compressed keys from the beginning of the compressed_bytes    
    std::size_t compressed_keys_size;
    std::memcpy(&compressed_keys_size, read_ptr, sizeof(compressed_keys_size));
    read_ptr += sizeof(compressed_keys_size);


    // Se calculan los tamaños de keys descomprimidos:
    unsigned long long const decompressed_zstd_keys_size =
        ZSTD_getFrameContentSize(read_ptr, compressed_keys_size);
    if (ZSTD_isError(decompressed_zstd_keys_size)) {
        return pod5::Status::Invalid(
            "keys Input data not compressed by zstd: (",
            decompressed_zstd_keys_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_keys_size),
            ")");
    }
    read_ptr += compressed_keys_size;
    
    // Extract the size of compressed data_low_low    
    std::size_t compressed_data_low_size;
    std::memcpy(&compressed_data_low_size, read_ptr, 
        sizeof(compressed_data_low_size));
    read_ptr += sizeof(compressed_data_low_size);

    // Se calculan los tamaños de data_low_low descomprimidos:
    unsigned long long const decompressed_zstd_data_low_size =
        ZSTD_getFrameContentSize(read_ptr, 
            compressed_data_low_size);
    if (ZSTD_isError(decompressed_zstd_data_low_size)) {
        return pod5::Status::Invalid(
            "data low Input data not compressed by zstd: (",
            decompressed_zstd_data_low_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_low_size),
            ")");
    }
    read_ptr += compressed_data_low_size;
    
    // Extract the size of compressed data_high

    std::size_t compressed_data_high_size = 
        compressed_bytes.size() - sizeof(compressed_keys_size) - compressed_keys_size - sizeof(compressed_data_low_size) 
        - compressed_data_low_size; 

    // Se calculan los tamaños de data_high descomprimidos:
    unsigned long long const decompressed_zstd_data_high_size = 
        ZSTD_getFrameContentSize(read_ptr,
         compressed_data_high_size);
    if (ZSTD_isError(decompressed_zstd_data_high_size)) {
        return pod5::Status::Invalid(
            "data high Input data not compressed by zstd: (",
            decompressed_zstd_data_high_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_data_high_size),
            ")");
    }
    read_ptr += compressed_data_high_size;

    // Calculo el allocation_padding y creo el buffer intermediate
    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer(decompressed_zstd_keys_size + 
                                    decompressed_zstd_data_low_size +
                                    decompressed_zstd_data_high_size +
                                    allocation_padding, pool));
    


    read_ptr = compressed_bytes.data() + sizeof(compressed_keys_size);
    auto write_ptr = intermediate->mutable_data();    

    // Descomprimo Keys
    size_t const decompress_res_keys = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_keys_size,
        read_ptr,
        compressed_keys_size);
    if (ZSTD_isError(decompress_res_keys)) {
        return pod5::Status::Invalid(
            "keys Input data failed to decompress using zstd: (",
            decompress_res_keys,
            " ",
            ZSTD_getErrorName(decompress_res_keys),
            ")");
    }

    read_ptr += compressed_keys_size + sizeof(compressed_data_low_size);
    write_ptr += decompressed_zstd_keys_size;
    
    // Descomprimo Data_low
    size_t const decompress_res_data_low = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_low_size,
        read_ptr,
        compressed_data_low_size);
    if (ZSTD_isError(decompress_res_data_low)) {
        return pod5::Status::Invalid(
            "data low Input data failed to decompress using zstd: (",
            decompress_res_data_low,
            " ",
            ZSTD_getErrorName(decompress_res_data_low),
            ")");
    }

    read_ptr += compressed_data_low_size;
    write_ptr += decompressed_zstd_data_low_size;

    // Descomprimo Data_high
    size_t const decompress_res_data_high = ZSTD_decompress(
        write_ptr,
        decompressed_zstd_data_high_size,
        read_ptr,
        compressed_data_high_size);
    if (ZSTD_isError(decompress_res_data_high)) {
        return pod5::Status::Invalid(
            "data high Input data failed to decompress using zstd: (",
            decompress_res_data_high,
            " ",
            ZSTD_getErrorName(decompress_res_data_high),
            ")");
    }  
    

    // Now decompress the data using svb:
    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;

    auto consumed_count = svb16::decode_lh<int16_t, UseDelta, UseZigzag>(destination, gsl::make_span(intermediate->data(), 
        intermediate->size()), decompress_res_data_low, decompress_res_data_high, 0);
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }

    auto t2d = std::chrono::steady_clock::now();

    decompression_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2d - t1d).count();

    return pod5::Status::OK();
}

}   // namespace pgnano