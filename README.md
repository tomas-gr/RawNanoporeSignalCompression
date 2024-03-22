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



