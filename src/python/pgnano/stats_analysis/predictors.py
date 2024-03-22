import abc
import numpy as np

class Predictor(abc.ABC):
    @abc.abstractmethod
    def __init__(self, params):
        pass

    @abc.abstractmethod
    def predict(self, correct_previous_sample: np.int16) -> np.int16:
        pass