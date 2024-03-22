#!/bin/bash

# This script is used to run compression experiments on a set of files located in a root folder.
# It iterates over all databases in the root folder and for each database, it processes all .pod5 files.
# For each file, it uses all compressors located in a specified compressors directory.
# The results of the compression are stored in a results folder, with a specific subfolder for each database and compressor.
# The script supports parallel processing of multiple files.
# It also supports an optional correctness check of the compression algorithm, and an option to overwrite existing results.

# Root folder
root_folder="/data/datananoraw/dna"

# Output folder
output_folder="results"

# Compressors directory
compressors_dir="compressors"

# Number of files to process in parallel
num_parallel=20

# Check if the script received the overwrite argument
overwrite=false
check_correctness=false
only_one=false

# Function to display help message
display_help() {
    echo "Usage: $0 [option...]" >&2
    echo
    echo "This script is used to run compression experiments on a set of files located in a root folder."
    echo "It iterates over all databases in the root folder and for each database, it processes all .pod5 files."
    echo "For each file, it uses all compressors located in a specified compressors directory."
    echo "The results of the compression are stored in a results folder, with a specific subfolder for each database and compressor."
    echo "The script supports parallel processing of multiple files."
    echo "It also supports an optional correctness check of the compression algorithm, and an option to overwrite existing results."
    echo
    echo "   -h, --help         Display this help message"
    echo "   -o, --overwrite    Overwrite existing results"
    echo "   -c, --check        Check the correctness of the compression algorithm"
    echo "   -compressor        Use only a specific compressor - VBZ, VBZ1, KD, L_H, LL_LH, N01, N02"
    echo
    exit 1
}

# Process all arguments
while (( "$#" )); do
  case "$1" in
    -h|--help)
      display_help
      ;;
    -o|--overwrite)
      overwrite=true
      shift
      ;;
    -c|--check)
      check_correctness=true
      shift
      ;;
    -VBZ|--VBZ)
      only_one=true
      the_one="VBZ"	
      shift
      ;;
    -VBZ1|--VBZ1)
      only_one=true
      the_one="VBZ1"
      shift
      ;;
    -KD|--KD)
      only_one=true
      the_one="KD"
      shift
      ;;
    -L_H|--L_H)
      only_one=true
      the_one="L_H"
      shift
      ;;
    -LL_LH|--LL_LH)
      only_one=true
      the_one="LL_LH"
      shift
      ;;
    -N01|--N01)
      only_one=true
      the_one="N01"
      shift
      ;;
    -N02|--N02)
      only_one=true
      the_one="N02"
      shift
      ;;
    -pgnano|--pgnano)
      only_one=true
      the_one="pgnano"
      shift
      ;;
    -N03|--N03)
      only_one=true
      the_one="N03"
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done


# Function to perform compression and optional correctness check
compress_and_check() {
    local compressor=$1
    local input_file=$2
    local output_name=$3
    local results_file=$4

    # Run the binary
    echo "Running binary..."
    compressor_name=$(basename "$compressor")	
    if [ "$compressor_name" = "VBZ" ]; then
      "$compressor" "$input_file" "$output_name" --VBZ > "$results_file"
    else
      "$compressor" "$input_file" "$output_name" --pgnano > "$results_file"
    fi

    # Check the correctness of the compression algorithm if the check argument was passed
    if $check_correctness; then
        # Generate a random name for the decompressed file
        
        # Decompress the compressed file
        if [ "$compressor_name" != "VBZ" ]; then
          local decompressed_file=$(mktemp)
          "$compressor" "$output_name" "$decompressed_file" --VBZ > /dev/null
        fi


        # Compare the original and decompressed files
        if [ "$compressor_name" = "VBZ" ]; then
          local check_result=$(python ont_check_pod5_files_equal.py "$output_name" "$input_file")
        else
          local check_result=$(python ont_check_pod5_files_equal.py "$decompressed_file" "$input_file")
        fi
        
        
        # Check if the files are consistent
        if [ "$check_result" != "Files consistent" ]; then
            # Write an error message to the results file and terminal
            echo "ERROR: Files are not consistent"
            echo "ERROR: Files are not consistent" >> "$results_file"
        else 
            # Write a success message to the results file
            echo "Files consistent"
            echo "Files consistent" >> "$results_file"
        fi
        
        # Delete the temporary decompressed file
        if [ "$compressor_name" != "VBZ" ]; then
          rm -f "$decompressed_file"
        fi        
    fi
}

echo "Creating output directory if it doesn't exist..."
# Create the output directory if it doesn't exist
mkdir -p "$output_folder"

echo "Iterating over all databases in the root folder..."
# Iterate over all databases in the root folder
for database in "$root_folder"/*; do
    # Check if it's a directory
    if [ -d "$database" ]; then
        # Get the database name
        database_name=$(basename "$database")

        echo "Processing database: $database_name"

        # Counter for parallelization
        count=0

        # Iterate over all .pod5 files in the database
        for input_file in "$database"/*.pod5; do
            # Check if it's a file
            if [ -f "$input_file" ]; then
                # Get the input file name without extension
                input_file_name=$(basename "$input_file" .pod5)

                echo "Processing file: $input_file_name.pod5"
                # Iterate over all compressors
                for compressor in "$compressors_dir"/*; do
                    compressor_name=$(basename "$compressor")
                    if [ "$only_one" != true ] || [ "$compressor_name" = "$the_one" ]; then
                        # Check if it's a file
                        if [ -f "$compressor" ]; then
                            # Get the compressor name

                            echo "Using compressor: $compressor_name"
                            # Generate a random output name
                            output_name=$(mktemp)

                            # Create a specific folder for the results of each compressor for each database
                            mkdir -p "$output_folder/$database_name/$compressor_name"

                            # Define the results file path
                            results_file="$output_folder/$database_name/$compressor_name/$input_file_name"

                            # Check if the results file exists and is not empty, or if overwrite is true
                            if $overwrite || [ ! -s "$results_file" ]; then
                                # Run the compression and optional correctness check in the background
                                compress_and_check "$compressor" "$input_file" "$output_name" "$results_file" &

                                # Increment the counter
                                ((count++))

                                # If the number of parallel processes has reached the limit, wait for all processes to finish
                                if ((count % num_parallel == 0)); then
                                    wait
                                fi
                            else
                                echo "Results file already exists and is not empty, skipping..."
                            fi
                            rm -f "$output_name"
                        fi
                    fi
                done
            fi
        done

        # Wait for all remaining processes to finish
        wait
    fi
done

echo "Script execution completed."
