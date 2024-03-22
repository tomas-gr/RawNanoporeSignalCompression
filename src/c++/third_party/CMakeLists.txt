add_subdirectory(fqzcomp)

set(BUILD_PYTHON_WHEEL OFF CACHE BOOL "Build a python wheel for pod5" FORCE)
set(POD5_DISABLE_TESTS OFF CACHE BOOL "Disable building all tests" FORCE)
set(POD5_BUILD_EXAMPLES OFF CACHE BOOL "Enable building all examples" FORCE)
set(ENABLE_POD5_PACKAGING OFF CACHE BOOL "Enable packaging support" FORCE)
add_subdirectory(pod5)
add_subdirectory(TurboPFor-Integer-Compression)