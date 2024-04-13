#!/bin/bash

# Check if MINICONDA_PATH is set
if [ -z "$MINICONDA_PATH" ]; then
    echo "Error: MINICONDA_PATH environment variable is not set."
    exit 1
fi

# Install dependencies using Conda
conda install boost-cpp cmake flatbuffers zstd setuptools_scm

# Install htslib
conda install -c bioconda htslib

if ! command -v gsl-config &> /dev/null; then
    echo "GSL is not installed. Installing..."
    conda install -c conda-forge gsl
else
    echo "GSL is already installed."
fi

# Install Arrow only if not provided by the OS
if ! command -v arrow &> /dev/null; then
    conda install arrow-cpp=8.0.0
else
    echo "Arrow is already installed. -> Check that version 8.0 is installed"
fi

# Install pod5 using pip
pip install pod5

echo "Environment setup complete."
