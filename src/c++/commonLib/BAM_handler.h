#pragma once

#include <string.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <exception>
#include <stdexcept>

#include <semaphore.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/functional/hash.hpp>

#include "sam.h"
#include "hfile.h"
#include "bgzf.h"
#include "tbx.h"
#include "BAM_enriched_read.h"

namespace pgnano
{

class BAMHandler
{
public:
    static constexpr std::size_t INITIAL_WORKSET_SIZE = 32; // TODO: initialize hash tables with atleast this size
    BAMHandler() { m_hts_file = nullptr; m_bam_data = nullptr; }
    inline void open_BAM_file(const std::string & BAM_file) { open_BAM_file(BAM_file, "/dev/null"); }
    void open_BAM_file(const std::string & BAM_file, const std::string & index_file)
    {
        m_bam_data = bam_init1();
        if (m_bam_data == NULL)
        {}

        m_BAM_file_path = BAM_file;
        m_index_file_path = index_file;
        htsFile* htsfile = hts_open(BAM_file.c_str(), "r");
        if (htsfile == NULL)
        {} //FIXME: error management
        m_hts_file = htsfile;

        m_bam_hdr = sam_hdr_read(m_hts_file);
        if (m_bam_hdr == NULL)
        {}
    }

    ~BAMHandler()
    {
        if (m_hts_file)
            hts_close(m_hts_file);
        if (m_bam_data)
            bam_destroy1(m_bam_data);
    }

    void build_query_index()
    {
        // FIXME: actually revise if it is necessary to reopen handles
        htsFile* htsfile;
        htsfile = hts_open(m_BAM_file_path.c_str(), "r");
        if (htsfile == NULL)
        {} //FIXME: error management
        
        bam_hdr_t * hdr;
        hdr = sam_hdr_dup(m_bam_hdr);
        if (hdr == NULL)
        {}

        int error_code = 0;
        off_t pos;
        while ((pos = bgzf_tell(hts_get_bgzfp(htsfile)) >= 0) && ((error_code = sam_read1(htsfile, hdr, m_bam_data)) >= 0))
        {
            std::string q_name = std::string(bam_get_qname(m_bam_data)); //Assumes that q_name == read_id
            index_read(parse_uuid(q_name),pos);
        }

        sam_hdr_destroy(hdr);
        hts_close(htsfile);

        if (error_code == -1)
            std::cerr << "End of stream";
        return; // End of stream
        if (error_code < -1)
            throw "Error when reading SAM file"; // error
    }


    // FIXME: The operation implies (WITH REGARDS TO get_read_data)
    // a file decompression, so maybe it's better to cache the results and read in batches => 
    // DO NOT DO PREMATURE OPTIMIZATION; PROFILE AN CHECK => 
    // THERE IS ALREADY SOME FORM OF CACHING INSIDE THE LIBRARY, 
    // FIRST I MUST VERIFY IF THIS IS ENOUGH FOR MY USAGE
    // To be called on first chunk of a signal

    std::unique_ptr<BAMEnrichedRead> get_read_data(const boost::uuids::uuid & uuid)
    {
        auto it = m_read_bam_seek_idx.find(uuid);
        if (it == m_read_bam_seek_idx.end())
            throw new std::runtime_error("Could not find the requested read in the indexed bam file"); //FIXME: add a fallback compression mechanism
        auto idx = it->second;
        int error_code;
        if (bgzf_seek(hts_get_bgzfp(m_hts_file),idx,SEEK_SET))
        {} // FIXME: Handle error

        error_code = sam_read1(m_hts_file, m_bam_hdr, m_bam_data);
        if (error_code < 0)
            throw new std::runtime_error("Could not read the read data from the bam file");
        return build_enriched_read();
    }

private:
    inline void index_read(const boost::uuids::uuid uuid, const int seek_idx)
    {
        if (m_read_bam_seek_idx.find(uuid) != m_read_bam_seek_idx.end())
            m_read_bam_seek_idx.insert({uuid, seek_idx});
        else
            std::cerr << "Silently ignoring duplicate reads"; // TODO: a better approach would be to detect primary alignments
    }

    inline boost::uuids::uuid parse_uuid(const std::string & s) { return m_uuid_parser(s); } // TODO: return const reference to the uuid
    inline boost::uuids::uuid parse_uuid(const char* s) { return parse_uuid(std::string(s)); } //TODO: optimize this code // TODO: return const reference to the uuid

    std::unique_ptr<BAMEnrichedRead> build_enriched_read()
    {
        auto mv_tag_ptr = bam_aux_get(m_bam_data, "mv");//FIXME: TODO: ver remora t
        if (mv_tag_ptr == NULL)
        {
            if (errno == ENOENT)
                std::cerr << "mv tag not found";
            if (errno == EINVAL)
                std::cerr << "corrupt data";
        //FIXME: fallback when no move tag can be retrieved
        }

        auto sequence_ptr = bam_get_seq(m_bam_data);
        auto sequence_bytes = m_bam_data->l_data;
        auto mv_tag_bytes = bam_get_l_aux(m_bam_data);

        if (sequence_bytes < 0)
            throw "Negative bytes in read's sequence data";

        return std::make_unique<BAMEnrichedRead>(mv_tag_ptr, sequence_ptr, sequence_bytes, mv_tag_bytes);
    }

    std::string m_BAM_file_path, m_index_file_path;
    htsFile* m_hts_file;
    bam1_t* m_bam_data;
    sam_hdr_t* m_bam_hdr;
    std::unordered_map<const boost::uuids::uuid, const int, boost::hash<boost::uuids::uuid>> m_read_bam_seek_idx;
    boost::uuids::string_generator m_uuid_parser;
};

};



//FIXME: UNMAPPED READS

/*

/// Create a BAM/CRAM iterator
/// @param idx     Index
///    @param tid     Target id
///    @param beg     Start position in target
///    @param end     End position in target
///    @return An iterator on success; NULL on failure
///
///The following special values (defined in htslib/hts.h)can be used for @p tid.
///When using one of these values, @p beg and @p end are ignored.
///
///  HTS_IDX_NOCOOR iterates over unmapped reads sorted at the end of the file
///  HTS_IDX_START  iterates over the entire file
///  HTS_IDX_REST   iterates from the current position to the end of the file
///  HTS_IDX_NONE   always returns "no more alignment records"
///
///When using HTS_IDX_REST or HTS_IDX_NONE, NULL can be passed in to @p idx.
///
///HTSLIB_EXPORT
///hts_itr_t *sam_itr_queryi(const hts_idx_t *idx, int tid, hts_pos_t beg, hts_pos_t end);

*/

// FIXME: error handling
// FIXME: memory allocations and deallocations
// FIXME: read index
// FIXME: por algún motivo hay entradas duplicadas => Puede ser lo de reverse strand y eso?
