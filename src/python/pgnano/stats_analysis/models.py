from dataclasses import dataclass
from typing import List
import numpy as np
import numpy.typing as npt
import itertools
from pgnano.stats_analysis.circular_array import CircularArray
import pod5
from pgnano.stats_analysis.predictors import Predictor
from pgnano.stats_analysis.primitives import Histogram
from pgnano.stats_analysis.base_models import Model, ByteSplitModel, RiceMapModel, NibbleSplitModel, DefaultAbstractContextModel
from sklearn.base import RegressorMixin

class VbzModel(Model):
    def __init__(self) -> None:
        pass

    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        pass

    def get_code_lenght(self, signal: npt.NDArray[np.int16]) -> np.unsignedinteger:
        return pod5.vbz_compress_signal(signal).nbytes * 8

class CppModel(Model, ByteSplitModel, RiceMapModel):
    def __init__(self) -> None:
        self.previous_symbol: np.uint16 = 0
        self.hgl = Histogram(255)
        self.hgh = Histogram(255)
        for i in range(0, 256):
            self.hgl[i] += 1
            self.hgh[i] += 1
        self.countsl = 256
        self.countsh = 256

    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        error = symbol - self.previous_symbol
        mapped_error = CppModel.rice_map(error)
        self.previous_symbol = symbol
        high, low = CppModel.split_bytes(mapped_error)
        h_prob = self.hgh[high] / self.countsh
        assert(h_prob < 1)
        l_prob = self.hgl[low] / self.countsl
        assert(l_prob < 1)
        self.hgl[low] += 1
        self.hgh[high] += 1
        self.countsl += 1
        self.countsh += 1
        return h_prob * l_prob

    # FIXME: refactor
    def reset_histograms(self) -> None:
        self.countsh = 0
        self.countsl = 0
        self.hgh = Histogram(initial_array=np.zeros(256, dtype=np.uint64))
        self.hgl = Histogram(initial_array=np.zeros(256, dtype=np.uint64))

    # FIXME: refactor
    def build_histograms(self, signal: npt.NDArray[np.int16]) -> None:
        for x in signal:
            error = x - self.previous_symbol
            mapped_error = CppModel.rice_map(error)
            self.previous_symbol = x
            high, low = CppModel.split_bytes(mapped_error)
            self.hgl[low] += 1
            self.hgh[high] += 1
            self.countsl += 1
        self.countsh = self.countsl

    def get_histograms(self) -> list[Histogram]:
        return [self.hgh, self.hgl]

class NibbleSimpleErrorModel(Model, NibbleSplitModel, RiceMapModel):
    def __init__(self) -> None:
        self.previous_symbol: np.uint16 = 0
        self.hgl = Histogram(15)
        self.hgm = Histogram(15)
        self.hgh = Histogram(15)
        for i in range(0, 16):
            self.hgl[i] += 1
            self.hgm[i] += 1
            self.hgh[i] += 1
        self.count = 16

    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        error = symbol - self.previous_symbol
        mapped_error = RiceMapModel.rice_map(error)
        self.previous_symbol = symbol
        _, high, med, low = NibbleSplitModel.split_nibbles(mapped_error)
        h_prob = self.hgh[high] / self.count
        assert(h_prob < 1)
        m_prob = self.hgm[med] / self.count
        assert(m_prob < 1)
        l_prob = self.hgl[low] / self.count
        assert(l_prob < 1)
        self.hgl[low] += 1
        self.hgm[med] += 1
        self.hgh[high] += 1
        self.count += 1
        return h_prob * l_prob * m_prob

# FIXME: as builder
class PredictorModel(Model, RiceMapModel):
    def __init__(self) -> None:
        super().__init__()
        self.hg = Histogram((1 << 16) - 1)
        self.count = (1 << 16)

    def _predict(self, x: np.int16) -> np.int16:
        return self.predictor.predict(x)

    def set_predictor(self, predictor: Predictor):
        self.predictor = predictor
    
    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        prediction = self._predict(symbol)
        error = PredictorModel.rice_map(symbol - prediction)
        p = self.hg[error] / self.count
        self.hg[error] += 1
        self.count += 1
        return p

@dataclass
class StabilityHistory:
    is_stable: bool#FIXME: not updating stability countdown nor "is_stable"
    prediction_error: CircularArray([0, 0, 0])
    derivatives: CircularArray([0, 0, 0])
    stability_timeout: int
    max_timeout: int

