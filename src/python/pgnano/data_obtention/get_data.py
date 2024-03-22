import sys
import os
import json
from pgnano.constants.constants import data_root_path, data_descriptor_filename, organisms_map_filename, author_map_filename, SEED, SAMPLE_NUMBER
from pgnano.test_scripts.dummy_download_test import dummy_download_test
from pgnano.test_scripts.normalize_chunk_size import normalize_chunk_size
import pgnano.data_obtention.utils as utils
import pgnano.data_obtention.parse_ont_s3_data as s3h
import re
import multiprocessing
import threading

## TODO: FIXME: Encapsulate dataset logic in a separate class
organisms_map = None
author_map = None
io_lock = threading.Lock()


def get_folder_dataset(dataset):
    dataset_name: str = dataset["name"]
    dataset_name = dataset_name.replace('.','_')
    ds_name_list = re.split(r'\s|\(\S*\)', dataset_name)
    ds_name = filter(lambda x: (re.fullmatch(r'\w*',x,flags=re.ASCII) is not None) and x != "", ds_name_list)
    ds_name = list(map(lambda x: x[0] if not x[0].isdigit() else x, ds_name))
    ds_name = "".join(ds_name)
    return ds_name

def get_folder_author(dataset):
    author = dataset["metadata"]["author"]
    if author in author_map:
        return author_map[author]
    else:
        print(f"Unrecognized author name {author}",file=sys.stderr)
        return author
    
def get_folder_organism(dataset):
    organisms = dataset["metadata"]["organisms"]
    if organisms in organisms_map:
        return organisms_map[organisms]
    else:
        print(f"Unrecognized organisms name {organisms}",file=sys.stderr)
        return organisms
    
def gen_folder_name(dataset):
    return get_folder_organism(dataset) + "_" \
         + get_folder_dataset(dataset) + "_" \
         + get_folder_author(dataset)

def is_DNA(dataset: object) -> bool:
    return dataset["metadata"]["is_DNA"]

def make_folder(dataset: object) -> str:
    folder_name = gen_folder_name(dataset)
    folder_path = data_root_path + os.sep \
                + ("dna" if is_DNA(dataset) else "rna") + os.sep \
                + folder_name
    if not os.path.isdir(folder_path):
        os.mkdir(folder_path)
    return folder_path

#TODO: Make async to improve data retrieval
#TODO: FIXME: FIXME: TODO: SHELL INJECTION!!!
def pull_data(dataset: object) -> None:
    location = dataset["data"]["location"]
    folder_path = make_folder(dataset)
    #if os.listdir(folder_path) == 0:
    curr_dir = os.getcwd()
    os.chdir(folder_path)
    if location == "script":
        os.system(dataset["data"]["script"])
    elif location == "s3":
        handler = s3h.ONTS3Downloader(dataset["data"]["link"], SEED, dataset["data"]["root_link"])
        handler.download_to_folder(os.getcwd(), handler.sample_buckets(SAMPLE_NUMBER))
    else:
        raise Exception(f"Unrecognized data location {location} \n Manual download might be required")
    
    run_post_process_script(dataset)
    os.chdir(curr_dir)

def handle_pull_data(dataset: object) -> None:
    try:
        pull_data(dataset)
    except Exception as e:
        io_lock.acquire(blocking=True)
        print(e,sys.stderr, flush=True)
        io_lock.release()

def run_post_process_script(dataset: object) -> None:
    if dataset["data"]["post_download_script"] is None:
        return
    
    if dataset["data"]["post_download_script"]["type"] == "custom":
        run_custom_script(dataset["data"]["post_download_script"]["script"])
        return

    raise NotImplementedError(f"Unrecognized post script type {dataset['post_download_script']['type']}")


def run_custom_script(script: list[str]):
    for command in script:
        if command == "fast5_to_pod5":
            utils.fast5_to_pod5(os.getcwd())
        elif command == "clean_fast5":
            utils.clear_fast5(os.getcwd())
        elif command == "dummy_copy_test":
            cwd = os.getcwd()
            res = []
            for x in os.listdir(cwd):
                res.append(dummy_download_test(cwd + os.sep + x))
            negative_count = res.count(False)
            if negative_count > 0:
                raise RuntimeError(f"Dummy download test failed at {cwd}\nGot {negative_count} negatives from {len(res)}")
        elif command == "normalize_chunk_size":
            cwd = os.getcwd()
            res = []
            for x in os.listdir(cwd):
                res.append(normalize_chunk_size(cwd + os.sep + x))
            negative_count = res.count(False)
            if negative_count > 0:
                raise RuntimeError(f"Failed to normalize chunk size at {cwd}\nGot {negative_count} negatives from {len(res)}")
        else:
            raise RuntimeError(f"Unrecognized command: {command}")

def get_dataset_descriptors():
    with open(data_root_path + os.sep + data_descriptor_filename, "r") as data_descriptor_fp:
        data_descriptor = json.load(data_descriptor_fp)
    return data_descriptor

def pull_and_get_folder_name(dataset: object):
    global organisms_map
    global author_map
    
    with open(data_root_path + os.sep + organisms_map_filename, "r") as h:
        organisms_map = json.load(h)
    
    with open(data_root_path + os.sep + author_map_filename, "r") as h:
        author_map = json.load(h)

    return gen_folder_name(dataset)

if __name__ == "__main__":
    utils.build_structure()

    with open(data_root_path + os.sep + data_descriptor_filename, "r") as data_descriptor_fp:
        data_descriptor = json.load(data_descriptor_fp)
    
    with open(data_root_path + os.sep + organisms_map_filename, "r") as h:
        organisms_map = json.load(h)
    
    with open(data_root_path + os.sep + author_map_filename, "r") as h:
        author_map = json.load(h)

    with multiprocessing.Pool(os.cpu_count()) as p:
        p.map(handle_pull_data, data_descriptor)