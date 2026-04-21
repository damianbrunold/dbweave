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

# Serial-loom controllers. On by default; pulls in Qt6SerialPort and
# defines DBWEAVE_HAVE_LOOM so TSTRGFRM fans out to the real serial
# controllers in src/loom/serialcontrollers.cpp. Disable with
# -DDBWEAVE_NO_LOOM=ON on hosts without Qt6SerialPort; in that
# configuration TSTRGFRM falls back to the built-in dummy controller
# and the loom sub-library isn't compiled.
if(NOT DBWEAVE_NO_LOOM)
    target_link_libraries(dbweave_ui PUBLIC Qt6::SerialPort)
    target_compile_definitions(dbweave_ui PUBLIC DBWEAVE_HAVE_LOOM=1)
    target_include_directories(dbweave_ui PUBLIC ${CMAKE_SOURCE_DIR}/src/loom)
    add_subdirectory(loom)
    target_link_libraries(dbweave_ui PUBLIC dbweave_loom)
    target_link_libraries(dbweave    PRIVATE dbweave_loom)
endif()
