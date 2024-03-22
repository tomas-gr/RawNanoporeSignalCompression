import os
import statistics as st
from typing import List

import pod5

import pgnano.stats_analysis.primitives as pgnprim
from pgnano.constants.constants import stats_analysis_root_path

def get_data(type: pgnprim.PGPoreType) -> List[str]:
    if type == pgnprim.PGPoreType.P10_4_1:
        path = stats_analysis_root_path + "/10_4_1"
    elif type == pgnprim.PGPoreType.P10_3:
        path = stats_analysis_root_path + "/10_3"
    elif type == pgnprim.PGPoreType.P9_4_1:
        path = stats_analysis_root_path + "/9_4_1"
    else:
        raise Exception("Pore type not recognised")
    candidates = map(lambda x: path + "/" + x, os.listdir(path))
    #datapaths = list(filter(lambda x: os.path.isfile(x), candidates))
    datapaths = list(filter(lambda x: x.endswith(".pod5"), filter(lambda x: os.path.isfile(x), candidates)))
    print(datapaths)
    data = []
    for x in datapaths:
        data.append((x, os.stat(x).st_size))
    data.sort(key=lambda x: x[1])
    print(data)
    return data

### 10.4.1
def flatten_sample_data(pore_type: pgnprim.PGPoreType, sample_size=100):
    ds = get_data(pore_type)[1][0]
    signal_data = []
    chunked_data = []
    with pod5.Reader(ds) as r:
        for x in r.reads():
            signal_data.append(x.signal)
            chunked_data.extend(pgnprim.get_read_signal_in_chunks(x))
#    print("loaded reads")
#    print(len(signal_data))
#    avg_len = st.mean(map(len, signal_data))
#    print(avg_len)
#    median_len = st.median(map(len, signal_data))
#    print(median_len)

    if sample_size != None:
        signal_data = signal_data[:sample_size]
        chunked_data = chunked_data[:sample_size]

    return signal_data, chunked_data