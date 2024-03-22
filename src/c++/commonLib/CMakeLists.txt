include(${PROJECT_SOURCE_DIR}/../../CMakeMacros.txt)
set_pgnano_compile_options()

add_library(
            pgnanoCommonLib 
            codecs.h
            codecs.cpp
            default_histogram.h
            known_pore_types.h
            concepts.h
            default_histogram.h
            kmer.h
            known_pore_types.h
            level_table.cpp
            level_table.h
            macros.h
            math_utils.h
            mv_tag.h
            model.h
            model.cpp
            pore_type_parser.h
            pore_type_parser.cpp
            BAM_enriched_read.h
            BAM_enriched_read.cpp
            linear_LMS.h
           )
target_link_libraries(pgnanoCommonLib
    ${HTSLIB_LIB_DIR}/libhts.so)
include_directories(${HTSLIB_INCLUDE_DIRS})
get_target_property(COMPILE_OPTIONS pgnanoCommonLib COMPILE_OPTIONS)
message("Compile options for target commonLib: ${COMPILE_OPTIONS}")
get_directory_property(COMPILE_OPTIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_OPTIONS)
message("Global compile options: ${COMPILE_OPTIONS}")