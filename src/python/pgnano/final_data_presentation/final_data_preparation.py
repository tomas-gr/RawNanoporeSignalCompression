import os
import statistics as st
from typing import List

import pod5

import pgnano.stats_analysis.primitives as pgnprim
from pgnano.constants.constants import stats_analysis_root_path

def final_get_data(type: pgnprim.PGPoreType) -> List[str]:
    if type == pgnprim.PGPoreType.P10_4_1:
        path = stats_analysis_root_path + "/10_4_1"
    elif type == pgnprim.PGPoreType.P10_3:
        path = stats_analysis_root_path + "/10_3"
    elif type == pgnprim.PGPoreType.P9_4_1:
        path = stats_analysis_root_path + "/9_4_1"
    else:
        raise Exception("Pore type not recognised")
    candidates = map(lambda x: path + "/" + x, os.listdir(path))
    datapaths = list(filter(lambda x: x.endswith(".pod5"), filter(lambda x: os.path.isfile(x), candidates)))
    return datapaths

def final_flattened_sample_data(pore_type: pgnprim.PGPoreType, sample_size=100):
    ds = final_get_data(pore_type)[1]
    i = 0
    signal_data = []
    chunked_data = []

    with pod5.Reader(ds) as r:
        if sample_size is not None:
            reads = r.reads()
            while i < sample_size:
                x = next(reads)
                signal_data.append(x.signal)
                chunked_data.extend(pgnprim.get_read_signal_in_chunks(x))
                i += 1
        else:
            for x in r.reads():
                signal_data.append(x.signal)
                chunked_data.extend(pgnprim.get_read_signal_in_chunks(x))

    return signal_data, chunked_data