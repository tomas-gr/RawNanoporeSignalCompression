import os
import typing
import statistics
from dataclasses import dataclass
import random
from pgnano.constants.constants import data_root_path
from functools import reduce
import hashlib

def human_to_byte_parse(size_unit_tuple: typing.Tuple[str, str]) -> int:
    unit_multipliers = {
                        "Bytes": 1,
                        "KiB": 2 << 10,
                        "MiB": 2 << 20,
                        "GiB": 2 << 30,
                        "TiB": 2 << 40
                       }
    size_str: str = size_unit_tuple[0]
    unit: str = size_unit_tuple[1]
    has_dot: bool = False 
    if size_str.find(".") != -1:
        size_str = size_str.replace(".", "")
        has_dot = True
    size: int = int(size_str)
    size = size * unit_multipliers[unit]
    if has_dot:
        size //= 10
    return size

def byte_to_human_parse(size):
    if size < (2 << 10):
        return (size, "Bytes")
    elif size < (2 << 20):
        return (size / 2**10, "KiB")
    elif size < (2 << 30):
        return (size / 2**20, "MiB")
    elif size < (2 << 40):
        return (size / 2**30, "GiB")

@dataclass
class DatasetStats:
    def __init__(self):
    # Constuctor is empty to allow creation and then initialization of fields
        pass

    biggest_size: typing.Tuple[str, str]
    smallest_size: typing.Tuple[str, str]
    mean_size_bytes: float
    median_size_bytes: float
    std_dev_bytes: float
    median_size: typing.Tuple[str, str]
    mean_size: typing.Tuple[str, str]
    std_dev: typing.Tuple[str, str]
    smallest_size_bytes: int
    biggest_size_bytes: int
    num_intervals: int
    interval_size: float

class ONTS3Parser:
    _workdir_name: str = ".ont_s3_workdir/"
    _workdir: str
    _s3_bucket_link: str
    _s3_root_link: str
    _worklist: list[object]

    def __init__(self, s3_root_link) -> None:
        self._s3_root_link = s3_root_link
        self._workdir = self._check_and_make_workdir()
    
    def _check_and_make_workdir(self) -> str:
        workdir_path = (data_root_path if data_root_path.endswith(os.sep) else data_root_path + os.sep) + self._workdir_name
        if not os.path.isdir(workdir_path):
            os.mkdir(workdir_path)
        return workdir_path

    def reset(self) -> None:
        raise NotImplementedError("Not yet implemented")

    def set_s3_bucket_link(self, link) -> None:
        self._s3_bucket_link = link
    
    def get_name(self) -> str:
        return self._s3_bucket_link.replace(self._s3_root_link, "").replace("/","_")[0:-2]

    def retrieve_list(self) -> None:
        if self._s3_bucket_link is None:
            raise Exception("S3 bucket not set!")
        this_file_path = self._workdir + self.get_name()
        is_cached = os.path.isfile(this_file_path)
        if not is_cached:
            os.system(f"aws s3 ls --no-sign-request --human --recursive {self._s3_bucket_link} > {this_file_path}")

    def get_full_list(self) -> list[str]:
        with open(self._workdir + self.get_name(), "r") as f:
            return f.readlines()
    
    def _split_list_to_tuples(self) -> None:
        self._worklist = self.get_full_list()
        self._worklist = map(lambda x: x.strip().split(" "), self._worklist)
        self._worklist = map(lambda x: list(filter(lambda y: y != "", x)), self._worklist)
        self._worklist = list(map(lambda x: {
                                        "size": x[2],
                                        "size_unit": x[3],
                                        "name": x[4]
                                       }, self._worklist))

    def _resolve_list_extensions(self) -> None:
        for x in self._worklist:
            extension: str = x["name"]
            extension = extension.split("/")[-1]
            extension = ".".join(filter(lambda x: len(x) < 6, extension.split(".")))
            x["extension"] = extension

    def _get_extensions(self) -> list[str]:
        return list(map(lambda x: x["extension"], self._worklist))

    def _prune_by_extension(self) -> None:
        self._worklist = filter(lambda x: x["extension"].find("fast5") != -1, self._worklist)

    def _calculate_real_size(self) -> None:
        self._worklist = list(self._worklist)

        for x in self._worklist:
            x["parsed_size"] = human_to_byte_parse((x["size"],x["size_unit"]))

    def get_unique_extension_list(self) -> list[str]:
        extensions = []
        for x in self._worklist:
            if x["extension"] not in extensions:
                extensions.append(x["extension"])
        return extensions

    def _gen_stats(self) -> None:
        self.stats = DatasetStats()
        self.stats.biggest_size = (self._worklist[-1]['size'], self._worklist[-1]['size_unit'])
        self.stats.smallest_size = (self._worklist[0]['size'], self._worklist[0]['size_unit'])
        self.stats.mean_size_bytes = statistics.mean(map(lambda x: x["parsed_size"],self._worklist))
        self.stats.median_size_bytes = statistics.median(map(lambda x: x["parsed_size"],self._worklist))
        self.stats.std_dev_bytes = statistics.stdev(map(lambda x: x["parsed_size"],self._worklist))
        
        self.stats.median_size = byte_to_human_parse(self.stats.median_size_bytes)
        self.stats.mean_size = byte_to_human_parse(self.stats.mean_size_bytes)
        self.stats.std_dev = byte_to_human_parse(self.stats.std_dev_bytes)
        self.stats.smallest_size_bytes = human_to_byte_parse(self.stats.smallest_size)
        self.stats.biggest_size_bytes = human_to_byte_parse(self.stats.biggest_size)
        self.stats.interval_size = self.stats.std_dev_bytes / 2

        self.stats.num_intervals = 0
        curr_val = self.stats.smallest_size_bytes
        while curr_val < self.stats.biggest_size_bytes:
            self.stats.num_intervals += 1
            curr_val += self.stats.interval_size
        self.stats.num_intervals -= 1



    def process_data(self) -> None:
        self._split_list_to_tuples()
        self._resolve_list_extensions()
        self._prune_by_extension()
        self._calculate_real_size()
        self._worklist.sort(key=lambda x: x["parsed_size"])
        self._gen_stats()

