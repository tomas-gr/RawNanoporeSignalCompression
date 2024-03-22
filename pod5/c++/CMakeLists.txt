if (ENABLE_CONAN)
    find_package(Arrow REQUIRED CONFIG)
    find_package(Boost REQUIRED CONFIG)
    find_package(Flatbuffers REQUIRED CONFIG)
    find_package(zstd REQUIRED CONFIG)

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(jemalloc REQUIRED CONFIG)
    endif()
else()
    find_package(Arrow REQUIRED)
    find_package(
        Boost REQUIRED
        COMPONENTS
            headers
    )
    find_package(Flatbuffers REQUIRED)
    find_package(zstd REQUIRED)
endif()

find_package(Threads REQUIRED)
find_package(GSL REQUIRED)

include_directories(${HTSLIB_INCLUDE_DIRS})
link_directories(${HTSLIB_LIB_DIR})

find_program(
    FLATBUFFERS_FLATC_EXECUTABLE
    flatc
)
include(BuildFlatBuffers)

configure_file(
    pod5_format/version.h.in
    pod5_format/version.h
)

set(pod5_library_type STATIC)
if (BUILD_SHARED_LIB)
    set(pod5_library_type SHARED)
endif()

add_library(pod5_format_fork ${pod5_library_type}
    pod5_format/file_writer.cpp
    pod5_format/file_writer.h
    pod5_format/file_reader.cpp
    pod5_format/file_reader.h
    pod5_format/file_updater.cpp
    pod5_format/file_updater.h
    pod5_format/file_recovery.h

    pod5_format/async_signal_loader.cpp
    pod5_format/async_signal_loader.h

    pod5_format/schema_metadata.cpp
    pod5_format/table_reader.h
    pod5_format/schema_field_builder.h

    pod5_format/read_table_reader.cpp
    pod5_format/read_table_reader.h
    pod5_format/read_table_schema.cpp
    pod5_format/read_table_schema.h
    pod5_format/read_table_writer.cpp
    pod5_format/read_table_writer.h
    pod5_format/read_table_writer_utils.cpp
    pod5_format/read_table_writer_utils.h
    pod5_format/read_table_utils.cpp
    pod5_format/read_table_utils.h

    pod5_format/run_info_table_reader.cpp
    pod5_format/run_info_table_reader.h
    pod5_format/run_info_table_schema.cpp
    pod5_format/run_info_table_schema.h
    pod5_format/run_info_table_writer.cpp
    pod5_format/run_info_table_writer.h

    pod5_format/signal_compression.cpp
    pod5_format/signal_compression.h
    pod5_format/signal_table_reader.cpp
    pod5_format/signal_table_reader.h
    pod5_format/signal_table_schema.cpp
    pod5_format/signal_table_schema.h
    pod5_format/signal_table_writer.cpp
    pod5_format/signal_table_writer.h
    pod5_format/signal_table_utils.h

    pod5_format/c_api.cpp
    pod5_format/c_api.h

    pod5_format/errors.cpp
    pod5_format/errors.h
    pod5_format/expandable_buffer.h
    pod5_format/result.h
    pod5_format/schema_utils.cpp
    pod5_format/schema_utils.h
    pod5_format/table_reader.cpp
    pod5_format/table_reader.h
    pod5_format/thread_pool.cpp
    pod5_format/thread_pool.h
    pod5_format/tuple_utils.h
    pod5_format/types.cpp
    pod5_format/types.h

    pod5_format/migration/migration.cpp
    pod5_format/migration/migration.h
    pod5_format/migration/migration_utils.h
    pod5_format/migration/v0_to_v1.cpp
    pod5_format/migration/v1_to_v2.cpp
    pod5_format/migration/v2_to_v3.cpp

    pod5_format/internal/async_output_stream.h
    pod5_format/internal/combined_file_utils.h

    pod5_format/svb16/common.hpp
    pod5_format/svb16/decode.hpp
    pod5_format/svb16/decode_scalar.hpp
    pod5_format/svb16/decode_x64.hpp
    pod5_format/svb16/encode.hpp
    pod5_format/svb16/encode_scalar.hpp
    pod5_format/svb16/encode_x64.hpp
    pod5_format/svb16/intrinsics.hpp
    pod5_format/svb16/shuffle_tables.hpp
    pod5_format/svb16/simd_detect_x64.hpp

    pod5_format/pgnano/pgnano.cpp
    pod5_format/pgnano/pgnano.h
    pod5_format/pgnano/compressor.cpp
    pod5_format/pgnano/compressor.h
    pod5_format/pgnano/metadata.h
    pod5_format/pgnano/header.h
    pod5_format/pgnano/decompressor.cpp
    pod5_format/pgnano/decompressor.h
    pod5_format/pgnano/pore_type_server.h
    pod5_format/pgnano/pore_type_server.cpp

    pod5_format/pgnano/pgnano_reader_state.h
    pod5_format/pgnano/pgnano_reader_state.cpp
    pod5_format/pgnano/pgnano_writer_state.h
    pod5_format/pgnano/pgnano_writer_state.cpp
)

