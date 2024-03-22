#include "pod5_format/pgnano/compressor.h"
#include "pod5_format/pgnano/decompressor.h"
#include "pod5_format/read_table_utils.h"
#include "pod5_format/pgnano/pore_type_server.h"
#include <random>

// FIXME: use a unit test framework (Catch2)
void case_1() 
{
    pod5::ReadData read_data;
    size_t samples = 1024000;
    int16_t x[samples];
    uint8_t out[4*samples];
    for (size_t i = 0; i < samples; i++)
    {
        x[i] = i % 256;
    }
    pgnano::Compressor compressor;
    pgnano::PGNanoWriterState writing_state;
    writing_state.m_pore_type_server->put_pore_type(read_data.pore_type,"");
    compressor.compress(read_data, samples, x, out, writing_state);
    compressor.reset();
}

void case_2()
{
    pod5::ReadData read_data;
    size_t samples = 1024000;
    int16_t compressed_in[samples];
    uint8_t compressed_out[4*samples];
    int16_t decompressed_out[samples];
    for (size_t i = 0; i < samples; i++)
    {
        compressed_in[i] = i % 256;
    }
    pgnano::Compressor compressor;
    pgnano::PGNanoWriterState writing_state;
    pgnano::PGNanoReaderState reader_state;
    writing_state.m_pore_type_server->put_pore_type(read_data.pore_type,"");
    compressor.compress(read_data, samples, compressed_in, compressed_out, writing_state);
    compressor.reset();
    pgnano::Decompressor decompressor;
    decompressor.decompress(compressed_out, decompressed_out, 100, reader_state);
    for (size_t i = 0; i < samples; i++)
    {
        assert(compressed_in[i] == decompressed_out[i]);
    }
}

void case_3()
{
    pod5::ReadData read_data;
    read_data.pore_type = 0;
    size_t samples = 1024;
    pgnano::Compressor compressor;
    pgnano::Decompressor decompressor;
    pgnano::PGNanoWriterState writing_state;
    pgnano::PGNanoReaderState reader_state;
    writing_state.m_pore_type_server->put_pore_type(read_data.pore_type,"");
    int16_t compressed_in[samples];
    uint8_t compressed_out[compressor.compressed_signal_max_size(samples,read_data)];
    int16_t decompressed_out[samples];
    for (size_t i = 0; i < samples; i++)
    {
        compressed_in[i] = i % 256;
    }
    compressor.compress(read_data, samples, compressed_in, compressed_out, writing_state);
    compressor.reset();
    decompressor.decompress(compressed_out, decompressed_out, 100, reader_state);
    for (size_t i = 0; i < samples; i++)
    {
        assert(compressed_in[i] == decompressed_out[i]);
    }
}

void case_4(size_t samples, int seed)
{
    std::mt19937_64 rnd;
    rnd.seed(seed);
    pod5::ReadData read_data;
    read_data.pore_type = 0;
    pgnano::Compressor compressor;
    pgnano::Decompressor decompressor;
    pgnano::PGNanoWriterState writing_state;
    pgnano::PGNanoReaderState reader_state;
    writing_state.m_pore_type_server->put_pore_type(read_data.pore_type,"");
    int16_t compressed_in[samples];
    uint8_t compressed_out[compressor.compressed_signal_max_size(samples,read_data)];
    int16_t decompressed_out[samples];
    for (size_t i = 0; i < samples; i++)
    {
        compressed_in[i] = rnd() % 256;
    }
    compressor.compress(read_data, samples, compressed_in, compressed_out, writing_state);
    compressor.reset();
    decompressor.decompress(compressed_out, decompressed_out, 100, reader_state);
    for (size_t i = 0; i < samples; i++)
    {
        assert(compressed_in[i] == decompressed_out[i]);
    }
}

int main()
{
    //case_1();
    //case_2();
    //case_3();
    size_t sizes[] = {1024, 1 << 12, 1 << 14, 1 << 15, 1 << 16, 1 << 20};
    int seeds[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (auto size : sizes)
        for (auto seed : seeds) 
            case_4(size,seed);
}