class ONTS3Downloader:
    s3_parser: ONTS3Parser
    _sampler_result: list[str] = []
    _rand_gen: random.Random

    def __init__(self, link: str, seed: int, s3_root_link: str):
        self.s3_parser = ONTS3Parser(s3_root_link)
        self.s3_parser.set_s3_bucket_link(link)
        self.s3_parser.retrieve_list()
        self.s3_parser.process_data()
        self._rand_gen = random.Random()
        self._rand_gen.seed(seed, version=2)
    

    def _gen_sample_buckets(self, samples):
        gotten_samples = 0
        i: int = 0
        # TODO: fix this assumption
        # Assumes that randint won't repeat atleast all samples are gotten
        while gotten_samples < samples:
            # Sampling this way is faster than sorting all values into buckets and then
            # sampling each bucket when the sample number is small
            new_sample = False
            while not new_sample:
                sampled_idx = self._rand_gen.randint(0,len(self.s3_parser._worklist) - 1)
                sampled_value = self.s3_parser._worklist[sampled_idx]
                if self.s3_parser.stats.smallest_size_bytes + i * self.s3_parser.stats.interval_size <= sampled_value["parsed_size"] \
                and sampled_value["parsed_size"] <= self.s3_parser.stats.smallest_size_bytes + (i + 1) * self.s3_parser.stats.interval_size:
                    new_sample = True
                    self._sampler_result.append(sampled_value["name"])
            gotten_samples += 1
            i = (i + 1) %  self.s3_parser.stats.num_intervals

    def sample_buckets(self, samples) -> list[str]:
        assert(self.s3_parser.get_unique_extension_list() == ["fast5"])
        if len(self._sampler_result) == 0:
            self._gen_sample_buckets(samples)
        return self._sampler_result
    
    
    # Folder must be created already.
    # Names are the result of self.sample_buckets
    def download_to_folder(self, folder: str, names: str) -> None:
        links = list(map(lambda x: self.s3_parser._s3_root_link + x, names))
        for link, name in zip(links, names):
            self._download_file(folder, link, name)
    
    def _build_download_name(self, name: str):
        file_name = (name.strip().split("/"))[-1]
        prefix = hashlib.md5(name.encode(), usedforsecurity=False).digest()[:4].hex()
        return f"{prefix}-{file_name}"

    # FIXME: SHELL INJECTION!!!
    def _download_file(self, folder, link, name):
        os.system(f"aws s3 cp --no-sign-request --cli-read-timeout 0 --cli-connect-timeout 0 --only-show-errors {link} {folder}/{self._build_download_name(name)}")

if __name__ == "__main__":
    links = [
                "s3://ont-open-data/giab_lsk114_2022.12/",
                "s3://ont-open-data/gm24385_2020.09/",
                "s3://ont-open-data/contrib/melanogaster_bkim_2023.01/flowcells/"
            ]

    s3_handler = ONTS3Downloader(links[1],0)

    print(s3_handler.s3_parser.get_unique_extension_list())
    assert(s3_handler.s3_parser.get_unique_extension_list() == ["fast5"])
    print(s3_handler.s3_parser.stats.biggest_size)
    print(s3_handler.s3_parser.stats.smallest_size)
    print(s3_handler.s3_parser.stats.mean_size)
    print(s3_handler.s3_parser.stats.median_size)
    print(s3_handler.s3_parser.stats.std_dev)
    print(s3_handler.s3_parser.stats.num_intervals)

    print(s3_handler.sample_buckets(1))
    
    a = s3_handler.sample_buckets(1)
    b = s3_handler.sample_buckets(1)
    assert(a == b)

    #s3_handler.download_to_folder(data_root_path,a)