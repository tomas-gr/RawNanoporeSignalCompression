#!/bin/bash

# Specify the source directory where your C++ files are located
source_directory="../"

# Specify the destination directory where you want to copy the binary files
if [ ! -d "compressors" ]; then
    mkdir -p compressors
fi
destination_directory="./compressors"

# List of C++ files to compile
files=("../pod5/c++/pod5_format/pgnano/pgnano.cpp" "../src/c++/copy.cpp")

# List of compressors to be compiled
compressors=("VBZ0" "VBZ1" "C1" "C2" "C3" "C4" "C5")
# 
# Specify the line number you want to replace
line_number=1


# Loop through each C++ file
for compressor in "${compressors[@]}"; do
    
    # Specify the new line content
    new_line="#define COMPRESSOR_$compressor"
    output_path="$destination_directory/$compressor"
    
    for file_path in "${files[@]}";do
    
    	# Modify the lines in your C++ file as needed
    	sed -i "${line_number}s/.*/$new_line/" "$file_path"
    	echo "Line $line_number in $file_path replaced with: $new_line"
    done
    
    # Compile
    cd ..
    /data/pinanoraw/tgonzalez/pod5_nanoraw/build.sh c dirty release
    cd utils


    # Check the exit status
    if [ $? -eq 0 ]; then
        echo "Compressor $compressor compiled succesfully"
    else
        echo "Compressor $compressor found an error in compilation"
    fi
    
    # Copy the binary file to the destination directory
    cp "../build/src/c++/copy" "$output_path"
    
done

