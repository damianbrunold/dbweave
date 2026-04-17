# Each sub-module will become its own static library as code is ported.
# During Phase 0, only the top-level executable with a placeholder main.cpp
# exists. Sub-directories (domain, io, ui, print, loom, compat) are added
# by later phases.

qt_add_executable(dbweave
    main.cpp
)

target_link_libraries(dbweave
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::PrintSupport
)

set_target_properties(dbweave PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE    ON
)

# Phase 1: VCL -> Qt compatibility shim.
add_subdirectory(compat)

# Phase 2 (early): file-format I/O primitives. felddef's Read/Write
# methods depend on the FfReader/FfWriter defined here, so this module
# must be built before any domain code that persists to .dbw files.
add_subdirectory(io)

# Phase 2: pure-domain logic.
add_subdirectory(domain)

# Phase 4: rendering primitives (no widget dependency yet).
add_subdirectory(ui)

# Placeholder targets for sub-modules — uncomment as each phase lands.
# add_subdirectory(ui)
# add_subdirectory(print)
# if(DBWEAVE_BUILD_LOOM)
#     add_subdirectory(loom)
# endif()
