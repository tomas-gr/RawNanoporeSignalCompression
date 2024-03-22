import abc
import numpy as np
import numpy.typing as npt
from typing import Tuple, List, Union
from pgnano.stats_analysis.circular_array import CircularArray
from pgnano.stats_analysis.primitives import Histogram

class Model(abc.ABC):
    @abc.abstractmethod
    def __init__(self) -> None:
        super().__init__()

    @abc.abstractmethod
    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        pass

    def get_code_lenght(self, signal: npt.NDArray[np.int16]) -> np.unsignedinteger:
        res = 0
        for x in signal:
            res -= np.log2(self._get_symbol_prob(x))
        return res
    
    def build_histograms(self, signal: npt.NDArray[np.int16]) -> None:
        pass

    def get_histograms(self) -> list[Histogram]:
        return []

class ByteSplitModel(abc.ABC):
    def split_bytes(x: Union[np.int16, np.uint16]) -> Tuple[np.uint8, np.uint8]:
        return ((x & 0xFF00) >> 8, x & 0xFF)
    
class NibbleSplitModel(abc.ABC):
    def split_nibbles(x: Union[np.int16, np.uint16]) -> Tuple[np.uint8, np.uint8, np.uint8, np.uint8]:
        return ( (x & 0xF000) >> 12, (x & 0xF00) >> 8,(x & 0xF0) >> 4, x & 0xF)

class RiceMapModel(abc.ABC):
    def rice_map(x: np.int16) -> np.uint16:
        assert(abs(x) < (1 << 15))
        if x < 0:
            return 2*(-x) - 1
        else:
            return 2*x

class AbstractContextModel(Model, ByteSplitModel, RiceMapModel, abc.ABC):
    @abc.abstractmethod
    def __init__(self, 
                 high_histograms: Histogram, 
                 low_histograms: Histogram,
                 counts: npt.NDArray[np.uint64],
                 history: CircularArray) -> None:
        self.high_histograms = high_histograms
        self.low_histograms = low_histograms
        self.counts = counts
        self.history = history
        for idx, _  in enumerate(self.high_histograms):
            for i in range(256):
                self.high_histograms[idx][i] = 1
                self.low_histograms[idx][i] = 1

    @abc.abstractmethod
    def _predict(self) -> np.int16:
        pass

    @abc.abstractmethod
    def _calculate_context_class(self) -> np.uint64:
        pass

    @abc.abstractmethod
    def _update_history(self, symbol) -> None:
        pass

    @abc.abstractmethod
    def _update_counts(self, context_class) -> None:
        pass

    @abc.abstractmethod
    def _update_histograms(self, high, low, context_class) -> None:
        pass
    
    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        prediction = self._predict()
        high, low = ByteSplitModel.split_bytes(RiceMapModel.rice_map(symbol - prediction))
        context_class = self._calculate_context_class()
        p_high, p_low = self._calculate_probabilities(high, low, context_class)
        self._update_stats(symbol, high, low, context_class)
        return p_high * p_low
    
    def _update_stats(self, symbol, high, low, context_class):
        self._update_histograms(high, low, context_class)
        self._update_counts(context_class)
        self._update_history(symbol)
    
    def _calculate_probabilities(self, high, low, context_class):
        p_high = self.high_histograms[context_class][high] / self.counts[context_class]
        p_low = self.low_histograms[context_class][low] / self.counts[context_class]
        return p_high,p_low

class DefaultAbstractContextModel(AbstractContextModel):
    def _update_history(self, symbol):
        self.history.push(symbol)

    def _update_counts(self, context_class):
        self.counts[context_class] += 1

    def _update_histograms(self, high, low, context_class):
        self.high_histograms[context_class][high] += 1
        self.low_histograms[context_class][low] += 1