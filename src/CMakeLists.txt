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
        dbweave_ui
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

# Phase 11 optional: serial-loom controllers. Requires Qt6SerialPort
# and is only compiled in when -DDBWEAVE_BUILD_LOOM=ON. When enabled,
# the ui lib also picks up the SerialPort dependency and a DBWEAVE_HAVE_LOOM
# define so LoomDialog can fan out to the real serial controllers;
# without it only the dummy is available.
if(DBWEAVE_BUILD_LOOM)
    target_link_libraries(dbweave_ui PUBLIC Qt6::SerialPort)
    target_compile_definitions(dbweave_ui PUBLIC DBWEAVE_HAVE_LOOM=1)
    # dbweave_ui's loomdialog.cpp #includes serialcontrollers.h from
    # src/loom when the feature is on; the static lib resolves at
    # link time.
    target_include_directories(dbweave_ui PUBLIC ${CMAKE_SOURCE_DIR}/src/loom)
    add_subdirectory(loom)
    target_link_libraries(dbweave_ui PUBLIC dbweave_loom)
    target_link_libraries(dbweave    PRIVATE dbweave_loom)
endif()
