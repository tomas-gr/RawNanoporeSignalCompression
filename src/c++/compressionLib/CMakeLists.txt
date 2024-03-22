include(${PROJECT_SOURCE_DIR}/../../CMakeMacros.txt)
set_pgnano_compile_options()

add_library(
            pgnanoCompressionLib 
            compressor.cpp
            compressor.h
            decompressor.cpp
            decompressor.h
            predictors.h
           )

target_link_libraries(pgnanoCompressionLib 
                    pgnanoCommonLib
                    Boost::headers
                    ${HTSLIB_LIB_DIR}/libhts.so)
include_directories(${HTSLIB_INCLUDE_DIRS})