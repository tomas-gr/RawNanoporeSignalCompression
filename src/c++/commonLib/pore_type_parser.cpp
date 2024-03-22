#include "pore_type_parser.h"

pgnano::PoreTypeParser::PoreTypeParser()
{
    m_regex[0] = std::regex(".*R?10(\\.|_|-)4((\\.|_|-)1)?.*");
    m_regex[1] = std::regex(".*R?10((\\.|_|-)3)?.*");
    m_regex[2] = std::regex(".*R?9(((\\.|_|-)4)((\\.|_|-)1)?)?");
    m_match_map[0] = PGNANO_PORE_TYPE::R10_4_1;
    m_match_map[1] = PGNANO_PORE_TYPE::R10_3;
    m_match_map[2] = PGNANO_PORE_TYPE::R9_4_1;
    m_match_map[3] = PGNANO_PORE_TYPE::UNKNOWN;
}

pgnano::PGNANO_PORE_TYPE pgnano::PoreTypeParser::parse_pore_type(const std::string & pore_description)
{
    uint_fast8_t i = 0;
    for (; i < types_of_pore; i++)
    {
        if (std::regex_match(pore_description,m_regex[i]))
            break;
    }
    return m_match_map[i];
}