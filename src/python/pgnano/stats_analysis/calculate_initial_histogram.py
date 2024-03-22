from typing import List, Iterator
from pgnano.stats_analysis.primitives import Histogram
from pgnano.stats_analysis.models import CppModel
from pgnano.data_obtention.dataset_retrieval import get_datasets
import os
import multiprocessing as mp
import numpy as np
import numpy.typing as npt
import pod5
import pickle
import tqdm.auto as tqdm
import random
import functools
import matplotlib.pyplot as plt

signal = npt.NDArray[np.int16]

def load_file_signals(file: str) -> List[signal]:
    signals = []
    with pod5.Reader(file) as r:
        for x in r.reads():
            signals.append(x.signal)
    return signals

def load_all_signals(files: Iterator[str]) -> List[signal]:
    with mp.Pool(os.cpu_count()) as p:
        files_signals = p.map(load_file_signals, files)
    res = []
    for file_signals in files_signals:
        res.extend(file_signals)
    return res

def retrieve_model_histogram(signal):
    model = CppModel()
    model.reset_histograms()
    model.build_histograms(signal)
    return model.get_histograms()

def build_histograms(signals: List[signal]) -> List[List[Histogram]]:    
    with mp.Pool(os.cpu_count()) as p:
        #, chunksize=int(len(signals)/(1000*os.cpu_count()))
        return list(tqdm.tqdm(p.imap(retrieve_model_histogram, signals), total=len(signals)))
#        res = list(tqdm.tqdm(p.imap(test, map(lambda x: x.path, datasets)), total=len(datasets)))
def build_global_histograms(histograms: List[List[Histogram]]) -> List[Histogram]:
    res = []
    for _ in range(len(histograms[0])):
        res.append(Histogram(len(histograms[0][_].get_indexes()) - 1))
    for histogram_list in histograms:
        for idx, histogram in enumerate(histogram_list):
            res[idx] = histogram + res[idx]
    return res

def clipped_shift(x: np.number, min_bits = 1):
    bits_used = np.floor(np.log2(x)) + 1
    if bits_used <= min_bits:
        return np.uint64(x)
    else:
        return np.uint64(x) >> np.uint64(1)

def process_low_histogram(histogram: npt.NDArray[np.number]) -> npt.NDArray[np.uint16]:
    return np.floor((histogram * (2**12)) / histogram.max())

def process_high_histogram(histogram: npt.NDArray[np.number]) -> npt.NDArray[np.uint16]:
    min_bits = 1
    max_bits = 12
    laplace = np.vectorize(lambda x: max(x, 1))
    bits_used = lambda x: np.floor(np.log2(x)) + 1
    clipped_bit_shift = np.vectorize(lambda x: clipped_shift(x, min_bits)) 

    histogram = laplace(histogram)
    histogram = histogram.astype(dtype=np.uint64)
    while np.round(bits_used(np.max(histogram))) > max_bits:
        histogram = clipped_bit_shift(histogram)
    return histogram

if __name__ == "__main__":
    high_byte_dump_path = "high_byte.json"
    low_byte_dump_path = "low_byte.json"
    this_seed = 20
    target_number_files = 1000
    rnd = random.Random(this_seed)

    #if (not os.path.exists(high_byte_dump_path)) or (not os.path.exists(low_byte_dump_path)):
    #    print("Retrieving datasets")
    #    dss = get_datasets()
    #    target_number_files = len(dss)
    #    sampled_dss = rnd.sample(dss,target_number_files)
    #    print(f"Total dataset size: {functools.reduce(lambda x, y: x + y, map(lambda x: x.size, sampled_dss), 0) / (1 << 20)} MB")
    #    print(len(sampled_dss))
    #    files = list(map(lambda x: x.path, sampled_dss))
    #    print("Loading signals")
    #    signals = load_all_signals(files)
    #    print("Building histograms")
    #    _ = build_histograms(signals)
    #    print("Calculating global histogram")
    #    histograms = build_global_histograms(_)
    #    high_byte = histograms[0]
    #    low_byte = histograms[1]
    #    with open(high_byte_dump_path, 'wb') as f:
    #        pickle.dump(high_byte.get_data(), f)
    #    with open(low_byte_dump_path, 'wb') as f:
    #        pickle.dump(low_byte.get_data(), f)
    with open(high_byte_dump_path, 'rb') as h, open(low_byte_dump_path, 'rb') as l:
        high_byte = pickle.load(h)
        low_byte = pickle.load(l)
    print(process_high_histogram(high_byte))
    #plt.plot(high_byte)
    #plt.savefig("high_byte.png")
    #plt.close()
    #plt.plot(low_byte)
    #plt.savefig("low_byte.png")
    #plt.close()