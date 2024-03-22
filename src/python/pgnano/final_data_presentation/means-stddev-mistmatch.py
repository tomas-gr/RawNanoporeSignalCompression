from matplotlib import pyplot as plt
from numpy import mean
from pgnano.stats_analysis.coding_analysis_scripts import transform_error_to_code, transform_signal_to_error
from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data
from pgnano.stats_analysis.primitives import PGPoreType
from itertools import repeat

signal_data, chunk_data = flatten_sample_data(PGPoreType.P10_4_1,100)
one_signal = signal_data[0]

start = 100
number = 500
limit = start + number

epsilon = 30
center = 870

fig = plt.figure(figsize=(16,8))

segment_1 = one_signal[start+122:start+183]
segment_2 = one_signal[start+212:start+250]

plt.plot(one_signal[start:limit])
plt.plot([i for i in range(122, 183)],list(repeat(mean(segment_2), 183-122)))
plt.plot([i for i in range(212, 250)],list(repeat(mean(segment_2), 250-212)))
plt.title('Señales con comportamiento distinto pero medias similares')
plt.xlabel('t')
plt.ylabel('DACs')

print(abs(mean(segment_1) - mean(segment_2)))
plt.savefig("/data/pgnanoraw/pod5_fork/final_data/means-stddev-mismatch.png")