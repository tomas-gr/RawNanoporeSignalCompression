# RawNanoporeSignalCompression
## This project consist on a thorough study of the state of the art lossless compressor for raw nanopore sequencing data (VBZ) developed by ONT, as well as the proposal of several improvements. 
### *Supported platforms:* Linux
### *Authors:* Rafael Castelli, Tomás González, Rodrigo Torrado, Álvaro Martín and Guillermo Dufort Y Álvarez
### *Contact:* tomas.gonzalez.uy@gmail.com

## Instalation from source code
### Install dependencies
A conda environment is used for the instalation.
See [this](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html) page for installation instructions for conda. Once conda is installed, create an environment called "nanoRawEnv" with python=3.11:
```
conda create --name nanoRawEnv python=3.11
```
### Download repository
```
git clone https://github.com/tomas-gr/RawNanoporeSignalCompression.git
```
### Set paths
```
export PROJECT_ROOT=<root to repository>
export MINICONDA_PATH=<root to the miniconda installation>
```
After that in order to install dependencies run:
```
./install_dependencies.sh
```
The packages installed are:
- boost-cpp
- cmake
- flatbuffers
- zstd
- setuptools_scm
- htslib
- GSL (GNU scientific library)
- arrow-cpp (Version 8.0)
- pod5 (python library)

When compiling for the first time run:
```
cd RawNanoporeSignalCompression
./build.sh init
```
After that the compressors are compiled with
```
cd utils
./compile_all.sh
```
This will create an executable file inside utils/compressors for each
compressor.

## How to run
To compress (or decompress) a single file it can be done using
```
./COMPRESSOR <input_file> <output> <--uncompressed | --VBZ | --pgnano>
```
The option pgnano uses the new compressor depending on which executable is being used.
To use the standard VBZ compressor run with --VBZ.

## Check consistency
In order to verify that the compressors are working correctly, there is a script to verify that the files can be recovered correctly.
With the conda environment activated run
```
python src/python/pgnano/main_scripts/ont_check_pod5_files_equal.py file1 file2
```
This script compares two files both compressed with VBZ.

## Obtaining Datasets
This projecto focuses on pore types R9.4.1, R10.3 and R10.4.1. At least one dataset for each was needed.
On the other hand we are mostly interested in human genome secuencing, but another organism was also included in the experiments.
In the following table the characteristics of each dataset are shown:
| Shortened Name | Organism | Author               | Pore type | Total size (GB) | Link for download                                                     | Information          |
|----------------|----------|----------------------|-----------|-----------------|-----------------------------------------------------------------------|----------------------|
| Fly            | Fly      | Standford University | R10.4.1   | 8.8             |[link](s3://ont-open-data/contrib/melanogaster bkim 2023.01/flowcells/)|[link](https://www.ncbi.nlm.nih.gov/bioproject/?term=PRJNA914057)|
| GIAB10.3       | Human    | ONT                  | R10.3     | 12.4            |[link](s3://ont-open-data/gm24385 2020.09/)                            |[link](https://labs.epi2me.io/gm24385_2020.09/ )|
| GIAB9.4.1      | Human    | ONT                  | R9.4.1    | 12.1            |[link](s3://ont-open-data/gm24385 2020.09/)                            |[link](https://labs.epi2me.io/gm24385_2020.09/ )|
| GIAB10.4.1     | Human    | ONT                  | R10.4.1   | 12.1            |[link](s3://ont-open-data/giab lsk114 2022.12/)                        |[link](https://labs.epi2me.io/askenazi-kit14-2022-12/)|

Note that GIAB10.3 and GIAB9.4.1 are published as a unique dataset obtained using the two different pore types.

Due to the fact that POD5 is a relatively new file format, most data is published under the format FAST5.
POD5 allows for an arbitrary chunk size to be selected,
which affects how the signal is segmented. Since this is relevant to the compression algorithms algorithms were implemented for POD5 files.
In order to run the experiments the files must be converted.
After the files have been converted they need to be normalized so that they have the same chunk size.

For conversion from FAST5 to POD5 we used a tool provided by ONT availabe on:
https://github.com/nanoporetech/pod5-file-format/tree/master/python/pod5#pod5-convert-fast5

For normalization of chunk size, all the POD5 generated with the conversion tool can then be copied with the POD5 library, using the default chunk size.

## An Example

In the "example" folder, there's a bash script named "download_example_file.sh." This script is meant to download a fast5 file from the "Fly" dataset mentioned earlier and convert it to a pod5 file. For example, to compress this file using the C1 compressor, you'll need to execute the following command from the "example" folder:

```
./../utils/compressors/C1 FAV70669_117da01a_45f6321d_55.pod5 FAV70669_117da01a_45f6321d_55.C1 --pgnano
```







