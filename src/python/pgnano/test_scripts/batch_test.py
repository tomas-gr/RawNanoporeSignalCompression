import argparse
import subprocess
import sys
import pgnano.constants.constants as consts
import pgnano.data_obtention.dataset_retrieval as pgdsretrieval
import os
import multiprocessing
import functools
import tqdm
import statistics
from pgnano.test_scripts.double_conversion import double_conversion, DoubleConversionResult, TestType
from pgnano.test_scripts.iteration_test import iteration_test
from pgnano.test_scripts.dummy_download_test import dummy_download_test
from pgnano.constants.constants import venv_interpreter_path, observational_equivalence_test_file
from collections import namedtuple
from itertools import repeat
from typing import Callable, List, Iterable

TestTuple = namedtuple('TestTuple', ['path', 'test_type', 'bam_path', 'level_path'])

def test(x: TestTuple):
    path = x.path
    file_stat_size = None
    try:
        if os.path.exists(path) and os.path.isfile(path):
            file_stat_size = os.stat(path).st_size
        else:
            raise Exception(f"Input path ({path}) does not exist or was deleted on runtime!")
    except OSError as e:
        raise e

    #print(iteration_test(5, path))
    double_conversion_result = double_conversion(test_type=x.test_type, 
                                                 bam_path=x.bam_path, 
                                                 level_path=x.level_path, 
                                                 is_process=False, 
                                                 param_in_file=path)
    success = double_conversion_result.success
    comp_time = double_conversion_result.compression_time
    decomp_time = double_conversion_result.decompression_time
    comparison_time = double_conversion_result.comparison_time
    compressed_size = double_conversion_result.compressed_size
    total_bits_written = double_conversion_result.compressed_bytes_count * 8 if success else None
    samples = double_conversion_result.total_sample_count

    #res = dummy_download_test(path)
    #print(path, res)
    #return res
    return (success, path, comp_time, decomp_time, comparison_time, file_stat_size, compressed_size, total_bits_written, samples)

def test_bed(sampler: Callable[[TestType], Iterable[pgdsretrieval.SequencedPOD5Info]],
             builder: Callable[[Iterable[pgdsretrieval.SequencedPOD5Info], TestType], Iterable[TestTuple]],
             test_type: TestType, cpu_count: int = os.cpu_count()) -> None:
    datasets = sampler()
    with multiprocessing.Pool(cpu_count) as p:
        res = list(tqdm.tqdm(p.imap(test, builder(datasets, test_type)), total=len(datasets)))
    return res

default_builder = lambda datasets, test_type: map(
    lambda x: TestTuple(x[0], x[1], x[2], x[3]), 
    zip(
        map(lambda x: x.path, datasets), 
        repeat(test_type),
        map(lambda x: x.bam_path, datasets),
        map(lambda x: x.level_path, datasets)))

def all_test(test_type: TestType, cpu_count: int = os.cpu_count()):
    sampler = pgdsretrieval.get_datasets_with_sequencing# if test_type == TestType.COPY_CONVERSION else pgdsretrieval.get_datasets_only_with_sequencing
    builder = default_builder
    return test_bed(sampler, builder, test_type, cpu_count)

def small_test(test_type: TestType, cpu_count: int = os.cpu_count()):
    def sampler():
        datasets_sample = (pgdsretrieval.get_datasets_with_sequencing)()# if test_type == TestType.COPY_CONVERSION else pgdsretrieval.get_datasets_only_with_sequencing)()
        datasets_sample.sort(key= lambda x: x.size)
        datasets_sample = datasets_sample[:cpu_count]
        return datasets_sample
    builder = default_builder
    return test_bed(sampler, builder, test_type, cpu_count)

def one_test(test_type: TestType, path: str = None):
    def sampler():
        ds = (pgdsretrieval.get_datasets_with_sequencing)()# if test_type == TestType.COPY_CONVERSION else pgdsretrieval.get_datasets_only_with_sequencing)()
        ds.sort(key=lambda x: x.size)
        if not path:
            test_case = ds[0].path
        else:
            test_case = path
        print(test_case.split("/")[-2:])
        return [ds[0]]
    builder = default_builder
    return test_bed(sampler, builder, test_type, 1)

