from enum import Enum
from dataclasses import dataclass

from pgnano.constants.constants import data_root_path
import os

from pgnano.data_obtention.folder_traversal import get_bams, get_levels, trim_absolute_path, trim_extensions
from pgnano.data_obtention.get_data import get_dataset_descriptors, pull_and_get_folder_name

Molecule = Enum('Molecule', ['RNA', 'DNA'])

@dataclass
class DatasetInfo():
    path: str
    molecule: Molecule
    dataset: str
    size: int
    #pore_type: PoreType

@dataclass
class SequencedPOD5Info(DatasetInfo):
    bam_path: str
    level_path: str

    @property
    def has_bam(self) -> bool:
        return self.bam_path != None
    
    @property
    def has_levels(self) -> bool:
        return self.level_path != None


def get_datasets() -> list[SequencedPOD5Info]:      # Consigue los Archivos para Analizar #
    files = []
    for molecule_type in ["dna", "rna"]:
        datasets_names = os.listdir(data_root_path + os.sep + molecule_type)
        for dataset_name in datasets_names:
            ds_name = dataset_name if dataset_name.endswith("/") else dataset_name + os.sep
            files_path = os.listdir(data_root_path + os.sep + molecule_type + os.sep + ds_name)
            for f in files_path:
                ds_path = data_root_path + os.sep + molecule_type \
                        + os.sep + dataset_name \
                        + os.sep + f
                mol_type = Molecule.RNA if molecule_type == 'rna' else Molecule.DNA
                dataset = dataset_name
                size = os.stat(ds_path).st_size
                files.append(SequencedPOD5Info(ds_path,mol_type,dataset,size, None, None))
    return files


def find_bam(path: str, bams: list[str]) -> str | None:
    name = trim_extensions(trim_absolute_path(path))
    bams_path_name = list(map(lambda x: (x, trim_extensions(trim_absolute_path(x))), bams))
    tmp = list(filter(lambda x: x[1] == name, bams_path_name))
    candidates = list(map(lambda y: y[0], tmp))
    if len(candidates) > 0:
        return candidates[0]
    else:
        return None


def find_levels(file: DatasetInfo, levels: list[str]) -> str | None:
    descriptors = get_dataset_descriptors()
    matching_descriptor = next(
                                filter(lambda x: x[0] == file.dataset,
                                    zip(
                                        map(pull_and_get_folder_name, descriptors), descriptors)))[1]
    pore_string = matching_descriptor["metadata"]["web_recovered_pore_type"][1:].replace('.', '_')
    candidates = list(filter(lambda x: x[0].find(pore_string) != -1, zip(map(trim_extensions, map(trim_absolute_path, levels)), levels)))
    if len(candidates) == 0:
        return None
    else:
        return candidates[0][1]


def get_datasets_with_sequencing() -> list[SequencedPOD5Info]:
    files = get_datasets()
    bams = get_bams()
    levels = get_levels()

    return list(map(lambda x: SequencedPOD5Info
                (
                path = x.path,
                molecule = x.molecule,
                dataset = x.dataset,
                size = x.size,
                bam_path = find_bam(x.path, bams),
                level_path = find_levels(x, levels)
                ),
                files
               ))


def get_datasets_only_with_sequencing() -> list[SequencedPOD5Info]:
    return list(filter(lambda x: x.has_bam and x.has_levels, get_datasets_with_sequencing()))

if __name__ == "__main__":
    x = get_datasets_with_sequencing()
    print(len(get_datasets_only_with_sequencing()))