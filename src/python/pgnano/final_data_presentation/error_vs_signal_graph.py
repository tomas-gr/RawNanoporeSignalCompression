from pgnano.final_data_presentation.final_data_preparation import *
from pgnano.final_data_presentation.shared import *
from pgnano.stats_analysis.coding_analysis_scripts import transform_signal_to_error
from pgnano.stats_analysis.primitives import PGPoreType

import matplotlib.pyplot as plt
import numpy as np

start = 200
limit = 500

signal_data, chunked_data = final_flattened_sample_data(PGPoreType.P10_4_1)
error = transform_signal_to_error(signal_data[0])

figs, axs = plt.subplots(2,1,figsize=(10, 10))
axs[0].plot(signal_data[0][start:limit])
axs[1].plot(error[start:limit])
axs[0].set_ylabel('DACs')
axs[1].set_ylabel('DACs')
axs[0].set_xlabel('t')
axs[1].set_xlabel('t')
axs[0].set_title('Señal')
axs[1].set_title('Error de predicción')
plt.savefig(OUT_PATH + "error-vs-signal.png")