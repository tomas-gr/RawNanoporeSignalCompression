#!/bin/bash
CNL_INSTALL_PATH="/data/pgnanoraw/pod5_fork/pod5/c++/pod5_format/pgnano/third_party"
CNL_URI="https://github.com/johnmcfarlane/cnl.git"

cd $CNL_INSTALL_PATH
git clone $CNL_URI --branch v1.x
cd $CNL_INSTALL_PATH/cnl
rm -rf .git