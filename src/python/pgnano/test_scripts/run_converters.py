import subprocess
from pgnano.constants.constants import copy_build_path, decompress_build_path, compress_build_path
import numpy as np

# FIXME: DO NOT BYPASS
# Bypass using pod5 API as it crashes on pgnano signal type
def get_file_id(filename: str):
     with open(filename, "rb") as f:
          header_approximation = f.read(1 << 10)
     idx = header_approximation.find(b"MINKNOW:file_identifier")
     if idx == -1:
          return None
     idx = idx + len("MINKNOW:file_identifier") # Skip over header description
     if idx % 8 != 0:
          idx = (idx // 8) * 8 + 8
     return header_approximation[idx:idx+36].decode()

class RunCopyResult:
     def __init__(self, exitcode, bytes_written, samples_processed) -> None:
          self.exitcode = exitcode
          self.bytes_written = bytes_written
          self.samples_processed = samples_processed

def run_copy(in_filename: str, out_filename: str, compression_option: str, capture_output) -> RunCopyResult:
     res = subprocess.run([copy_build_path, in_filename, out_filename, compression_option], capture_output=capture_output, text=True)
     if res.returncode == 0:
          bytes_written_str, sample_count_str = res.stdout.splitlines()[0].split(';', maxsplit=2)
          try:
               bytes_written = int(bytes_written_str)
               sample_count = int(sample_count_str)
               return RunCopyResult(0, bytes_written, sample_count)
          except ValueError as e:
               return RunCopyResult(0, None, None)
     else:
          return RunCopyResult(res.returncode, None, None)

def run_compress(in_filename: str, out_filename: str, bam_file: str, levels_file: str, capture_output) -> RunCopyResult:
     args = [compress_build_path, in_filename, out_filename]
     if bam_file is not None:
          args.append(bam_file)
     else:
          args.append("/")
     if levels_file is not None:
          args.append(levels_file)
     else:
          args.append("/")
     res = subprocess.run(args, capture_output=capture_output, text=True)
     if res.returncode == 0:
          bytes_written_str, sample_count_str = res.stdout.splitlines()[0].split(';', maxsplit=2)
          try:
               bytes_written = int(bytes_written_str)
               sample_count = int(sample_count_str)
               return RunCopyResult(0, bytes_written, sample_count)
          except ValueError as e:
               return RunCopyResult(0, None, None)
     else:
          return RunCopyResult(res.returncode, None, None)

def run_decompress(in_filename: str, out_filename: str, bam_file: str, levels_file: str, capture_output) -> RunCopyResult:
     args = [decompress_build_path, in_filename, out_filename]#, bam_file, levels_file]
     if bam_file is not None:
          args.append(bam_file)
     else:
          args.append("/")
     
     if levels_file is not None:
          args.append(levels_file)
     else:
          args.append("/")
     
     res = subprocess.run(args, capture_output=capture_output, text=True)
     if res.returncode == 0:
          bytes_written_str, sample_count_str = res.stdout.splitlines()[0].split(';', maxsplit=2)
          try:
               bytes_written = int(bytes_written_str)
               sample_count = int(sample_count_str)
               return RunCopyResult(0, bytes_written, sample_count)
          except ValueError as e:
               return RunCopyResult(0, None, None)
     else:
          return RunCopyResult(res.returncode, None, None)
