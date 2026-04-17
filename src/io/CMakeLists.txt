# Phase 2/3: file I/O. The "@dbw3:file" text-format reader/writer
# (FfBuffer, FfToken*, FfFile, FfBase, FfReader, FfWriter) lives here
# as a leaf module so that the domain layer can depend on it for its
# Read/Write persistence hooks.

add_library(dbweave_io STATIC
    fileformat.cpp
)

target_include_directories(dbweave_io
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbweave_io
    PUBLIC
        Qt6::Core
        dbweave_compat
)
