# pod5_nanoraw_comp

Project to develop a compressor for pod5 files containing raw nanopore sequencing data

# How to compile
If compiling for the first time then run
./build.sh init
If not execute
./build.sh c clean release
After that run the script compile_all.sh inside experiments directory.
This will create an executable file inside experiments/compressors for each
compressor.


# How to run
To compress (or decompress) a single file it can be done using
./COMPRESSOR <archivo_entrada> <archivo_salida> <--uncompressed | --VBZ | --pgnano>
Te option pgnano uses the new compressor depending on which executable is being used.
To use the standard VBZ compressor run with --VBZ.

# How to recreate experiments
Inside the experiments directory run the script 
run_experiments.sh

NOTE: For the time results to be coherent, the experiments must be run sequentially. For this the parameter
num_paraller must be set to 1 (line 20 in run_experiments.sh). This will be much slower.

This will run each compressor for each file specified in the root folder inside the script.
The output of the results is written to a file inside the results directory.

Then there are two python script which can be used to process the results.
The first is aggregated.py which will compute the total bps of each database (and for each buffer), and the average of several statistics 
The second one is total_results.py which process the bps, compression and decompression rate (mb/s) and the amount of memory used for each file in each database. 
Both scripts will output to a CSV file.

# Obtaining Datasets
This projecto focuses on pore types R9.4.1, R10.3 and R10.4.1. At least one dataset for each was needed.
On the other hand we are mostly interested in human genome secuencing, but another organism was also included in the experiments.
In the following table the characteristics of each dataset are shown:
| Shortened Name | Organism | Author               | Number of files sampled | Pore type | Total size (GB) |
|----------------|----------|----------------------|-------------------------|-----------|-----------------|
| Fly            | Fly      | Standford University | 14                      | R10.4.1   | 8.8             |
| GIAB10.3       | Human    | ONT                  | 14                      | R10.3     | 12.4            |
| GIAB9.4.1      | Human    | ONT                  | 15                      | R9.4.1    | 12.1            |
| GIAB10.4.1     | Human    | ONT                  | 15                      | R10.4.1   | 12.1            |

Note that GIAB10.3 and GIAB9.4.1 are published as a unique dataset obtained using the two different pore types.

Due to the fact that POD5 is a relatively new file format, most data is published under the format FAST5.
POD5 allows for an arbitrary chunk size to be selected,
which affects how the signal is segmented. Since this is relevant to the compression algorithms algorithms were implemented for POD5 files.
In order to run the experiments the files must be converted.
After the files have been converted they need to be normalized so that they have the same chunk size.

For conversion from FAST5 to POD5 we used a tool provided by ONT availabe on:
https://github.com/nanoporetech/pod5-file-format/tree/master/python/pod5#pod5-convert-fast5

For normalization of chunk size, all the POD5 generated with the conversion tool can then be copied with the POD5 library, using the default chunk size.

| Shortened Name | Link for download                                               |
|:--------------:|-----------------------------------------------------------------|
| GIAB10.4.1     | s3://ont-open-data/giab lsk114 2022.12/                         |
| GIAB9.4.1      | s3://ont-open-data/gm24385 2020.09/                             |
| GIAB10.3       | s3://ont-open-data/gm24385 2020.09/                             |
| Fly            | s3://ont-open-data/contrib/melanogaster bkim 2023.01/flowcells/ |

| Shortened Name | Webpage with more information                             |
|:--------------:|-----------------------------------------------------------|
| GIAB10.4.1     | https://labs.epi2me.io/askenazi-kit14-2022-12/            |
| GIAB9.4.1      | https://labs.epi2me.io/gm24385_2020.09/                   |
| GIAB10.3       | https://labs.epi2me.io/gm24385_2020.09/                   |
| Fly            | https://www.ncbi.nlm.nih.gov/bioproject/?term=PRJNA914057 |

| Shortened Name | Name                                            |
|:--------------:|-------------------------------------------------|
| GIAB10.4.1     | Genome in a Bottle - Ashkenazi Trio             |
| GIAB9.4.1      | GM24385 (Genome in a bottle; 9.4.1)             |
| GIAB10.3       | GM24385 (Genome in a bottle; 10.3)              |
| Fly            | Drosophila melanogaster Nanopore Q20 sequencing |

# Included benchmark
There is also a built in benchmark which runs over a sample of the data.
In order to run it first the data path must be specified in some of the files inside src/python/pgnano.
The first is constants/constants.py where the roots and the venv interpreter path must be specified correctly.
Then in data_obtention/build_benchmark_dataset.py the output path must be specified to output the file benchmark_file.bin inside the root directory of the project.

If the benchmark is being run for the first time then build_benchmark_dataset.py must be run.
Then the executable benchmark inside build/src/c++/benchmark can be run.

The version of the compressor being benchmarked is the one specified in the constant defined un the first line of pod5/c++/pod5_format/pgnano/pgnano.cpp







