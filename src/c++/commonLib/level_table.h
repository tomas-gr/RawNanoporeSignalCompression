#pragma once

#include <string>
#include <regex>

#include "kmer.h"

namespace pgnano
{

class LevelTable
{
public:
    LevelTable();
    void parse_level_table(const std::string & path);
    double operator[](const pgnano::PackedKMer & idx);
    inline constexpr uint8_t kmer_size() const noexcept { return m_kmer_size; };
private:
    std::string m_level_table_file_path;
    std::regex m_line_regex;
    std::pair<pgnano::PackedKMer, double> parse_line(const std::string & line);
    std::vector<double> m_rep;
    uint8_t m_kmer_size;
};

};