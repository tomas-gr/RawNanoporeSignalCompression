import os
from pgnano.constants.constants import *
from typing import Callable

def ls_files_recursive(root: str) -> list[str]:
    ls_dir = list(map(lambda x: root + os.sep + x, os.listdir(root)))
    files = list(filter(lambda x: os.path.isfile(x), ls_dir))
    folders = filter(lambda x: os.path.isdir(x), ls_dir)
    for folder in folders:
        files.extend(ls_files_recursive(folder))
    return files

def _recursive_special(default_root: str, filter_predicate: Callable[[str], bool], root: str) -> list[str]:
    if root is None:
        root = default_root
    return list(filter(filter_predicate, ls_files_recursive(root)))

def get_bams(root: str = None) -> list[str]:
    return _recursive_special(bam_path, lambda x: x.endswith(".bam"), root)

def get_levels(root: str = None) -> list[str]:
    return _recursive_special(levels_path, lambda x: x.endswith(".txt"), root)

def trim_absolute_path(path: str) -> str:
    return path.split("/")[-1]

def trim_extensions(path: str) -> str:
    if path[0] == '.':
        return "." + path.split(".")[1]
    else:
        return path.split(".")[0]