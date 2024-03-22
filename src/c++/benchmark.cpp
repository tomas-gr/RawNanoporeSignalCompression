#include <iostream>
#include <memory>
#include <vector>
#include <stdint.h>
#include <assert.h>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <sstream>

#include "pod5_format/c_api.h"
#include "pod5_format/pgnano/compressor.h"

#include "pod5_format/pgnano/pgnano.cpp"

#include "ic.h"
#include "try_golombs.cpp"

const std::string binary_chunks_file_path = std::string("/data/pinanoraw/rtorrado/Git_Pinanoraw/pod5_nanoraw/benchmark_file.bin");

typedef struct {
    uint32_t samples;
    std::unique_ptr<int16_t[]> data;
} SignalInfo;

std::unique_ptr<std::vector<SignalInfo>> pull_samples_from_binary()
{
    std::unique_ptr<std::vector<SignalInfo>> res = std::make_unique<std::vector<SignalInfo>>(); 
    auto source = fopen(binary_chunks_file_path.data(), "rb");
    uint32_t samples;
    size_t read_bytes;
    read_bytes = fread(&samples, sizeof(uint32_t), 1, source);
    while (read_bytes > 0)
    {   
        assert(read_bytes == 1);
        SignalInfo x;
        x.samples = samples;
        x.data = nullptr;
        std::unique_ptr<int16_t[]> data = std::make_unique<int16_t[]>(samples);
        assert(data != nullptr);
        read_bytes = fread(data.get(), sizeof(int16_t), samples, source);
        assert(read_bytes == samples);
        x.data.swap(data);
        assert(data == nullptr);
        res->push_back(std::move(x));
        read_bytes = fread(&samples, sizeof(uint32_t), 1, source);
    }
    return res;
}

SignalInfo transform_signal_to_differences(const SignalInfo &  in)
{
    SignalInfo res = {in.samples, nullptr};
    std::unique_ptr<int16_t[]> transformed_array = std::make_unique<int16_t[]>(in.samples);
    int16_t *current_in_ptr, *end_ptr, *current_out_ptr;
    int16_t previous_value = 0;
    current_in_ptr = in.data.get();
    end_ptr = current_in_ptr + in.samples;
    current_out_ptr = transformed_array.get();
    for (; current_in_ptr != end_ptr; current_in_ptr++, current_out_ptr++)
    {
        *current_out_ptr = *current_in_ptr - previous_value;
        previous_value = *current_in_ptr;
    }
    res.data.swap(transformed_array);
    return res;
}

SignalInfo transform_signal_to_code(const SignalInfo &  in)
{
    SignalInfo res;
    res.samples = in.samples;
    res.data = nullptr;
    std::unique_ptr<int16_t[]> new_data = std::make_unique<int16_t[]>(in.samples);
    int16_t *begin_ptr, *end_ptr, *out_ptr;
    begin_ptr = in.data.get();
    end_ptr = begin_ptr + in.samples;
    out_ptr = new_data.get();
    int16_t previous_sample = 0;
    for (;begin_ptr != end_ptr; begin_ptr++, out_ptr++)
    {
        auto sample = *begin_ptr;
        int16_t error = sample - previous_sample;
        previous_sample = sample;
        *out_ptr = (error > 0 ? (error << 1) - 1 : (-error) << 1);
    }
    res.data.swap(new_data);
    return res;
}

typedef struct {
    uint32_t processed_samples;
    size_t bytes_written;
    std::chrono::duration<double> elapsed_time;
} BenchmarkBasicResult;

typedef struct {
    double bits_per_symbol;
    double mbs_per_second;
} BenchmarkResult;

typedef struct
{
    double bits_per_symbol;
    double mbs_per_second;
    std::string name;
} NamedBenchmarkResult;

typedef struct
{
    uint32_t samples_processed;
    std::chrono::duration<double> elapsed_time;
} TransformationBenchmark;

typedef BenchmarkBasicResult (*RawCompressionBenchmark)(const SignalInfo &, uint8_t);

struct DriverInfo
{
public:
    RawCompressionBenchmark f;
    std::string name;
    bool must_transform;
    uint8_t bit;

    DriverInfo(RawCompressionBenchmark f, std::string name, bool must_transform, uint8_t bit)
    {
        this->f = f;
        this->name = name;
        this->must_transform = must_transform;
        this->bit = bit;
    }
};

BenchmarkBasicResult benchmark_one_vbz(const SignalInfo & signal_info, uint8_t)
{
    BenchmarkBasicResult res;
    
    size_t bytes_written = signal_info.samples * 4;
    pod5_error_t error;
    
    char * out_buffer = new char[signal_info.samples * 4];
    assert(out_buffer != nullptr);
    
    auto t1 = std::chrono::steady_clock::now();
    error = pod5_vbz_compress_signal(signal_info.data.get(), signal_info.samples, out_buffer, &bytes_written);
    auto t2 = std::chrono::steady_clock::now();
    
    delete out_buffer;
    
    if (error != POD5_OK)
    {
        std::cout << pod5_get_error_string() << "\n";
    }
    assert(error == POD5_OK);
    
    res.bytes_written = bytes_written;
    res.processed_samples = signal_info.samples;
    res.elapsed_time = t2 - t1;
    
    return res;
}