#FIXME: USE PANDAS
def calculate_stats(ress: list):
    num_tests = len(ress)
    comp_times = map(lambda x: x[2], ress)
    decomp_times = map(lambda x: x[3], ress)
    comparison_times = map(lambda x: x[4], ress)
    sizes = list(map(lambda x: x[5], ress))
    compressed_sizes = list(map(lambda x: x[6], ress))
    bits_written = list(map(lambda x: x[7], ress))
    total_samples = list(map(lambda x: x[8], ress))
    compression_ratios = list(map(lambda x: x[0]/x[1], zip(bits_written, total_samples)))
    compression_ratio_macro_avg = statistics.mean(compression_ratios)
    compression_ratio_micro_avg = sum(bits_written) / sum(total_samples)

    closure = lambda x : [(size / (1 << 20)) / time for (time, size) in zip(x, sizes)]
    comp_MB_s = closure(comp_times)
    decomp_MB_s = closure(decomp_times)
    comparison_MB_s = closure(comparison_times)
    closure2 = lambda x: functools.reduce(lambda y, z: y + z, x, 0) / num_tests
    comp_MB_s_avg = closure2(comp_MB_s)
    decomp_MB_s_avg = closure2(decomp_MB_s)
    comparison_MB_s_avg = closure2(comparison_MB_s)
    external_comp_ratio_precalc = list(map(lambda x: (x[6] / x[5], x[5]),ress))
    external_comp_ratio_macro_avg = functools.reduce(lambda x, y: x + y[0], external_comp_ratio_precalc, 0) / num_tests
    external_comp_ratio_micro_avg = sum(compressed_sizes) / sum(sizes)
    #functools.reduce(lambda x, y: x + y[0]*y[1],
    #(lambda x_max : map(lambda x: (x[0], x[1] / x_max),comp_ratio_precalc))
    #(max(list(map(lambda x: x[1], comp_ratio_precalc)))),0)

    return (comp_MB_s_avg, decomp_MB_s_avg, comparison_MB_s_avg, external_comp_ratio_macro_avg, external_comp_ratio_micro_avg, compression_ratio_macro_avg, compression_ratio_micro_avg)


def print_all_test_resume(res):
    total_tests = len(res)
    x = list(filter(lambda x: not x[0], res))
    failed_tests = len(x)
    if failed_tests > 0:
        failed_paths = map(lambda y: y[1], x)
        for p in failed_paths:
            print(p)
        print(f"{failed_tests} of {total_tests} failed")
    else:
        print("All test successful")
        print("Statistics:")
        comp_throughput, decomp_throughput, comparison_throughput, size_ratio_pgnano_vbz_macro, size_ratio_pgnano_vbz_micro, macro_comp_ratio, micro_comp_ratio = calculate_stats(res)
        print(f"Average compression speed: {comp_throughput} MB/s")
        print(f"Average decompression: {decomp_throughput} MB/s")
        print(f"Average comparison: {comparison_throughput} MB/s")
        print(f"Macro average size ratio: {size_ratio_pgnano_vbz_macro}")
        print(f"Micro average size ratio: {size_ratio_pgnano_vbz_micro}")
        print(f"Macro average compression ratio: {macro_comp_ratio} bits/sample")
        print(f"Micro average compression ratio: {micro_comp_ratio} bits/sample")

def bam_test():
    c_program = "/data/pgnanoraw/pod5_fork/build/src/c++/copy"
    in_file = "/data/datananoraw/BAM_TEST/batch15.pod5"
    bam_file = "/data/datananoraw/BAM_TEST/calls2ref.bam"
    intermediate_file = "/data/pgnanoraw/pod5_fork/my_test_data/out.pod5"
    out_file = "/data/pgnanoraw/pod5_fork/my_test_data/out2.pod5"
    levels_file = "/data/pgnanoraw/remora/tests/data/levels.txt"
    first_c_params = f"{in_file} {intermediate_file} --pgnano {bam_file} {levels_file}"
    second_c_params = f"{intermediate_file}  {out_file} --VBZ {bam_file} {levels_file}"
    os.system(command=f"{c_program} {first_c_params}")
    os.system(command=f"{c_program} {second_c_params}")
    ret_code = subprocess.run([venv_interpreter_path,
                               observational_equivalence_test_file,
                               in_file,
                               out_file
                               ]).returncode
    if ret_code == 0:
        print("SUCCESS")
    else:
        print("FAILURE")
    os.remove(intermediate_file)
    os.remove(out_file)

#TODO: FIXME: Only works on linux
def tmp_cleanup():
    os.system(f"find {consts.data_root_path} -regex .*tmp.* | xargs rm 2> /dev/null")

def remove_intermediate_files():
    for x in map(lambda x: x.path, pgdsretrieval.get_datasets()):
        if not x.endswith(".pod5"):
            os.remove(x)

def cleanup():
    tmp_cleanup()
    remove_intermediate_files()

if __name__ == "__main__":
    cleanup()
    test_type = None
    if len(sys.argv) == 1:
        res = one_test(TestType.STANDALONE_CONVERSION)
    else:
        if sys.argv[1] == "copy":
            test_type = TestType.COPY_CONVERSION
        elif sys.argv[1] == "standalone":
            test_type = TestType.STANDALONE_CONVERSION
        else:
            raise Exception("Unrecognized test type")
        if len(sys.argv) > 2:
            if sys.argv[2] == "small":
                if len(sys.argv) > 3:
                    parsed_int = int(sys.argv[3])
                    res = small_test(test_type, parsed_int)
                else:
                    res = small_test(test_type)
            elif sys.argv[2] == "one":
                res = one_test(test_type)
            else:
                res = all_test(test_type)
        else:
            res = all_test(test_type)
#    print(pgdsretrieval.get_datasets()[4].path)
    #res = one_test("/data/pgnanoraw/pod5_fork/my_test_data/one_read.pod5")
    print_all_test_resume(res)
    cleanup()
    #reg_tests(True,False,False)