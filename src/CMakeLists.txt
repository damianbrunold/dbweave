configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_BINARY_DIR}/generated/dbweave/version.h"
    @ONLY
)

set(_dbweave_platform_sources "")
if(WIN32)
    # Embed the application icon into dbweave.exe so Explorer, the
    # taskbar and the Start menu pick it up. Both MSVC's rc.exe and
    # MinGW's windres compile .rc files automatically when listed
    # as a target source.
    list(APPEND _dbweave_platform_sources
        "${CMAKE_SOURCE_DIR}/resources/windows/dbweave.rc")
endif()

qt_add_executable(dbweave
    main.cpp
    ${_dbweave_platform_sources}
)

target_include_directories(dbweave PRIVATE
    "${CMAKE_BINARY_DIR}/generated/dbweave"
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

add_subdirectory(compat)

# File-format I/O primitives. felddef's Read/Write methods depend on the
# FfReader/FfWriter defined here, so this module must be built before any
# domain code that persists to .dbw files.
add_subdirectory(io)

add_subdirectory(domain)
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
