#pragma once

#include <stdint.h>

namespace pgnano
{

enum Nucleotides {
    A, C, G, T
};

constexpr uint8_t A_PACKED = 0x0;
constexpr uint8_t C_PACKED = 0x1;
constexpr uint8_t G_PACKED = 0x2;
constexpr uint8_t T_PACKED = 0x3;

constexpr uint8_t PackedNucleotideBitsize = 2;
constexpr uint8_t TotalUniqueBases = 4;

typedef uint8_t PackedNucleotide;

};