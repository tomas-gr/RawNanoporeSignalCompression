#include "decompressor.h"

#include <cstring>

#include "constants.h"
#include "codecs.h"

pgnano::standalone::Decompressor::Decompressor(const std::string & bam_filename, const std::string & levels_filename)
{

}

pgnano::standalone::Decompressor::~Decompressor()
{

}

void pgnano::standalone::Decompressor::reset_models()
{
    m_first_byte_model_zero.reset();
    m_first_byte_model_non_zero.reset();
    m_second_byte_model.reset();
}


uint32_t pgnano::standalone::Decompressor::retrieve_sample_count(const int16_t * in) noexcept
{
    return retrieve_sample_count(reinterpret_cast<const uint8_t *>(in));
}

void pgnano::standalone::Decompressor::decompress(const boost::uuids::uuid & read_id, int16_t * in, int16_t * out)
{
    decompress(read_id, reinterpret_cast<uint8_t*>(in), out);
}

void pgnano::standalone::Decompressor::decompress(const boost::uuids::uuid & read_id, uint8_t * in, int16_t * out)
{
    Header h;
    decompress_header(in, h);
    decompress_signal(read_id, in + header_words * sizeof(uint16_t), out, h);
}

void pgnano::standalone::Decompressor::decompress_header(const uint8_t * in, Header & h)
{
    std::memcpy(&h.sample_count, in, sizeof(decltype(h.sample_count)));
    std::memcpy(&h.is_raw, in + sizeof(decltype(h.sample_count)), sizeof(decltype(h.is_raw)));
}

void pgnano::standalone::Decompressor::decompress_signal(const boost::uuids::uuid & read_id, uint8_t * in, int16_t * out, const Header & h)
{
    if (h.is_raw)
    {
        std::memcpy(out, in, h.sample_count * sizeof(int16_t));
        return;
    }

    m_rc.input(in);
    m_rc.StartDecode();
    reset_models();
    int16_t previous_sample = initial_sample;
    uint8_t * current_in_ptr = in;

    
    for (decltype(h.sample_count) i = 0; i < h.sample_count; i++)
    {
        uint8_t first_byte, second_byte;
        second_byte = m_second_byte_model.decode_symbol(&m_rc);
        if (second_byte)
            first_byte = m_first_byte_model_non_zero.decode_symbol(&m_rc);
        else
            first_byte = m_first_byte_model_zero.decode_symbol(&m_rc);
        int16_t signed_val = signed_decode(first_byte | ((uint16_t)(second_byte) << 8));
        int16_t sample = signed_val + previous_sample;
        previous_sample = sample;
        out[i] = sample;
    }

    m_rc.FinishDecode();
}

uint32_t pgnano::standalone::Decompressor::retrieve_sample_count(const uint8_t *in) noexcept
{
    Header h;
    decompress_header(in, h);
    return h.sample_count;
}