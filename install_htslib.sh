#!/bin/bash
HTSLIB_INSTALL_PATH=/data/pgnanoraw/pod5_fork/htslib
HTSLIB_VERSION="1.18"
HTSLIB_URI="https://github.com/samtools/htslib/releases/download/"$HTSLIB_VERSION"/htslib-"$HTSLIB_VERSION".tar.bz2"


mkdir -p $HTSLIB_INSTALL_PATH
if [ ! -d $PATH ]
then
    mkdir -p $PATH
fi
cd $PATH
wget $HTSLIB_URI -O htslib.tar.bz2
tar -xjf htslib.tar.bz2
cd htslib-$HTSLIB_VERSION
./configure --prefix=$HTSLIB_INSTALL_PATH
make install -j
