#run from utils
git submodule update --init --recursive
pip install build
pip install pre-commit==v2.21.0
pip install setuptools_scm


cd ..
mkdir build

# Must be rerun on every update from ONT
python -m setuptools_scm
python -m pod5_make_version

cd build
cmake ..
