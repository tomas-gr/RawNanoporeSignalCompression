from pgnano.stats_analysis.jupyter_data_preparation import flatten_sample_data
from typing import Callable, List, Iterable, Tuple
from functools import partial
from scipy.stats import geom
from scipy.linalg import lstsq
import numpy as np
import numpy.typing as npt
import matplotlib.pyplot as plt

def transform_signal_to_error(signal: npt.NDArray[np.int16]) -> npt.NDArray[np.int16]:
    prediction = np.append([0], np.copy(signal[:-1]))
    assert(len(prediction) == len(signal))
    return signal - prediction

def contextualise_signal(signal: npt.NDArray[np.int16]) -> npt.NDArray[np.int16]:
    previous_value = np.append([0], np.copy(signal[:-1]))
    previous_previous_value = np.append([0], np.copy(previous_value[:-1]))
    f = np.vectorize(lambda x: 0 if x >= 0 else 1)
    return f(previous_value - previous_previous_value) 

def transform_error_to_code(error: npt.NDArray[np.int16]) -> npt.NDArray[np.uint16]:
    vectorized = np.vectorize(lambda x: 2*x - 1 if x > 0 else (-(2*x)))
    return vectorized(error).astype(np.uint16)

def get_splitted_histograms(signal: npt.NDArray[np.int16], bit: np.integer) -> Tuple[np.ndarray]:
    low_mask: np.int16 = (np.uint16(1) << bit) - 1
    high_mask: np.int16 = (np.uint16(0xFFFF) ^ low_mask)

    f_low = np.vectorize(lambda x: x & low_mask)
    f_high = np.vectorize(lambda x: (x & high_mask) >> bit)

    low_signal = f_low(signal)
    high_signal = f_high(signal)

    return np.bincount(low_signal, minlength=(np.uint16(1) << bit)), np.bincount(high_signal, minlength=(np.uint16(1) << (16 - bit)))

def get_contextualised_histograms(signal: npt.NDArray[np.int16], bit: np.integer) -> List[Tuple[np.ndarray]]:
    context_classes = contextualise_signal(signal)
    class_0 = signal[np.vectorize(lambda x: x == 0)(context_classes)]
    class_1 = signal[np.vectorize(lambda x: x == 1)(context_classes)]
    low_0, high_0 = get_splitted_histograms(class_0, bit)
    low_1, high_1 = get_splitted_histograms(class_1, bit)
    return [(low_0, high_0), (low_1, high_1)]


def plot_histograms(low, high, bit):
    fig, axs = plt.subplots(1, 2, figsize=(10, 5))
    axs[0].plot(low)
    axs[0].set_title('Low')
    axs[1].plot(high)
    axs[1].set_title('High')

def estimate_geometric_param(xs: npt.NDArray[np.uint16]) -> np.floating:
    n = len(xs)
    s = sum(xs)
    return n / s