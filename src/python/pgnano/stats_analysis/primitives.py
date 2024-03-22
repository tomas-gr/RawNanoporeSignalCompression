import pod5
import numpy as np
import numpy.typing as npt
from typing import List, Tuple, Union
import enum
import abc

class PGPoreType(enum.Enum):
    P10_4_1 = 1
    P10_3 = 2
    P9_4_1 = 3
#FIXME: Use np.arary
#FIXME: Change semantics of limit...
class Histogram:
    def __init__(self, limit=None, initial_array=None) -> None:
        if limit is None and initial_array is None:
            raise RuntimeError("Histogram limit must not be None!")
        if initial_array is not None:
            self.m_rep = np.copy(initial_array)
        elif limit is not None:
            self.m_rep = np.zeros(limit + 1, dtype=np.uint64)
        self.idxs = np.arange(self.m_rep.shape[0], dtype=np.uint64)
    
    def __getitem__(self, index):
        return self.m_rep[index]

    def __setitem__(self, index, value):
        self.m_rep[index] = value

    def __add__(self, other):
        return Histogram(initial_array=(self.m_rep + other.m_rep))
    
    def get_indexes(self) -> npt.NDArray[np.uint64]:
        return self.idxs

    def get_data(self) -> npt.NDArray[np.uint64]:
        return self.m_rep

def split_by_bytes(signal: npt.NDArray[np.int16]) -> npt.NDArray[np.uint8]:
    return signal.to_bytes()
    
def split_by_bytes_hg(signal: npt.NDArray[np.int16]) -> Tuple[Histogram, Histogram]:
    low_hs = Histogram(255)
    high_hs = Histogram(15)
    first_byte = signal & 0xFF
    second_byte = (signal & 0xFF00) >> 8
    for i in range(0, len(signal)):
        low_hs[first_byte[i]] += 1
        high_hs[second_byte[i]] += 1
    return (low_hs, high_hs)

def split_by_nibbles_hg(signal: npt.NDArray[np.int16]) -> Tuple[Histogram, Histogram, Histogram, Histogram]:
    hg_1 = Histogram(15)
    hg_2 = Histogram(15)
    hg_3 = Histogram(15)
    hg_4 = Histogram(15)
    nibble_1 = signal & 0xF
    nibble_2 = (signal & 0xF0) >> 4
    nibble_3 = (signal & 0xF00) >> 8
    nibble_4 = (signal & 0xF000) >> 12
    for i in range(0,len(signal)):
        hg_1[nibble_1[i]] += 1
        hg_2[nibble_2[i]] += 1
        hg_3[nibble_3[i]] += 1
        hg_4[nibble_4[i]] += 1
    return (hg_1, hg_2, hg_3, hg_4)

def get_read_signal_in_chunks(r: pod5.ReadRecord):
    num_signal_rows = len(r.signal_rows)
    res = []
    for i in range(num_signal_rows):
        res.append(r.signal_for_chunk(i))
    return res