# FIXME: as builder
class StabilitySeparatorModel(Model, ByteSplitModel, RiceMapModel):
    def __init__(self) -> None:
        super().__init__()
        self.stable_histogram = Histogram(255)
        self.high_byte_unstable_histogram = Histogram(255)
        self.low_byte_unstable_histogram = Histogram(255)
        for i in range(256):
            self.stable_histogram[i] += 1
            self.high_byte_unstable_histogram[i] += 1
            self.low_byte_unstable_histogram[i] += 1
        self.scape_symbol = 0x1FF
        self.scape_symbol_bits = 9
        self.previous_symbol = 0x00
        self.prev_prev_symbol = 0x00
        self.count_stable = 256
        self.count_unstable = 256
        self.stability_history = StabilityHistory(is_stable=False,
                                                  prediction_error=CircularArray([0, 0, 0]),
                                                  derivatives=CircularArray([0, 0, 0]),
                                                  stability_timeout=3,
                                                  max_timeout=5)

    def _stability(self, errors: CircularArray, derivatives: CircularArray) -> bool:
        for idx in errors.get_indexes():
            if abs(errors[idx]) > 0xFF:
                return False
        error_sum = sum(map(abs,errors))
        if error_sum > 64: # FIXME: parametrize by stddev or stmg
            return False
        for idx in derivatives.get_indexes():
            if abs(derivatives[idx]) > 10: # FIXME: parametrize by stddev or stmg
                return False
        return True

    def _is_signal_stable(self, symbol: np.int16) -> bool:
        if not self.stability_history.is_stable:
            return False
        error = self._get_prediction_error(symbol)
        tmp_error = self.stability_history.prediction_error.copy()
        tmp_derivatives = self.stability_history.derivatives.copy()
        tmp_error.push(error)
        tmp_derivatives.push(self.previous_symbol - self.prev_prev_symbol)
        return self._stability(tmp_error, tmp_derivatives)#FIXME: refactor

    def _update_stability_history(self, symbol: np.int16) -> None:
        error = self._get_prediction_error(symbol)
        self.stability_history.prediction_error.push(error)
        self.stability_history.derivatives.push(self.previous_symbol - self.prev_prev_symbol)
        if not self.stability_history.is_stable:
            self.stability_history.stability_timeout -= 1
        if self.stability_history.stability_timeout == 0:
            self.stability_history.stability_timeout = self.stability_history.max_timeout
            self.stability_history.is_stable = True

    def _get_prediction_error(self, symbol: np.int16) -> np.int16:
        return self.previous_symbol - symbol

    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        mapped_error = RiceMapModel.rice_map(self._get_prediction_error(symbol))
        p = 0
        if self._is_signal_stable(symbol):
            p = self.stable_histogram[mapped_error] / self.count_stable
            self.stable_histogram[mapped_error] += 1
            self.count_stable += 1
        else:
            high_byte, low_byte = ByteSplitModel.split_bytes(mapped_error)
            p_low = self.low_byte_unstable_histogram[low_byte] / self.count_unstable
            p_high = self.high_byte_unstable_histogram[high_byte] / self.count_unstable
            self.low_byte_unstable_histogram[low_byte] += 1
            self.high_byte_unstable_histogram[high_byte] += 1
            self.count_unstable += 1
            p = p_low * p_high
        self._update_stability_history(symbol)
        self.prev_prev_symbol = self.previous_symbol
        self.previous_symbol = symbol
        return p
    
    def get_code_lenght(self, signal: npt.NDArray[np.int16]) -> np.unsignedinteger:
        res = 0
        stability = True
        for x in signal:
            res -= np.log2(self._get_symbol_prob(x))
            if (self._is_signal_stable(x) != stability):
                stability = not stability
                res += self.scape_symbol_bits
        return res

