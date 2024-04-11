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
include src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/compiler_depend.make

# Include the progress variables for this target.
include src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/progress.make

# Include the compile flags for this target's objects.
include src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/flags.make

src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o: src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/flags.make
src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o: ../src/c++/tests/standalone_compression_decompression_test.cpp
src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o: src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o -MF CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o.d -o CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o -c /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/tests/standalone_compression_decompression_test.cpp

src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.i"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/tests/standalone_compression_decompression_test.cpp > CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.i

src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.s"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/tests/standalone_compression_decompression_test.cpp -o CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.s

# Object files for target standalone_compression_decompression_test
standalone_compression_decompression_test_OBJECTS = \
"CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o"

# External object files for target standalone_compression_decompression_test
standalone_compression_decompression_test_EXTERNAL_OBJECTS =

src/c++/tests/standalone_compression_decompression_test: src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/standalone_compression_decompression_test.cpp.o
src/c++/tests/standalone_compression_decompression_test: src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/build.make
src/c++/tests/standalone_compression_decompression_test: src/c++/compressionLib/libpgnanoCompressionLib.a
src/c++/tests/standalone_compression_decompression_test: src/c++/commonLib/libpgnanoCommonLib.a
src/c++/tests/standalone_compression_decompression_test: ../src/c++/third_party/htslib/lib/libhts.so
src/c++/tests/standalone_compression_decompression_test: src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable standalone_compression_decompression_test"
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/standalone_compression_decompression_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/build: src/c++/tests/standalone_compression_decompression_test
.PHONY : src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/build

src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/clean:
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests && $(CMAKE_COMMAND) -P CMakeFiles/standalone_compression_decompression_test.dir/cmake_clean.cmake
.PHONY : src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/clean

src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/depend:
	cd /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tomasgonzalez/Documents/RawNanoporeSignalCompression /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/tests /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/c++/tests/CMakeFiles/standalone_compression_decompression_test.dir/depend

