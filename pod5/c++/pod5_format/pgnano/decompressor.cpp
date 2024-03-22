#include "decompressor.h"

#include "codecs.h"

namespace pgnano
{
    void Decompressor::reset()
    {
        //m_high_byte_ctx_class.reset(high_byte_histogram);
        //m_low_byte_ctx_class.reset(low_byte_histogram);
        m_high_byte_ctx_class.reset();
        m_low_byte_ctx_class_zero.reset();
        m_low_byte_ctx_class_non_zero.reset();
        m_range_coder.FinishDecode();
        m_range_coder.StartDecode();//FIXME: Estos resets serían mejor sacarlos o dejarlos pero agregar StartDecode y eso
    }

    void Decompressor::decompress(uint8_t const * const & in, int16_t * const & dest, size_t const & bytes, PGNanoReaderState & state)
    {
        size_t offset;
        pgnano::Header header;
        decompress_header(in, header);
        offset = pgnano::header_size;
        assert(bytes >= offset);
        auto signal_in = in + offset;
        decompress_signal(signal_in, header, dest, bytes - offset);
    }

    void Decompressor::decompress_metadata(uint8_t const * const & in, pgnano::Metadata & metadata)
    {
        metadata.samples = 0;
        for (size_t i = 0; i < sizeof(size_t); i++)
        {
            metadata.samples |= in[i] << (8 * i);
        }
        metadata.pore_type = to_pore_type(in[sizeof(size_t)]);
    }

    void Decompressor::decompress_header(uint8_t const * const & in, pgnano::Header & header)
    {
        header.is_raw = in[0] & 0x1;
        decompress_metadata(in + 1, header.metadata);
    }

    void Decompressor::decompress_signal(uint8_t const * const & in, pgnano::Header const & header, int16_t * const & dest, size_t const & bytes)
    {
        if (header.is_raw)
        {
            std::memcpy(dest,in,header.metadata.samples);
            return;
        }

        constexpr int16_t initial_sample = 0;
        int16_t previous_sample = initial_sample;
        m_range_coder.input(const_cast<uint8_t*>(in));// FIXME: const correctness
        m_range_coder.StartDecode();
        auto dest_ptr = dest;
        for (size_t i = 0; i < header.metadata.samples; i++)
        {
            int16_t this_decode;
            int16_t second_byte = m_high_byte_ctx_class.decode_symbol(&m_range_coder) << 8;//FIXME: int overflow...
            int16_t first_byte;
            if (second_byte)
                first_byte = m_low_byte_ctx_class_non_zero.decode_symbol(&m_range_coder);
            else
                first_byte = m_low_byte_ctx_class_zero.decode_symbol(&m_range_coder);
            this_decode = signed_decode(first_byte | second_byte);
            *dest_ptr = this_decode + previous_sample;
            previous_sample = this_decode + previous_sample;
            dest_ptr++;
        }
        m_range_coder.FinishDecode();
    }
}