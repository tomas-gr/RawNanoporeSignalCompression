import os
import subprocess
import sys
import random
import time
import numpy as np
from dataclasses import dataclass
from pgnano.constants.constants import build_path, venv_interpreter_path, observational_equivalence_test_file
from pgnano.test_scripts.run_converters import run_copy, run_compress, run_decompress
from enum import Enum

class TestType(Enum):
    COPY_CONVERSION = 1
    STANDALONE_CONVERSION = 2

@dataclass(frozen=True)
class DoubleConversionResult:
    success: bool
    compression_time: float
    decompression_time: float
    comparison_time: float
    compressed_size: int
    compressed_bytes_count: np.uint64
    total_sample_count: np.uint64

def double_conversion(test_type: TestType, bam_path: str | None, level_path: str | None, is_process: bool = False, param_in_file: str = "", capture_output = True):
    rnd = random.SystemRandom()
    compressed_size = None
    if is_process:
        in_file = sys.argv[1]
    else:
        in_file = param_in_file
    
    intermediate_file = in_file + str(rnd.randint(0, (1 << 15)))
    out_file = in_file + str(rnd.randint(0, (1 << 15)))
    time_pre_compress = time.time()
    if test_type == TestType.COPY_CONVERSION:
        fst_run_res = run_copy(in_file, intermediate_file, "--pgnano", capture_output)
    elif test_type == TestType.STANDALONE_CONVERSION:
        fst_run_res = run_compress(in_file, intermediate_file, bam_path, level_path, capture_output)
    time_post_compress = time.time()
    if fst_run_res.exitcode != 0:
        if os.path.isfile(intermediate_file):
            os.remove(intermediate_file)
        if is_process:
            exit(-1)
        else:
            return DoubleConversionResult(False,None,None,None,None,None,None)
    compressed_size = os.stat(intermediate_file).st_size
    if not compressed_size:
        raise RuntimeError("Couldn't get PGNano's compressed file!...\n aborting")
    time_pre_decompress = time.time()
    if test_type == TestType.COPY_CONVERSION:
        snd_exit_code = run_copy(intermediate_file, out_file, "--VBZ", capture_output).exitcode
    elif test_type == TestType.STANDALONE_CONVERSION:
        snd_exit_code = run_decompress(intermediate_file, out_file, bam_path, level_path, capture_output).exitcode
    time_post_decompress = time.time()
    if snd_exit_code != 0:
        os.remove(intermediate_file)
        if os.path.isfile(out_file):
            os.remove(out_file)
        if is_process:
            exit(-1)
        else:
            return DoubleConversionResult(False,None,None,None,None,None,None)
    
    time_pre_obs_equivalence = time.time()
    comparison_exit_code = subprocess.run([venv_interpreter_path, observational_equivalence_test_file, in_file, out_file], capture_output=capture_output).returncode
    time_post_obs_equivalence = time.time()

    os.remove(intermediate_file)
    os.remove(out_file)

    if is_process:
        exit(comparison_exit_code)
    else:
        return DoubleConversionResult(
                                        success=comparison_exit_code == 0,
                                        compression_time=time_post_compress - time_pre_compress,
                                        decompression_time=time_post_decompress - time_pre_decompress,
                                        comparison_time=time_post_obs_equivalence - time_pre_obs_equivalence,
                                        compressed_size=compressed_size,
                                        compressed_bytes_count=fst_run_res.bytes_written,
                                        total_sample_count=fst_run_res.samples_processed
                                     )

if __name__ == "__main__":
    double_conversion(True)