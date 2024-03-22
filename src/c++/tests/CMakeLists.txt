add_executable(compression_decompression_test compression_decompression.cpp)

target_compile_options(compression_decompression_test PRIVATE -pg -g)
target_compile_options(compression_decompression_test PRIVATE -Werror -Wall -Wno-comment)
target_link_options(compression_decompression_test PRIVATE -pg -g)

# Add pod5_format library
target_link_libraries(compression_decompression_test PUBLIC pod5_format_fork)
include_directories("../../../pod5/third_party/include")
include_directories(${HTSLIB_INCLUDE_DIRS})

target_include_directories(compression_decompression_test PUBLIC ${HTSLIB_INCLUDE_DIRS})
#target_link_directories(BAMHandler PUBLIC ${HTSLIB_LIB_DIR})

add_executable(standalone_compression_decompression_test standalone_compression_decompression_test.cpp)
target_link_libraries(standalone_compression_decompression_test pgnanoCompressionLib)
target_compile_options(standalone_compression_decompression_test PRIVATE -g)