BenchmarkBasicResult benchmark_one_pinanoraw(const SignalInfo & signal_info, uint8_t)       // NUEVO --> PINANORAW
{
    BenchmarkBasicResult res;
    
    size_t bytes_written = signal_info.samples * 4;
    pod5_error_t error;
    
    char * out_buffer = new char[signal_info.samples * 4];
    assert(out_buffer != nullptr);
    
    auto t1 = std::chrono::steady_clock::now();

    error = pod5_pinanoraw_compress_signal(signal_info.data.get(), signal_info.samples, out_buffer, &bytes_written);
            
    auto t2 = std::chrono::steady_clock::now();
    
    delete out_buffer;
    
    if (error != POD5_OK)
    {
        std::cout << pod5_get_error_string() << "\n";
    }
    assert(error == POD5_OK);
    
    res.bytes_written = bytes_written;
    res.processed_samples = signal_info.samples;
    res.elapsed_time = t2 - t1;
    
    return res;
}

BenchmarkBasicResult benchmark_one_pgnano(const SignalInfo & signal_info, uint8_t)
{
    BenchmarkBasicResult res;
    
    size_t bytes_written = signal_info.samples * 2;
    
    uint8_t * out_buffer = new uint8_t[signal_info.samples * 2];
    assert(out_buffer != nullptr);
    
    pgnano::Compressor compressor;
    pgnano::PGNanoWriterState writer_state;
    
    writer_state.m_pore_type_server = std::make_unique<pgnano::PoreTypeServer>();
    writer_state.m_pore_type_server->put_pore_type(0,"R10.4.1");
    
    pod5::ReadData read_data;
    read_data.pore_type = 0;
    
    auto t1 = std::chrono::steady_clock::now();
    bytes_written = compressor.compress(read_data, signal_info.samples, signal_info.data.get(), out_buffer, writer_state);
    auto t2 = std::chrono::steady_clock::now();
    
    delete out_buffer;
    
    res.bytes_written = bytes_written;
    res.processed_samples = signal_info.samples;
    res.elapsed_time = t2 - t1;
    
    return res;
}

void run_transformation_benchmarks(const std::vector<SignalInfo> & samples)
{
    std::vector<TransformationBenchmark> transformation_benchmarks;
    for (auto & sample : samples)
    {
        const auto & t1 = std::chrono::steady_clock::now();
        auto _ = transform_signal_to_differences(sample);
        const auto & t2 = std::chrono::steady_clock::now();
        transformation_benchmarks.push_back({sample.samples, t2 - t1});
    }
    std::vector<double> transformation_speeds;
    for (const auto & x : transformation_benchmarks)
    {
        transformation_speeds.push_back((double(x.samples_processed * sizeof(int16_t)) / x.elapsed_time.count()) / (1 << 20));
    }
    double cummulative_speed = std::accumulate(transformation_speeds.begin(), transformation_speeds.end(), double(0));
    std::cout << "Naive difference benchmark operated at speed: " << cummulative_speed / transformation_speeds.size() << "MB/s\n";
}

void run_max_bit_benchmarks(const std::vector<SignalInfo> & samples)
{
    std::vector<double> max_bit_benchmark;
    for (auto & sample : samples)
    {
        auto t1 = std::chrono::steady_clock::now();
        bitz16(reinterpret_cast<uint16_t*>(sample.data.get()),sample.samples,nullptr, 0);
        auto t2 = std::chrono::steady_clock::now();
        max_bit_benchmark.push_back((double(sample.samples * sizeof(int16_t)) / (t2-t1).count()) / (1 << 20));
    }
    double cummulative_speed = std::accumulate(max_bit_benchmark.begin(), max_bit_benchmark.end(), double(0));
    std::cout << "Max bit benchmark operated at speed: " << cummulative_speed / max_bit_benchmark.size() << "MB/s\n";
}

BenchmarkBasicResult benchmark_with_transform(const RawCompressionBenchmark & f, const SignalInfo & signal_info, uint8_t bit)
{
    auto t1 = std::chrono::steady_clock::now();
    const SignalInfo & new_signal_info = transform_signal_to_code(signal_info);
    auto t2 = std::chrono::steady_clock::now();
    auto transformation_duration = t2 - t1;
    auto benchmark_result = f(new_signal_info, bit);
    benchmark_result.elapsed_time += transformation_duration;
    return benchmark_result;
}

