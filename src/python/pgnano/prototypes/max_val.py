from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data, get_data
from pgnano.stats_analysis.primitives import PGPoreType
import pod5

pores = [PGPoreType.P10_4_1, PGPoreType.P10_3, PGPoreType.P9_4_1]
current_max = -9999999999999
current_min = 9999999999999

for pore in pores:
    dss = map(lambda x: x[0], get_data(pore))
    for ds in dss: 
        signal_data = []
        with pod5.Reader(ds) as r:
            for x in r.reads():
                signal_data.append(x.signal)
                
        for one_signal in signal_data:
            this_max = max(one_signal)
            this_min = min(one_signal)
            if this_max > current_max:
                current_max = this_max
            if this_min < current_min:
                current_min = this_min
print(current_min, current_max)