class SklearnModel(Model, ByteSplitModel, RiceMapModel):
    def __init__(self, model: RegressorMixin, context_size = 3):
        self._context_size = context_size
        self.low_byte_histogram = Histogram(255)
        self.high_byte_histogram = Histogram(255)
        self.count = 256
        for i in range(256):
            self.low_byte_histogram[i] = 1
            self.high_byte_histogram[i] = 1
        self.sk_model = model
        self.history = CircularArray([0 for _ in range(self._context_size)])

    def _build_context(signal: npt.NDArray[np.int16], context_size: int) -> [npt.NDArray[np.int16]]:
        assert(context_size >= 1)
        buffer = CircularArray([0 for _ in range(context_size)])
        res = []
        for x in signal:
            res.append(np.asarray(buffer.to_list()))
            buffer.push(x)
        return res
    
    def _train(self, signal):
        context_array = SklearnModel._build_context(signal, self._context_size)
        self.sk_model.fit(context_array, signal)

    def _predict(self) -> np.int16:
        prediction = round(self.sk_model.predict(np.asarray(self.history.to_list()).reshape(1,-1))[0])
        if prediction >= (1 << 15):
            prediction = (1 << 15) - 1
        if prediction < 0:
            prediction = 0
        return np.int16(prediction)

    def _predict_and_advance_history(self, x: np.int16) -> np.int16:
        res = self._predict()
        self.history.push(x)
        return res

    def get_code_lenght(self, signal: npt.NDArray[np.int16]) -> np.unsignedinteger:
        self._train(signal)
        return super().get_code_lenght(signal)

    def _get_symbol_prob(self, symbol: np.int16) -> np.floating:
        prediction = self._predict_and_advance_history(symbol)

        error = symbol - prediction
        high, low = ByteSplitModel.split_bytes(RiceMapModel.rice_map(error))
        p = (self.high_byte_histogram[high] / self.count) * (self.low_byte_histogram[low] / self.count)
        self.high_byte_histogram[high] += 1
        self.low_byte_histogram[low] += 1
        self.count += 1
        return p

class DerivativeContextModel(DefaultAbstractContextModel):
    def __init__(self) -> None:
        super().__init__(
            [Histogram(255), Histogram(255)],
            [Histogram(255), Histogram(255)],
            [256, 256],
            CircularArray([0,0])
        )
    def _predict(self) -> np.int16:
        return self.history[-1]

    def _derivative(self) -> np.int16:
        return self.history[-1] - self.history[-2]

    def _calculate_context_class(self) -> np.uint64:
        derivative = self._derivative()
        if derivative > 0:
            return np.uint64(0)
        else:
            return np.uint64(1)

class DerivativeContextModel2(DefaultAbstractContextModel):
    def __init__(self, thresholds: List[np.int16]):
        self.thresholds = thresholds.copy()
        self.thresholds.sort(reverse=True)
        high_histograms = []
        low_histograms = []
        for _ in range(len(thresholds) + 1):
            high_histograms.append(Histogram(255))
            low_histograms.append(Histogram(255))
        super().__init__(
            list(high_histograms),
            list(low_histograms),
            list(itertools.repeat(256, len(thresholds) + 1)),
            CircularArray([0,0])
        )
        
    def _predict(self) -> np.int16:
        return self.history[-1]

    def _derivative(self) -> np.int16:
        return self.history[-1] - self.history[-2]

    def _calculate_context_class(self) -> np.uint64:
        derivative = self._derivative()
        for i, threshold in enumerate(self.thresholds):
            if abs(derivative) > threshold:
                return i
        return len(self.thresholds)

class DerivativeContextModel3(DerivativeContextModel):
    def _predict(self) -> np.int16:
        return super()._predict() - super()._derivative()

class DerivativeContextModel4(DefaultAbstractContextModel):
    def __init__(self, thresholds: List[np.int16]):
        self.thresholds = thresholds.copy()
        self.thresholds.sort(reverse=True)
        context_classes = (len(thresholds) + 1) * 2
        high_histograms = []
        low_histograms = []
        for _ in range(context_classes):
            high_histograms.append(Histogram(255))
            low_histograms.append(Histogram(255))
        super().__init__(
            list(high_histograms),
            list(low_histograms),
            list(itertools.repeat(256, context_classes)),
            CircularArray([0,0])
        )
    
    def _predict(self) -> np.int16:
        return self.history[-1]

    def _derivative(self) -> np.int16:
        return self.history[-1] - self.history[-2]

    def _calculate_context_class_threshold(self) -> np.uint64:
        derivative = self._derivative()
        for i, threshold in enumerate(self.thresholds):
            if abs(derivative) > threshold:
                return i
        return len(self.thresholds)

    def _calculate_context_class(self) -> np.uint64:
        threshold_ctx = self._calculate_context_class_threshold()
        sign_ctx = 0 if self._derivative() >= 0 else 1
        return (threshold_ctx << 1) | sign_ctx

class RunCodingModel:
    def __init__(self) -> None:
        self.stable_histogram = Histogram(255)
        self.unstable_low_byte_histogram = Histogram(255)
        self.unstable_high_byte_histogram = Histogram(255)
        self.stable_count = 256
        self.unstable_count = 256
        for i in range(256):
            self.stable_histogram[i] += 1
            self.unstable_high_byte_histogram[i] += 1
            self.unstable_low_byte_histogram[i] += 1

