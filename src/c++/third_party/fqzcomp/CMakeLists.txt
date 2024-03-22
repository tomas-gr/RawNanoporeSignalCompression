add_library(fqzcomp INTERFACE
${FQZCOMP_SOURCE_DIR}/clr.h
${FQZCOMP_SOURCE_DIR}/simple_model.h)

set_target_properties(fqzcomp PROPERTIES CMAKE_CXX_STANDARD 11)
set_target_properties(fqzcomp PROPERTIES LINKER_LANGUAGE 11)
target_compile_options(fqzcomp INTERFACE -O3 -fstrict-aliasing -ffast-math -fomit-frame-pointer -Wall)#-march=native