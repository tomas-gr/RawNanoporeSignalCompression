#include "level_table.h"

#include <iostream>
#include <fstream>

#include "math_utils.h"

pgnano::LevelTable::LevelTable()
{
    m_line_regex = std::regex("^([A|T|C|G]+)\\s+(-?\\d+(\\.\\d+)?(e[\\+|-]?\\d+)?)\\s*$");
}

void pgnano::LevelTable::parse_level_table(const std::string & path)
{
    m_level_table_file_path = path;

    std::ifstream file(m_level_table_file_path);
    std::string line;
    if (!std::getline(file, line))
    {
        throw "Empty levels file!";
    }

    pgnano::PackedKMer kmer;
    double level;
    uint32_t kmer_size;
    std::pair<pgnano::PackedKMer, double> x_tuple;

    kmer_size = parse_kmer_size(line);
    m_rep.reserve(uint_pow<size_t>(pgnano::TotalUniqueBases, kmer_size));
    
    x_tuple = parse_line(line);
    kmer = x_tuple.first;
    level = x_tuple.second;

    m_rep[kmer] = level;

    while (std::getline(file, line))
    {
        x_tuple = parse_line(line);
        kmer = x_tuple.first;
        level = x_tuple.second;        
        m_rep[kmer] = level;
    }
    file.close();
}
// TODO: this packing and unpacking into a tuple is quite inefficient...
std::pair<pgnano::PackedKMer, double> pgnano::LevelTable::parse_line(const std::string & line)
{
    std::smatch matches;
    if (std::regex_search(line, matches, m_line_regex))
    {
        pgnano::PackedKMer kmer = pgnano::parse_kmer(matches[1].str());
        double level = std::stod(matches[2].str());
        return {kmer, level};
    } else
    {
        throw "Error parsing level table!";
    }
}

double pgnano::LevelTable::operator[](const pgnano::PackedKMer & idx)
{
    return m_rep[idx];
}