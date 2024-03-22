#pragma once

#include <vector>
#include <stdint.h>
#include <string>

#include "concepts.h"


namespace pgnano
{
// FIXME: Only 4 bases recognized

typedef uint32_t PackedKMer;

inline PackedKMer parse_kmer(const std::string & s);
inline uint8_t parse_kmer_size(const std::string & s);
inline PackedKMer add_nucleotide(PackedKMer kmer, const char nucleotide, const uint8_t k);
//inline PackedKMer parse_kmer(const std::string & s, const uint8_t k);

PackedKMer parse_kmer(const std::string & s)
{
    uint32_t res = 0;
    for (auto x : s)
    {
        switch (x)
        {
        case 'A':
            res = (res << PackedNucleotideBitsize) | A_PACKED;
            break;
        case 'C':
            res = (res << PackedNucleotideBitsize) | C_PACKED;
            break;
        case 'G':
            res = (res << PackedNucleotideBitsize) | G_PACKED;
            break;
        case 'T':
            res = (res << PackedNucleotideBitsize) | T_PACKED;
            break;
        default:
            throw "Unknown nucleotide base";
            break;
        }
    }
    return res;
}

uint8_t parse_kmer_size(const std::string & s)
{
    uint8_t i = 0;
    for (auto x : s)
    {
        if (x == 'A' || x == 'C' || x == 'G' || x == 'T')
            i++;
        else
            break;
    }
    return i;
}

PackedKMer add_nucleotide(const PackedKMer kmer, const char nucleotide, const uint8_t k)
{
    uint8_t packed_new_nucleotide;
    PackedKMer mask = (1 << (k * pgnano::PackedNucleotideBitsize)) - 1;
    switch (nucleotide)
    {
    case 'A':
        packed_new_nucleotide = pgnano::A_PACKED;
        break;
    case 'C':
        packed_new_nucleotide = pgnano::C_PACKED;
        break;
    case 'G':
        packed_new_nucleotide = pgnano::G_PACKED;
        break;
    case 'T':
        packed_new_nucleotide = pgnano::T_PACKED;
        break;
    default:
        throw "Unrecognized nucleotide!"; //TODO: throw an exception not a string...
    }
    return ((kmer << pgnano::PackedNucleotideBitsize) | packed_new_nucleotide) && mask;
}

inline constexpr PackedKMer add_nucleotide(const PackedKMer kmer, const PackedNucleotide nucleotide, const uint8_t k) noexcept
{
    return ((kmer << pgnano::PackedNucleotideBitsize) | nucleotide) && ((1 << (k * pgnano::PackedNucleotideBitsize)) - 1);
}

};