BenchmarkResult process_benchmarks(const std::vector<BenchmarkBasicResult> & results, FILE *fp)
{
    std::vector<double> bits_per_symbol, mbs_per_second;
    
    bits_per_symbol.reserve(results.size());
    
    mbs_per_second.reserve(results.size());
    
    for (const auto & result : results)
    {
        double bps_div = double(result.bytes_written * 8) / double(result.processed_samples);
        bits_per_symbol.push_back(bps_div);
        
        double mbs_div = ((result.processed_samples * sizeof(int16_t)) / result.elapsed_time.count()) / (1 << 20);
        mbs_per_second.push_back(mbs_div);

        fprintf(fp, "%f \t\t", bps_div);        
        fprintf(fp, "%f \n", mbs_div);
    }
    
    // SE CALCULA EL PROMEDIO DE LOS VALORES //
    BenchmarkResult res;
    res.bits_per_symbol = std::accumulate(bits_per_symbol.begin(), bits_per_symbol.end(), double(0)) / bits_per_symbol.size();
    res.mbs_per_second = std::accumulate(mbs_per_second.begin(), mbs_per_second.end(), double(0)) / mbs_per_second.size();
    
    return res;
}

void print_benchmark_results(const NamedBenchmarkResult & result)
{
    std::cout << result.name << ": " << result.bits_per_symbol << " bits/symbol at speed: " << result.mbs_per_second << "MB/s\n";
}

void run_compression_benchmarks(const std::vector<SignalInfo> & samples, 
                                const std::vector<DriverInfo> drivers)
{
    const auto total_samples = samples.size();
    std::cout << "Cantidad de Samples: " << total_samples << "\n \n";

    std::vector<BenchmarkBasicResult> raw_benchmark_results;
    
    raw_benchmark_results.reserve(total_samples);
    
    std::vector<NamedBenchmarkResult> results;

    FILE *fp;
    fp = fopen("../Output.txt", "w");   // IMPRIMO RESULTADOS SOBRE UN ARCHIVO

    for (const auto & driver_info : drivers)        // Recorre todas las funciones benchmark //
    {
        raw_benchmark_results.clear();
        
        const auto & driver = driver_info.f;
        const auto & name = driver_info.name;
        const auto & bit = driver_info.bit;     // No se usa en las funciones benchmark actuales //
        
        fprintf(fp, "# ------------------------ Benchmark ------------------------ # \n\n"); 
        fprintf(fp, "BPS \t\t\t");        
        fprintf(fp, "MBS");         
        fprintf(fp, "\n\n");
        
        std::cout << "Procesando: Benchmark " + name;
        std::cout << "\n";

        for (const auto & sample : samples)         // Recorre todas las muestras //
        {
            if (!driver_info.must_transform)
                raw_benchmark_results.push_back(driver(sample, bit));   // Llama a la función benchmark
            else {
                raw_benchmark_results.push_back(benchmark_with_transform(driver, sample, bit));
                std::cout << "BENCHMARK TRANSFORM \n";
            }
        }
        
        auto benchmark_results = process_benchmarks(raw_benchmark_results, fp); // IMPRIMIR ADENTRO DE ESTA FUNCIÓN //
        
        NamedBenchmarkResult named_results;
        
        named_results.bits_per_symbol = benchmark_results.bits_per_symbol;
        named_results.mbs_per_second = benchmark_results.mbs_per_second;
        named_results.name = name;
        
        results.push_back(named_results);
    }

    fclose(fp);

    std::sort(results.begin(), results.end(), [](const NamedBenchmarkResult & a, const NamedBenchmarkResult & b) -> bool {return a.bits_per_symbol < b.bits_per_symbol;});
    size_t max_name_lenght = 0;
    for (const auto & x : results)
    {
        if (x.name.size() > max_name_lenght)
        {
            max_name_lenght = x.name.size();
        }
    }

    std::vector<NamedBenchmarkResult> new_results;
    for (const auto & x : results)
    {
        size_t pad_len = max_name_lenght - x.name.size();
        std::string pad_str = "";
        for (size_t _ = 0; _ < pad_len; _++)
        {
            pad_str = pad_str + " ";
        }
        std::string new_name = x.name + pad_str;
        NamedBenchmarkResult new_result;
        new_result.bits_per_symbol = x.bits_per_symbol;
        new_result.mbs_per_second = x.mbs_per_second;
        new_result.name = new_name;
        new_results.push_back(new_result);
    }
    std::cout << "\n";
    
    for (const auto & named_result : new_results)
    {   
        print_benchmark_results(named_result);
    }    
 
    std::cout << "\n";
}

int main()
{
    const auto samples = pull_samples_from_binary();
    std::vector<DriverInfo> drivers;

    std::cout << "------------------------------------------------------------------\n";
    std::cout << "------------------- Transformation Benchmarks --------------------\n";
    std::cout << "------------------------------------------------------------------\n";

    run_transformation_benchmarks(*samples);
    run_max_bit_benchmarks(*samples);


    std::cout << "------------------------------------------------------------------\n";
    std::cout << "--------------------- Compression Benchmarks ---------------------\n";
    std::cout << "------------------------------------------------------------------\n";

    // Se inicializa cada driver //
    drivers.push_back(DriverInfo(benchmark_one_vbz, "Vbz", false, 5));
    drivers.push_back(DriverInfo(benchmark_one_pgnano, "PGNano", false, 5));
    drivers.push_back(DriverInfo(benchmark_one_pinanoraw, "Pinanoraw", false, 5));
    
    run_compression_benchmarks(*samples, drivers);

}