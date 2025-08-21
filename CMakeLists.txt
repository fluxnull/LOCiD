cmake_minimum_required(VERSION 3.10)

project(locid C)

include_directories(include)

set(LIB_SOURCES
    src/globals.c
    src/hashing.c
    src/loci.c
    src/stability.c
    src/library.c
    src/device.c
    lib/blake3.c
    lib/blake3_dispatch.c
    lib/blake3_portable.c
    lib/blake3_avx2.c
    lib/blake3_avx512.c
    lib/blake3_sse2.c
    lib/blake3_sse41.c
)

add_library(locid_lib STATIC ${LIB_SOURCES})

add_executable(locid src/main.c)
target_link_libraries(locid locid_lib)

# Set compile flags for the SIMD-specific source files.
set_source_files_properties(lib/blake3_sse2.c PROPERTIES COMPILE_FLAGS "-msse2")
set_source_files_properties(lib/blake3_sse41.c PROPERTIES COMPILE_FLAGS "-msse4.1")
set_source_files_properties(lib/blake3_avx2.c PROPERTIES COMPILE_FLAGS "-mavx2")
set_source_files_properties(lib/blake3_avx512.c PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512vl")
