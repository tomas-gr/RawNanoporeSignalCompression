import pandas as pd
import numpy as np
import numpy.typing as npt
from pgnano.stats_analysis.map_reduce_stats import run_mapper_reducer
from pgnano.stats_analysis.models import DerivativeContextModel4
from typing import List
from pgnano.stats_analysis.map_reduce_stats import ReducerReportResult

ModelT = DerivativeContextModel4

def linear_parameter_search(params: List[object], signal_dataset: List[npt.NDArray[np.int16]]) -> pd.DataFrame:
    res = []
    for x in params:
        model = ModelT(x)
        reducer_output = run_mapper_reducer(model=model, signal=signal_dataset, disable=True)
        res.append((x, reducer_output.macro_avg_bits_symbol))
    return pd.DataFrame(res,columns=["params", "bits/symbol"])