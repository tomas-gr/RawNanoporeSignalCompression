cmake_minimum_required(VERSION 3.16.0)

project(PGNano)

include(${PROJECT_SOURCE_DIR}/../../CMakeMacros.txt)
set_pgnano_compile_options()

find_package(Boost REQUIRED COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(tests)
add_subdirectory(commonLib)
add_subdirectory(compressionLib)
add_subdirectory(third_party)

include_directories(${HTSLIB_INCLUDE_DIRS})
add_executable(copy copy.cpp)
target_compile_options(copy PRIVATE -Werror -Wall -Wno-comment -Wextra)

add_executable(compress compress.cpp)
target_compile_options(compress PRIVATE -Werror -Wall -Wno-comment -Wextra)

add_executable(decompress decompress.cpp)
target_compile_options(decompress PRIVATE -Werror -Wall -Wno-comment -Wextra)
#get_target_property(COMPILE_OPTIONS copy COMPILE_OPTIONS)
#message("Compile options for target copy: ${COMPILE_OPTIONS}")
#get_directory_property(COMPILE_OPTIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_OPTIONS)
#message("Global compile options: ${COMPILE_OPTIONS}")


add_executable(benchmark benchmark.cpp)
target_compile_options(benchmark PRIVATE -Wall -Wno-comment -Wextra) #-Werror )

# Add pod5_format library
target_link_libraries(copy PUBLIC pgnanoCommonLib)
target_link_libraries(copy PUBLIC pod5_format_fork)
target_link_libraries(copy PUBLIC Boost::filesystem)

target_link_libraries(compress PUBLIC pgnanoCompressionLib pgnanoCommonLib pod5_format Boost::filesystem)
target_link_libraries(decompress PUBLIC pgnanoCompressionLib pgnanoCommonLib pod5_format Boost::filesystem)

find_package(GSL REQUIRED)

include(Findzstd.cmake)
find_package(zstd REQUIRED)
install(
    FILES ${zstd_LIBS_RELEASE}>
    TYPE LIB
    COMPONENT third_party
)

target_link_libraries(benchmark PUBLIC pgnanoCommonLib pgnanoCompressionLib pod5_format_fork GSL::gsl GSL::gslcblas)
target_link_libraries(benchmark PUBLIC ${TURBOPFOR_LIB_DIR}/libic.a)
target_link_libraries(benchmark PUBLIC zstd::zstd)
target_include_directories(benchmark PRIVATE "../../pod5/third_party/include")
#target_include_directories(BAMHandler PUBLIC ${HTSLIB_INCLUDE_DIRS})
#include_directories("/data/pgnanoraw/htslib/include/htslib")
#target_link_directories(BAMHandler PUBLIC ${HTSLIB_LIB_DIR})