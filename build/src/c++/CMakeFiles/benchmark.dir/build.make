# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/tomasgonzalez/miniconda3/envs/nanoRawEnv/bin/cmake

# The command to remove a file.
RM = /home/tomasgonzalez/miniconda3/envs/nanoRawEnv/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tomasgonzalez/Documents/RawNanoporeSignalCompression

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build

# Include any dependencies generated for this target.
include src/c++/CMakeFiles/benchmark.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/c++/CMakeFiles/benchmark.dir/compiler_depend.make

# Include the progress variables for this target.
include src/c++/CMakeFiles/benchmark.dir/progress.make

# Include the compile flags for this target's objects.
include src/c++/CMakeFiles/benchmark.dir/flags.make

src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o: src/c++/CMakeFiles/benchmark.dir/flags.make
src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o: ../src/c++/benchmark.cpp
src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o: src/c++/CMakeFiles/benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o -MF CMakeFiles/benchmark.dir/benchmark.cpp.o.d -o CMakeFiles/benchmark.dir/benchmark.cpp.o -c /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/benchmark.cpp

src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/benchmark.dir/benchmark.cpp.i"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/benchmark.cpp > CMakeFiles/benchmark.dir/benchmark.cpp.i

src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/benchmark.dir/benchmark.cpp.s"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/benchmark.cpp -o CMakeFiles/benchmark.dir/benchmark.cpp.s

# Object files for target benchmark
benchmark_OBJECTS = \
"CMakeFiles/benchmark.dir/benchmark.cpp.o"

# External object files for target benchmark
benchmark_EXTERNAL_OBJECTS =

src/c++/benchmark: src/c++/CMakeFiles/benchmark.dir/benchmark.cpp.o
src/c++/benchmark: src/c++/CMakeFiles/benchmark.dir/build.make
src/c++/benchmark: src/c++/commonLib/libpgnanoCommonLib.a
src/c++/benchmark: src/c++/compressionLib/libpgnanoCompressionLib.a
src/c++/benchmark: pod5/c++/libpod5_format_fork.a
src/c++/benchmark: /usr/lib64/libgsl.so
src/c++/benchmark: /usr/lib64/libgslcblas.so
src/c++/benchmark: ../src/c++/third_party/TurboPFor-Integer-Compression/libic.a
src/c++/benchmark: /usr/lib64/libzstd.so
src/c++/benchmark: src/c++/commonLib/libpgnanoCommonLib.a
src/c++/benchmark: ../src/c++/third_party/htslib/lib/libhts.so
src/c++/benchmark: /home/tomasgonzalez/miniconda3/envs/nanoRawEnv/lib/libarrow.so
src/c++/benchmark: /home/tomasgonzalez/miniconda3/envs/nanoRawEnv/lib/libflatbuffers.a
src/c++/benchmark: /usr/lib64/libzstd.so
src/c++/benchmark: /usr/lib64/libgsl.so
src/c++/benchmark: /usr/lib64/libgslcblas.so
src/c++/benchmark: src/c++/CMakeFiles/benchmark.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable benchmark"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/benchmark.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/c++/CMakeFiles/benchmark.dir/build: src/c++/benchmark
.PHONY : src/c++/CMakeFiles/benchmark.dir/build

src/c++/CMakeFiles/benchmark.dir/clean:
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ && $(CMAKE_COMMAND) -P CMakeFiles/benchmark.dir/cmake_clean.cmake
.PHONY : src/c++/CMakeFiles/benchmark.dir/clean

src/c++/CMakeFiles/benchmark.dir/depend:
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tomasgonzalez/Documents/RawNanoporeSignalCompression /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++ /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++ /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/CMakeFiles/benchmark.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/c++/CMakeFiles/benchmark.dir/depend

