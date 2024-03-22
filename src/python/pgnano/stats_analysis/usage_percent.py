import os
import pod5
import functools
import pgnano.data_obtention.dataset_retrieval as pgdsretrieval
from dataclasses import dataclass
from typing import List

@dataclass()
class UsagePercentSummary:
    total_bytes: int
    uncompressed_signal_bytes: int
    compressed_signal_bytes: int
    rows: int
    
    def __post_init__(self):
        self.total_signal_table_bytes: int = self.compressed_signal_bytes + 4 * self.rows + 16 * self.rows
        self.samples_percentage: float = self.compressed_signal_bytes / self.total_bytes
        self.signal_table_percentage: float = self.total_signal_table_bytes / self.total_bytes

    def __add__(self, other: 'UsagePercentSummary'):
        return UsagePercentSummary(
                                    total_bytes=self.total_bytes+other.total_bytes,
                                    uncompressed_signal_bytes=self.uncompressed_signal_bytes+other.uncompressed_signal_bytes,
                                    compressed_signal_bytes=self.compressed_signal_bytes+other.compressed_signal_bytes,
                                    rows=self.rows+other.rows
                                  )

    def __str__(self) -> str:
        return f'''Total bytes: {self.total_bytes}
Uncompressed signal bytes: {self.uncompressed_signal_bytes}
Compressed signal bytes: {self.compressed_signal_bytes}
Total signal table bytes: {self.total_signal_table_bytes}
Samples usage percentage: {self.samples_percentage * 100}%
Signal table usage percentage: {self.signal_table_percentage * 100}%'''

def get_file_percent_summary(file):
    rows = 0
    samples = 0
    compressed_bytes = 0
    with pod5.Reader(file) as r:
        for read in r.reads():
            compressed_bytes += read.byte_count
            samples += read.num_samples
            rows += len(read.signal_rows)
    total_bytes = os.stat(file).st_size
    return UsagePercentSummary(
                                total_bytes=total_bytes,
                                uncompressed_signal_bytes=samples*2,
                                compressed_signal_bytes=compressed_bytes,
                                rows=rows
                              )

def get_dataset_percent_summary(files: List[str]):
    summaries = []
    for file in filter(lambda x: x.endswith(".pod5"), files):
        summaries.append(get_file_percent_summary(file))
    final_summary = functools.reduce(lambda x,y: x + y, summaries)
    return final_summary

if __name__ == "__main__":
    print(get_dataset_percent_summary(map(lambda x: x.path, pgdsretrieval.get_datasets())))