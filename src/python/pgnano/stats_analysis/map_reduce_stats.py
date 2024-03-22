from collections import namedtuple
from pgnano.stats_analysis.models import Model, VbzModel
from typing import Iterable, Callable, List
import statistics as st
from tqdm.auto import tqdm
import multiprocessing as mp
import os
import numpy as np
import numpy.typing as npt
import functools
from pgnano.stats_analysis.models import StabilitySeparatorModel

MapperReportResult = namedtuple('MapperReportResult', 
                                ['idx', 
                                 'code_len', 
                                 'signal_length',
                                 'uncompressed_bits', 
                                 'compression_ratio', 
                                 'is_better_compressed', 
                                 "vbz_compressed_bits"])
ReducerReportResult = namedtuple('ReducerReportResult', 
                                 ['macro_avg_bits_symbol',
                                  'macro_avg_ratio',
                                  'micro_avg_ratio', 
                                  'percentage_better_compressed', 
                                  'number_better_compressed', 
                                  'total_number',
                                  'against_vbz_micro_avg_ratio',
                                  ])
MapperInput = namedtuple('MapperInput', ['idx', 'signal'])

def abstract_mapper(m: Model, x: MapperInput) -> MapperReportResult:
    signal = x[1]
    idx = x[0]
    signal_length = len(signal)
    code_len = m.get_code_lenght(signal)
    uncompressed_bits = signal_length * 16
    compression_ratio = code_len / uncompressed_bits
    is_better_compressed = code_len < uncompressed_bits
    return MapperReportResult(idx, code_len, signal_length, uncompressed_bits, compression_ratio, is_better_compressed, VbzModel().get_code_lenght(signal))

def abstract_reducer(it: Iterable[MapperReportResult]) -> ReducerReportResult:
    res = list(it)
    return ReducerReportResult(
        macro_avg_bits_symbol=st.mean(map(lambda x: x.code_len / x.signal_length, res)),
        macro_avg_ratio=st.mean(map(lambda x: x.compression_ratio, res)),
        micro_avg_ratio= (sum(map(lambda x: x.code_len, res))) / (sum(map(lambda x: x.uncompressed_bits, res))),
        number_better_compressed=sum(map(lambda x: 1 if x.is_better_compressed else 0, res)),
        total_number=len(res),
        percentage_better_compressed=100*(sum(map(lambda x: 1 if x.is_better_compressed else 0, res)) / len(res)),
        against_vbz_micro_avg_ratio= (sum(map(lambda x: x.code_len, res))) / (sum(map(lambda x: x.vbz_compressed_bits, res)))
    )

def run_mapper(mapper: Callable[[MapperInput], MapperReportResult],
               signal: List[npt.NDArray[np.int16]],
               disable: bool = False) -> Iterable[MapperReportResult]:
    with mp.Pool(os.cpu_count()) as p:
        res = list(
            tqdm(
                p.imap(mapper, enumerate(signal)),
                total=len(signal),
                disable=disable
            )
        )
    return res

def run_mapper_reducer(model: Model,
                       signal: List[npt.NDArray[np.int16]],
                       disable: bool = False) -> ReducerReportResult:
    concrete_mapper = functools.partial(abstract_mapper, model)
    mapper_res = run_mapper(concrete_mapper, signal, disable)
    return abstract_reducer(mapper_res)