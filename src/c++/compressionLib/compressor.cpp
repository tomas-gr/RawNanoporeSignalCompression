#include "compressor.h"

#include <memory.h>
#include <cstring>
#include "constants.h"
#include "macros.h"
#include "codecs.h"

pgnano::standalone::Compressor::Compressor(const std::string & bam_filepath, const std::string & levels_filepath)
{
    m_samples_processed = m_signal_16_bit_words_written = 0;
    //m_level_table.parse_level_table(levels_filepath);
    //m_bam_handler.open_BAM_file(bam_filepath);
    //m_bam_handler.build_query_index();
}

pgnano::standalone::Compressor::~Compressor()
{

}

void pgnano::standalone::Compressor::reset_models()
{
    m_first_byte_model_zero.reset();
    m_first_byte_model_non_zero.reset();
    m_second_byte_model.reset();
}

void pgnano::standalone::Compressor::start_encode(uint8_t * out_ptr)
{
    m_rc.output(out_ptr);
    m_rc.StartEncode();
    reset_models();
}

size_t pgnano::standalone::Compressor::finish_encode()
{
    return m_rc.FinishEncode();
}

size_t pgnano::standalone::Compressor::compress(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, int16_t * out_ptr)
{
    return compress(read_id, in_ptr, sample_count, reinterpret_cast<uint8_t*>(out_ptr));
}

size_t pgnano::standalone::Compressor::compress(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, uint8_t * out_ptr)
{
    auto compression_result = compress_signal(read_id, in_ptr, sample_count, out_ptr + header_words * sizeof(uint16_t));
    compress_header({sample_count, compression_result.is_raw}, out_ptr);
    return compression_result.words_written + header_words;
}

void pgnano::standalone::Compressor::compress_header(const Header & h, uint8_t * out_ptr)
{
    std::memcpy(out_ptr, &h.sample_count, sizeof(decltype(h.sample_count)));
    std::memcpy(out_ptr + sizeof(decltype(h.sample_count)), &h.is_raw, sizeof(decltype(h.is_raw)));
}

pgnano::standalone::CompressionResult pgnano::standalone::Compressor::compress_signal(const boost::uuids::uuid & read_id, int16_t * in_ptr, uint32_t sample_count, uint8_t * out_ptr)
{
    const size_t threshold = send_raw_threshold(sample_count);
    uint_fast64_t bytes_written = 0;
    m_samples_processed += sample_count;

    start_encode(out_ptr);
    
    auto general_compression_chunk_result = compress_signal_chunk_arithmetic(in_ptr, sample_count, out_ptr, threshold);
    
    bytes_written += general_compression_chunk_result.bytes_written;
    bytes_written += finish_encode();
    auto words_written = (bytes_written >> 1) + (bytes_written & 0x1);
    m_signal_16_bit_words_written += words_written;
    return {words_written, general_compression_chunk_result.is_raw};
}

pgnano::standalone::IntermediateCompressionResult pgnano::standalone::Compressor::compress_signal_chunk_arithmetic(int16_t *in_ptr, uint32_t samples_in_chunk, uint8_t * out_ptr, size_t abort_threshold)
{
    int16_t previous_sample = initial_sample;
    uint_fast64_t bytes_written = 0;

    for (size_t i = 0; i < samples_in_chunk; i++)
    {
        uint8_t first_byte, second_byte;
        int16_t this_sample = in_ptr[i];
        uint16_t mapped_val = signed_encode(this_sample - previous_sample);
        previous_sample = this_sample;
        BREAK_INTO_BYTES(first_byte, second_byte, mapped_val);
        bytes_written += m_second_byte_model.encode_symbol(&m_rc, second_byte);
        if (second_byte)
            bytes_written += m_first_byte_model_non_zero.encode_symbol(&m_rc, first_byte);
        else
            bytes_written += m_first_byte_model_zero.encode_symbol(&m_rc, first_byte);
        if (bytes_written > abort_threshold)
        {
            std::memcpy(in_ptr, out_ptr, samples_in_chunk * sizeof(int16_t));
            return {samples_in_chunk * sizeof(int16_t), true};
        }
    }

    return {bytes_written, false};
}
