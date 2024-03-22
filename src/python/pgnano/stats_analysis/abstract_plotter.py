import numpy as np
import numpy.typing as npt
import matplotlib.pyplot as plt

from pgnano.stats_analysis.models import Model, SklearnModel

def abstract_plot(model: SklearnModel, 
                  signal: npt.NDArray[np.int16],
                  start: int = 1,
                  size: int = 250,
                  show_signal = True,
                  show_error = True) -> None:
    limit = start + size
    model._train(signal)
    signal_chunk = signal[start:limit]
    predicted_signal = []
    for x in signal_chunk:
        predicted_signal.append(model._predict_and_advance_history(x))
    if show_signal:
        plt.plot(signal_chunk)
        plt.plot(predicted_signal)
    if show_error:
        plt.plot(
            list(
                map(lambda x: abs(x[0] - x[1]), 
                    zip(signal_chunk, predicted_signal))
            )
        )