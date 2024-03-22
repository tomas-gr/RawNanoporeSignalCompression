#include "BAM_enriched_read.h"

#include <algorithm>
#include "sam.h"


// FIXME: sequence/mv_tag <= 0 y != null => ¿Is defined?
pgnano::BAMEnrichedRead::BAMEnrichedRead(const uint8_t* mv_tag, const uint8_t* sequence, int sequence_byte_length, unsigned int mv_tag_byte_length)
{
    if (mv_tag && mv_tag_byte_length > 0)
        m_mv_tag = std::make_unique<uint8_t[]>(sequence_byte_length);
    else
        m_mv_tag = nullptr;

    if (sequence && sequence_byte_length > 0)
        m_sequence = std::make_unique<uint8_t[]>(mv_tag_byte_length);
    else
        m_sequence = nullptr;
    
    if (!m_sequence || !m_mv_tag)
        return;

    std::copy(mv_tag + 1, mv_tag + mv_tag_byte_length, m_mv_tag.get());
    std::copy(sequence, sequence + sequence_byte_length, m_sequence.get());
    m_stride = m_mv_tag[0];
}

bool pgnano::BAMEnrichedRead::next()
{
    throw "Not implemented";
}

bool pgnano::BAMEnrichedRead::is_jump()
{
    throw "Not implemented";
}

pgnano::PackedNucleotide pgnano::BAMEnrichedRead::current_nucleotide()
{
    throw "Not implemented";
}

// each base is encoded in a nybble. Therefore, there are sequence_bytes * 2 bases in the sequence => How to differentiate last base???
/*
        for (size_t i = 0; i < (size_t)sequence_bytes * 2; i++)
        {
            bases[i] = bam_seqi(sequence_ptr, i);
        }
        bases[0]++;
*/