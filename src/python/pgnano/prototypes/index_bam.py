# Simplified from remora/io.py@212[compute_read_index]

import pysam
from dataclasses import dataclass
from typing import Callable
from tqdm.auto import tqdm
from functools import cached_property
from collections import defaultdict
from memory_profiler import profile
import pod5

root_path = "/data/pgnanoraw/"
can_pod5_path = root_path + "remora/tests/data/can_reads.pod5"
can_bam_path = root_path + "remora/tests/data/can_mappings.bam"

alt_bam_path = "/data/datananoraw/BAM_TEST/calls2ref.bam"
alt_pod5_path = "/data/datananoraw/BAM_TEST/batch15.pod5"

@dataclass
class ReadIndexedBam:
    """Index bam file by read id. Note that the BAM file handle is closed after
    initialization. Any other operation (e.g. fetch, get_alignments,
    get_first_alignment) will open the pysam file handle and leave it open.
    This allows easier use with multiprocessing using standard operations.

    Args:
        bam_path (str): Path to BAM file
        skip_non_primary (bool): Should non-primary alignmets be skipped
        req_tags (bool): Skip reads without required tags
        read_id_converter (Callable[[str], str]): Function to convert read ids
            (e.g. for concatenated duplex read ids)
    """

    bam_path: str
    skip_non_primary: bool = True
    req_tags: set = None
    read_id_converter: Callable = None

    def __post_init__(self):
        self.num_reads = None
        self.bam_fh = None
        self._bam_idx = None
        self.compute_read_index()

    def open(self):
        # hide warnings for no index when using unmapped or unsorted files
        self.pysam_save = pysam.set_verbosity(0)
        self.bam_fh = pysam.AlignmentFile(
            self.bam_path, mode="rb", check_sq=False
        )

    def close(self):
        self.bam_fh.close()
        self.bam_fh = None
        pysam.set_verbosity(self.pysam_save)

    #@profile
    def compute_read_index(self):
        bam_was_closed = self.bam_fh is None
        if bam_was_closed:
            self.open()
        self._bam_idx = defaultdict(list)
        #pbar = tqdm()
        self.num_records = 0
        # iterating over file handle gives incorrect pointers
        while True:
            read_ptr = self.bam_fh.tell()
            try:
                read = next(self.bam_fh)
            except StopIteration:
                break
            #pbar.update()
            index_read_id = (
                read.query_name
                if self.read_id_converter is None
                else self.read_id_converter(read.query_name)
            )
            self.num_records += 1
            self._bam_idx[index_read_id].append(read_ptr)
        # close bam if it was closed at start of function call
        if bam_was_closed:
            self.close()
        #pbar.close()
        # convert defaultdict to dict
        self._bam_idx = dict(self._bam_idx)
        self.num_reads = len(self._bam_idx)

    def __contains__(self, read_id):
        assert isinstance(read_id, str)
        return read_id in self._bam_idx

    def __getitem__(self, read_id):
        return self._bam_idx[read_id]

    def __del__(self):
        if self.bam_fh is not None:
            self.bam_fh.close()


if __name__ == "__main__":
    bam_path =  alt_bam_path
    pod5_path = alt_pod5_path

    rib = ReadIndexedBam(bam_path)
    with pod5.Reader(pod5_path) as p5:
        fst_read = next(p5.reads())
        #print(rib._bam_idx)
        #print(set([type(x) for x in rib._bam_idx.keys()]))
        #print(fst_read.read_id.__str__())
        idxs = rib._bam_idx[str(fst_read.read_id)]
        one_idx = idxs[0]
        print(str(fst_read.read_id))

    #pysam.index(pod5_path)
    with pysam.AlignmentFile(bam_path, mode="rb", check_sq=False) as bam_fh:
        bam_fh.seek(one_idx)
        r = next(bam_fh)
        #print(r)