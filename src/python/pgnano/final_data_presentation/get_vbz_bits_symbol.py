import sys
import pod5
import pandas as pd
import numpy as np
from pgnano.data_obtention.dataset_retrieval import get_datasets_with_sequencing
import multiprocessing as mp
from tqdm import tqdm
import pod5.signal_tools

FLY_DS_NAME = 'Fly_DmNQs_Standford_University'
GIAB_10_4_1_DS_NAME = 'Human_GiaBAT_ONT'
GIAB_10_3_DS_NAME = 'Human_GGiab10_3_ONT'
GIAB_9_4_1_DS_NAME = 'Human_GGiab9_4_1_ONT'

def vbz_bits_symbol(file):
    total_samples = 0
    total_bytes = 0
    with pod5.Reader(file.path) as r:
        for read in r.reads():
            total_bytes += read.byte_count
            #signal = read.signal

            #total_bytes += sum(map(lambda x: len(x), pod5.signal_tools.vbz_compress_signal_chunked(signal, 1000)[0]))
            total_samples += read.num_samples
    return (float(total_bytes) * 8) / float(total_samples)

def vbz_bits_symbol_standalone(file):
    total_samples = 0
    total_bytes = 0
    with pod5.Reader(file.path) as r:
        for read in r.reads():
            total_samples += read.num_samples
            signal = read.signal
            total_bytes += len(pod5.vbz_compress_signal(signal))
    return (float(total_bytes) * 8) / float(total_samples)


if __name__ == "__main__":
    dataset_name = sys.argv[1]
    all_datasets = get_datasets_with_sequencing()
    if dataset_name == "all":
        filter_expression = lambda x: True
    if dataset_name == "10.4.1":
        filter_expression = lambda x: x.dataset == FLY_DS_NAME or x.dataset == GIAB_10_4_1_DS_NAME
    if dataset_name == "10.3":
        filter_expression = lambda x: x.dataset == GIAB_10_3_DS_NAME
    if dataset_name == "9.4.1":
        filter_expression = lambda x: x.dataset == GIAB_9_4_1_DS_NAME
    if dataset_name == "Fly":
        filter_expression = lambda x: x.dataset == FLY_DS_NAME
    if dataset_name == 'Human10.4.1':
        filter_expression = lambda x: x.dataset == GIAB_10_4_1_DS_NAME
    datasets = list(filter(filter_expression, all_datasets))

    with mp.Pool(min(mp.cpu_count(), len(datasets))) as p:
        res = list(tqdm(p.imap(
            vbz_bits_symbol,
            datasets
        ), total=len(datasets), desc="integrated"))
        res_standalone = list(tqdm(p.imap(
            vbz_bits_symbol_standalone,
            datasets
        ), total=len(datasets), desc="standalone"))
    
    print(f"Integrated: {np.mean(res)}")
    print(f"Standalone: {np.mean(res_standalone)}")