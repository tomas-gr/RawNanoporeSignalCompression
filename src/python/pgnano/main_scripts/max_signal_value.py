import numpy as np
import multiprocessing
import os
import pod5
import pgnano.data_obtention.dataset_retrieval as pgdsretrieval

ds = map(lambda x: x.path, pgdsretrieval.get_datasets())

def local_optima(path: str):
    max_val = -(2**17)
    min_val = (2**17)
    try:
        with pod5.Reader(path) as reader:
            for r in reader.reads():
                this_max = np.max(r.signal)
                this_min = np.min(r.signal)
                if this_max > max_val:
                    max_val = this_max
                if this_min < min_val:
                    min_val = this_min
    except:
        pass
    return min_val, max_val

with multiprocessing.Pool(os.cpu_count()) as pool:
    optima = pool.map(local_optima,ds)
print(optima)
max_val = -(2**17)
min_val = (2**17)

for (this_min, this_max) in optima:
    if max_val < this_max:
        max_val = this_max
    if min_val > this_min:
        min_val = this_min

print(max_val)
print(min_val)