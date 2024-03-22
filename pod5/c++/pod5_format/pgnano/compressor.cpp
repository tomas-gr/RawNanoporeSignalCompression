#include "compressor.h"

#include "macros.h"
#include "codecs.h"
#include "pgnano_writer_state.h"
#include "pore_type_server.h"

namespace pgnano
{
    void Compressor::reset()
    {
        //m_high_byte_ctx_class.reset(high_byte_histogram);
        //m_low_byte_ctx_class.reset(low_byte_histogram);
        m_high_byte_ctx_class.reset();
        m_low_byte_ctx_class_non_zero.reset();
        m_low_byte_ctx_class_zero.reset();
        m_range_coder.FinishEncode();
        m_range_coder.StartEncode();
    }

    void Compressor::compress_metadata(pgnano::Metadata const & metadata, uint8_t* const & dest) 
    {
        size_t mask = 0xFF;
        for (size_t i = 0; i < sizeof(size_t); i++)
        {
            dest[i] = (metadata.samples & mask) >> (8 * i);
            mask <<= 8;
        }
        dest[sizeof(size_t)] = from_pore_type(metadata.pore_type) & 0xFF;
    }

    void Compressor::compress_header(pgnano::Header const & header, uint8_t* const & dest)
    {
        dest[0] = header.is_raw ? 1 : 0;
        compress_metadata(header.metadata, dest + 1);
    }

    // TODO: const correctness
    CompressionResult Compressor::compress_signal(pod5::ReadData const & read_data, std::size_t sample_count, int16_t const * samples, uint8_t* const & dest)
    {
        constexpr int16_t initial_sample = 0;
        size_t threshold = send_raw_threshold(sample_count, read_data);
        m_range_coder.output(dest);
        m_range_coder.StartEncode();
        int16_t const *iptr = samples;
        int16_t previous_sample = initial_sample;
        size_t bytes_written = 0;
        for (std::size_t i = 0; i < sample_count; i++, iptr++)
        {
            uint8_t first_byte, second_byte;
            int16_t this_sample = *iptr;
            uint16_t mapped_val = signed_encode(this_sample - previous_sample);
            previous_sample = this_sample;
            BREAK_INTO_BYTES(first_byte, second_byte, mapped_val);
            bytes_written += m_high_byte_ctx_class.encode_symbol(&m_range_coder, second_byte);
            if (second_byte)
                bytes_written += m_low_byte_ctx_class_non_zero.encode_symbol(&m_range_coder, first_byte);
            else
                bytes_written += m_low_byte_ctx_class_zero.encode_symbol(&m_range_coder, first_byte);
            if (bytes_written > threshold)
            {
                std::memcpy(const_cast<int16_t *>(samples), dest, sample_count * sizeof(int16_t));
                Compressor::update_compression_stats(sample_count * sizeof(int16_t), sample_count);
                return {sample_count * sizeof(int16_t), true};
            }
        }
        bytes_written += m_range_coder.FinishEncode();
        Compressor::update_compression_stats(bytes_written, sample_count);
        return {bytes_written, false};
    }

    size_t Compressor::compress(pod5::ReadData const & read_data, std::size_t sample_count, int16_t const * samples, uint8_t* const & dest, pgnano::PGNanoWriterState & state) 
    {
        pgnano::Metadata metadata;
        pgnano::Header header;
        pgnano::PGNANO_PORE_TYPE pore_type = state.m_pore_type_server->get_pore_type(read_data.pore_type);
        metadata.m_read_data = read_data;
        metadata.samples = sample_count;
        metadata.pore_type = pore_type;
        header.is_raw = false;
        header.metadata = metadata;
        size_t offset = pgnano::header_size;
        auto compression_result = compress_signal(read_data, sample_count, samples, dest + offset);
        size_t compressed_byte_count = compression_result.bytes_written;
        header.is_raw = compression_result.is_raw;
        compress_header(header, dest);//FIXME: is_raw == false always
        return compressed_byte_count + header_size;
    }

    std::atomic<std::uint_fast64_t> Compressor::bytes_written;
    std::atomic<std::uint_fast64_t> Compressor::total_sample_size;
}