#FIXME: bajar run_lenght de unstable y bajar toleracia de estabilidad
#FIXME: dejar periodo de inestabilidad fijo al menos x X tiempo
    def get_code_lenght(self, signal: npt.NDArray[np.int16]) -> np.unsignedinteger:
        code_lenght = 0
        run_length = 0
        code_lenght += 16 # Send first symbol plain
        previous_symbol = signal[0]
        is_stable = True
        for x in signal[1:]:
            prediction = previous_symbol
            prediction_error = x - prediction
            mapped_error = RiceMapModel.rice_map(prediction_error)
            low_byte = mapped_error & 0xFF
            high_byte = (mapped_error & 0xFF00) >> 8
            if is_stable and high_byte == 0:
                code_lenght -= np.log2(self.stable_histogram[low_byte] / self.stable_count)
                self.stable_histogram[low_byte] += 1
                self.stable_count += 1
                run_length += 1
                if run_length == 255:
                    code_lenght += 8 # Send uncompressed run lenght
                    run_length = 0
            elif not is_stable and high_byte != 0:
                code_lenght -= np.log2(self.unstable_high_byte_histogram[high_byte] / self.unstable_count)
                code_lenght -= np.log2(self.unstable_low_byte_histogram[low_byte] / self.unstable_count)
                self.unstable_count += 1
                self.unstable_high_byte_histogram[high_byte] += 1
                self.unstable_low_byte_histogram[low_byte] += 1
                run_length += 1
                if run_length == 255:
                    code_lenght += 8
                    run_length = 0
            elif is_stable and high_byte != 0:
                code_lenght += 8
                run_length = 0
                is_stable = False
                code_lenght -= np.log2(self.unstable_high_byte_histogram[high_byte] / self.unstable_count)
                code_lenght -= np.log2(self.unstable_low_byte_histogram[low_byte] / self.unstable_count)
                self.unstable_count += 1
                self.unstable_high_byte_histogram[high_byte] += 1
                self.unstable_low_byte_histogram[low_byte] += 1
                run_length += 1
            elif not is_stable and high_byte == 0:
                code_lenght += 8
                run_length = 0
                is_stable = True
                code_lenght -= np.log2(self.stable_histogram[low_byte] / self.stable_count)
                self.stable_histogram[low_byte] += 1
                self.stable_count += 1
                run_length += 1
        return code_lenght
                
class EvenOddTimestepCodingModel(DefaultAbstractContextModel):
    def __init__(self) -> None:
        super().__init__(
            high_histograms=[Histogram(255), Histogram(255)],
            low_histograms=[Histogram(255), Histogram(255)],
            counts=[256, 256],
            history=CircularArray([0]))
        self.timestep_parity = 0

    def _predict(self) -> np.int16:
        return self.history[-1]

    def _update_history(self, symbol):
        super()._update_history(symbol)
        if self.timestep_parity == 0:
            self.timestep_parity = 1
        elif self.timestep_parity == 1:
            self.timestep_parity = 0

    def _calculate_context_class(self) -> np.uint64:
        return self.timestep_parity

class EvenOddDerivativeCodingModel(DefaultAbstractContextModel):
    def __init__(self):
        super().__init__(
            high_histograms=[Histogram(255), Histogram(255)],
            low_histograms=[Histogram(255), Histogram(255)],
            counts=[256, 256],
            history=CircularArray([0,0]))
    
    def _predict(self) -> np.int16:
        return self.history[-1]       
    
    def _calculate_context_class(self) -> np.uint64:
        return (self.history[-1] - self.history[-2]) % 2

class DerivativeContextModel5(DefaultAbstractContextModel):
    def __init__(self, add: bool, use_deriv: bool) -> None:
        super().__init__(
            [Histogram(255)],
            [Histogram(255)],
            [256],
            CircularArray([0,0])
        )
        self.add = add
        self.use_deriv = use_deriv
    def _predict(self) -> np.int16:
        if self.use_deriv:
            offset = self._derivative()
        else:
            offset = np.sign(self._derivative())
        if self.add:
            return self.history[-1] + offset
        else:
            return self.history[-1] - offset

    def _derivative(self) -> np.int16:
        return self.history[-1] - self.history[-2]

    def _calculate_context_class(self) -> np.uint64:
        return 0
#if __name__ == "__main__":
    #report = run_mapper_reducer(StabilitySeparatorModel(), [[1,2,3,4]])
    #print(report)
    
    #SklearnModel(LinearRegression()).get_code_lenght([10000,1,2,3,4])