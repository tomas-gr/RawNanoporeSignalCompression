import os
import pod5
from pgnano.constants.constants import data_root_path

def fast5_to_pod5(folder_path: str):
    if folder_path[-1] != os.sep:
        folder_path = folder_path + os.sep
    command = f"pod5 convert fast5 {folder_path}*.fast5 --output {folder_path} --one-to-one {folder_path} -t {os.cpu_count()}"
    os.system(command)

def build_structure():
    if not os.path.isdir(data_root_path + os.sep + "dna"):
        os.mkdir(data_root_path + os.sep + "dna")
    if not os.path.isdir(data_root_path + os.sep + "rna"):
        os.mkdir(data_root_path + os.sep + "rna")

def infer_pore_types(file):
    pore_types = []
    with pod5.Reader(file) as reader:
        for read_record in reader.reads():
            if not read_record.pore.pore_type in pore_types:
                pore_types.append(read_record.pore.pore_type)
    return pore_types

def clear_fast5(folder_path: str):
    folder_path = folder_path if folder_path.endswith(os.sep) else folder_path + os.sep
    for file in os.listdir():
        if os.path.isfile(file) and file.endswith(".fast5"):
            os.remove(file)
