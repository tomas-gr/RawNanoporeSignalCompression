# Install script for directory: /home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xthird_partyx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xthird_partyx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xthird_partyx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xarchivex" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/third_party/pod5/c++/libpod5_format.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xarchivex" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pod5_format" TYPE FILE FILES
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/file_writer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/file_reader.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/schema_metadata.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/read_table_reader.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/read_table_schema.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/read_table_writer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/read_table_writer_utils.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/read_table_utils.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/run_info_table_writer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/run_info_table_reader.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/run_info_table_schema.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/signal_compression.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/signal_table_reader.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/signal_table_schema.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/signal_table_writer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/signal_table_utils.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/c_api.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/errors.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/expandable_buffer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/result.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/dictionary_writer.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/schema_field_builder.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/schema_utils.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/table_reader.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/thread_pool.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/tuple_utils.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/src/c++/third_party/pod5/c++/pod5_format/types.h"
    "/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/third_party/pod5/c++/pod5_format/pod5_format_export.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/tomasgonzalez/Documents/RawNanoporeSignalCompression/build/src/c++/third_party/pod5/c++/test/cmake_install.cmake")
endif()

