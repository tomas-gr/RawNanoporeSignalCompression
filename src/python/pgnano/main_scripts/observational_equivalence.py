import sys
import pod5
import typing
import functools
from dataclasses import dataclass
from math import isnan
import numpy as np

# TODO: use hashable property to speedup search on read comparison
# FIXME: report issue => default er?
# Wrapped Read class with custom eq (default eq fails on signal array comparison)
# Also makes class "inmutable"
# TODO: Just make a function, no need for a class
@dataclass(init=True,frozen=True)
class EqRead:
    _read: pod5.Read
    # TODO: code cleanup
    def __eq__(self: pod5.Read, other: pod5.Read):
        def nan_eq(x: float, y: float):
            if x != y:
                return isnan(x) and isnan(y)
            else:
                return True
        
        def shiftScalePair_eq(x: pod5.pod5_types.ShiftScalePair, y: pod5.pod5_types.ShiftScalePair):
            return nan_eq(x.scale, y.scale) and nan_eq(x.shift, y.shift)

        if isinstance(other, EqRead):
            sread: pod5.Read = self._read
            oread: pod5.Read = other._read
            if sread is None or oread is None:
                return False
            if sread.read_id != oread.read_id:
                return False
            if sread.pore != oread.pore:
                return False
            if sread.read_number != oread.read_number or sread.start_sample != oread.start_sample:
                return False
            if not nan_eq(sread.median_before, oread.median_before):
                return False
            if sread.end_reason != oread.end_reason:
                return False
            if sread.run_info != oread.run_info: # TODO: check that this comparison is relevant
                return False
            if not nan_eq(sread.time_since_mux_change, oread.time_since_mux_change):
                return False
            if sread.num_minknow_events != oread.num_minknow_events or sread.num_reads_since_mux_change != oread.num_reads_since_mux_change:
                return False
            if not shiftScalePair_eq(sread.tracked_scaling, oread.tracked_scaling) or not shiftScalePair_eq(sread.predicted_scaling, oread.predicted_scaling):
                return False
            return np.array_equal(sread.signal, oread.signal)
        return NotImplemented


def get_readers() -> typing.Tuple[pod5.Reader, pod5.Reader]:
    in_args = sys.argv
    filename1 = filename2 = None
    if len(in_args) != 3:
        raise Exception(f"Incorrect arguments passed: {functools.reduce(lambda acc, x: acc + ' ' + x,in_args, '')}")
    else:
        filename1 = in_args[1]
        filename2 = in_args[2]

    reader1 = pod5.Reader(filename1)
    reader2 = pod5.Reader(filename2)
    return (reader1, reader2)

def compare_reads(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    raise NotImplementedError()
    pass

def eq_read_ids(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    ids1 = r1.read_ids.sort()
    ids2 = r2.read_ids.sort()
    return ids1 == ids2

def eq_read_ids_raw(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    raise NotImplementedError()
    pass

def eq_reads(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    r1_len = r2_len = 0
    reads_2 = r2.reads()
    reads_2_cache = []
    for read_record in r1.reads():
        r1_len += 1
        read1 = EqRead(read_record.to_read())
        try:
            idx = reads_2_cache.index(read1)
            reads_2_cache.remove(idx)
        except ValueError:
            try:
                read2 = EqRead(next(reads_2).to_read())
                r2_len += 1
                while read2 != read1:
                    reads_2_cache.append(read2)
                    read2 = next(reads_2)
                    r2_len += 1
            except StopIteration:
                return False
    try:
        next(reads_2)
        return False
    except StopIteration:
        return r1_len == r2_len

def compare_metadata(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    same_compression = r1.is_vbz_compressed == r2.is_vbz_compressed
    if not same_compression:
        return False
    same_num_reads = r1.num_reads == r2.num_reads
    if not same_num_reads:
        return False
    same_read_ids = eq_read_ids(r1, r2) 
    if not same_read_ids:
        return False
    #same_read_ids_raw = eq_read_ids_raw(r1, r2)
    return same_read_ids

def compare_data(r1: pod5.Reader, r2: pod5.Reader) -> bool:
    comparable_reads = eq_reads(r1, r2)
    # TODO: FIXME:
    # Compare reads with gen
    # TODO:
    # Compare tables with arrow => https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table.equals
    # Verify tables with arrow https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table.validate
    comparable_signal_tables = None
    comparable_read_tables = None
    comparable_run_tables = None
    signal_table_1_ok = None
    signal_table_2_ok = None
    run_table_1_ok = None
    run_table_2_ok = None
    read_table_1_ok = None
    read_table_2_ok = None
    return comparable_reads

if __name__ == "__main__":
    print(sys.argv)
    reader1, reader2 = get_readers()
    if not compare_metadata(reader1, reader2):
        print("Files metadata differ")
    elif not compare_data(reader1, reader2):
        print("Files data differ")
    else:
        print("Files are observationally equal")

    reader1.close()
    reader2.close()