set(public_headers)
list(APPEND public_headers
    pod5_format/file_writer.h
    pod5_format/file_reader.h

    pod5_format/schema_metadata.h

    pod5_format/read_table_reader.h
    pod5_format/read_table_schema.h
    pod5_format/read_table_writer.h
    pod5_format/read_table_writer_utils.h
    pod5_format/read_table_utils.h

    pod5_format/run_info_table_writer.h
    pod5_format/run_info_table_reader.h
    pod5_format/run_info_table_schema.h

    pod5_format/signal_compression.h
    pod5_format/signal_table_reader.h
    pod5_format/signal_table_schema.h
    pod5_format/signal_table_writer.h
    pod5_format/signal_table_utils.h

    pod5_format/c_api.h

    pod5_format/errors.h
    pod5_format/expandable_buffer.h
    pod5_format/result.h
    pod5_format/dictionary_writer.h
    pod5_format/schema_field_builder.h
    pod5_format/schema_utils.h
    pod5_format/table_reader.h
    pod5_format/thread_pool.h
    pod5_format/tuple_utils.h
    pod5_format/types.h

    ${CMAKE_CURRENT_BINARY_DIR}/pod5_format/pod5_format_export.h
)

set_target_properties(pod5_format_fork
    PROPERTIES
        POSITION_INDEPENDENT_CODE 1
        CXX_STANDARD 14
        PUBLIC_HEADER "${public_headers}"
)

target_link_libraries(pod5_format_fork
    PUBLIC
        arrow::arrow
        flatbuffers::flatbuffers
        Boost::headers
    PRIVATE
        pod5_flatbuffers_fork
        zstd::zstd
        Threads::Threads
        ${HTSLIB_LIB_DIR}/libhts.so
        GSL::gsl
        GSL::gslcblas
        pgnanoCommonLib
        fqzcomp
)

if (NOT BUILD_SHARED_LIB AND INSTALL_THIRD_PARTY)
    set(pod5_libs arrow zstd jemalloc)
    foreach (lib ${pod5_libs})

        install(
            FILES $<IF:$<CONFIG:Debug>,${${lib}_LIBS_DEBUG},${${lib}_LIBS_RELEASE}>
            TYPE LIB
            COMPONENT third_party
        )
    endforeach()
endif()

target_include_directories(pod5_format_fork
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

flatbuffers_generate_headers(
    TARGET pod5_flatbuffers_fork
    SCHEMAS
        pod5_format/flatbuffers/footer.fbs
    INCLUDE_PREFIX ""
    FLAGS --cpp
)

if (NOT MSVC)
    target_compile_options(pod5_format_fork PRIVATE -Werror -Wall -Wno-comment)
endif()
generate_export_header(pod5_format_fork BASE_NAME pod5_format EXPORT_FILE_NAME pod5_format/pod5_format_export.h)

install(
    TARGETS pod5_format_fork
    PUBLIC_HEADER DESTINATION "include/pod5_format"
)

if (POD5_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if (NOT POD5_DISABLE_TESTS)
    add_subdirectory(test)
endif()

if (BUILD_PYTHON_WHEEL)
    add_subdirectory(pod5_format_pybind)
endif()
get_target_property(COMPILE_OPTIONS pod5_format_fork COMPILE_OPTIONS)
message("Compile options for target pod5_format: ${COMPILE_OPTIONS}")
get_directory_property(COMPILE_OPTIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_OPTIONS)
message("Global compile options: ${COMPILE_OPTIONS}")