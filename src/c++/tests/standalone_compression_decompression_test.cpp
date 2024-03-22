#include "compressor.h"
#include "decompressor.h"

#include <random>
#include <stdint.h>
#include "boost/uuid/uuid_generators.hpp"

void case_1(uint32_t LENGTH, int seed)
{
    std::mt19937_64 rnd;
    rnd.seed(seed);
    //constexpr uint32_t LENGTH = 10;
    pgnano::standalone::Compressor compressor("/dev/null", "/dev/null");
    pgnano::standalone::Decompressor decompressor("/dev/null", "/dev/null");
    int16_t* uncompressed_buffer = new int16_t[LENGTH];
    int16_t* compressed_buffer = new int16_t[compressor.compressed_signal_max_size(LENGTH)];
    for (size_t i = 0; i < LENGTH; i++)
    {
        uncompressed_buffer[i] = rnd() % 256;
    }
    compressor.compress(boost::uuids::nil_uuid(), uncompressed_buffer, LENGTH, compressed_buffer);
    uint32_t SAMPLES = decompressor.retrieve_sample_count(compressed_buffer);
    assert(LENGTH == SAMPLES);
    int16_t* decompressed_buffer = new int16_t[SAMPLES];
    decompressor.decompress(boost::uuids::nil_uuid(), compressed_buffer, decompressed_buffer);
    for (size_t i = 0; i < LENGTH; i++)
    {
        assert(uncompressed_buffer[i] == decompressed_buffer[i]);
    }
    delete[] uncompressed_buffer;
    delete[] compressed_buffer;
    delete[] decompressed_buffer;
}

int main()
{
    size_t sizes[] = {1024, 1 << 12, 1 << 14, 1 << 15, 1 << 16, 1 << 20};
    int seeds[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (auto size : sizes)
        for (auto seed : seeds) 
            case_1(size,seed);
}