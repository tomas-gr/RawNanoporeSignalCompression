#include "pod5_format/signal_compression.h"

#include "pod5_format/svb16/decode.hpp"
#include "pod5_format/svb16/encode.hpp"

#include <arrow/buffer.h>
#include <zstd.h>

namespace pod5 {

std::size_t compressed_signal_max_size(std::size_t sample_count)
{
    auto const max_svb_size = svb16_max_encoded_length(sample_count);
    auto const zstd_compressed_max_size = ZSTD_compressBound(max_svb_size);
    return zstd_compressed_max_size;
}

arrow::Result<std::size_t> compress_signal(
    gsl::span<SampleType const> const & samples,
    arrow::MemoryPool * pool,
    gsl::span<std::uint8_t> const & destination)
{
    // First compress the data using svb:
    auto const max_size = svb16_max_encoded_length(samples.size());
    ARROW_ASSIGN_OR_RAISE(auto intermediate, arrow::AllocateResizableBuffer(max_size, pool));

    static constexpr bool UseDelta = true;
    static constexpr bool UseZigzag = true;
    auto const encoded_count = svb16::encode<SampleType, UseDelta, UseZigzag>(
        samples.data(), intermediate->mutable_data(), samples.size());
    ARROW_RETURN_NOT_OK(intermediate->Resize(encoded_count));

    // Now compress the svb data using zstd:
    size_t const zstd_compressed_max_size = ZSTD_compressBound(intermediate->size());
    if (ZSTD_isError(zstd_compressed_max_size)) {
        return pod5::Status::Invalid("Failed to find zstd max size for data");
    }

    /* Compress.
     * If you are doing many compressions, you may want to reuse the context.
     * See the multiple_simple_compression.c example.
     */
    size_t const compressed_size = ZSTD_compress(
        destination.data(), destination.size(), intermediate->data(), intermediate->size(), 1);
    if (ZSTD_isError(compressed_size)) {
        return pod5::Status::Invalid("Failed to compress data");
    }
    return compressed_size;
}

arrow::Result<std::shared_ptr<arrow::Buffer>> compress_signal(
    gsl::span<SampleType const> const & samples,
    arrow::MemoryPool * pool)
{
    ARROW_ASSIGN_OR_RAISE(
        std::shared_ptr<arrow::ResizableBuffer> out,
        arrow::AllocateResizableBuffer(compressed_signal_max_size(samples.size()), pool));

    ARROW_ASSIGN_OR_RAISE(
        auto final_size,
        compress_signal(samples, pool, gsl::make_span(out->mutable_data(), out->size())));

    ARROW_RETURN_NOT_OK(out->Resize(final_size));
    return out;
}

POD5_FORMAT_EXPORT arrow::Status decompress_signal(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    arrow::MemoryPool * pool,
    gsl::span<std::int16_t> const & destination)
{
    // First decompress the data using zstd:
    unsigned long long const decompressed_zstd_size =
        ZSTD_getFrameContentSize(compressed_bytes.data(), compressed_bytes.size());
    if (ZSTD_isError(decompressed_zstd_size)) {
        return pod5::Status::Invalid(
            "Input data not compressed by zstd: (",
            decompressed_zstd_size,
            " ",
            ZSTD_getErrorName(decompressed_zstd_size),
            ")");
    }

    auto allocation_padding = svb16::decode_input_buffer_padding_byte_count();
    ARROW_ASSIGN_OR_RAISE(
        auto intermediate,
        arrow::AllocateResizableBuffer(decompressed_zstd_size + allocation_padding, pool));
    size_t const decompress_res = ZSTD_decompress(
        intermediate->mutable_data(),
        intermediate->size(),
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
    auto consumed_count = svb16::decode<SampleType, UseDelta, UseZigzag>(
        destination, gsl::make_span(intermediate->data(), intermediate->size()));
    if ((consumed_count + allocation_padding) != (std::size_t)intermediate->size()) {
        return pod5::Status::Invalid("Remaining data at end of signal buffer");
    }

    return pod5::Status::OK();
}

arrow::Result<std::shared_ptr<arrow::Buffer>> decompress_signal(
    gsl::span<std::uint8_t const> const & compressed_bytes,
    std::uint32_t samples_count,
    arrow::MemoryPool * pool)
{
    ARROW_ASSIGN_OR_RAISE(
        std::shared_ptr<arrow::ResizableBuffer> out,
        arrow::AllocateResizableBuffer(samples_count * sizeof(SampleType), pool));

    auto signal_span = gsl::make_span(out->mutable_data(), out->size()).as_span<std::int16_t>();

    ARROW_RETURN_NOT_OK(decompress_signal(compressed_bytes, pool, signal_span));
    return out;
}
}  // namespace pod5
