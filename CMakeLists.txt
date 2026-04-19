cmake_minimum_required(VERSION 3.21)
project(dbweave
    VERSION 0.1.0
    DESCRIPTION "DB-WEAVE textile CAD/CAM (Qt 6 port)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/ui")

option(DBWEAVE_BUILD_TESTS "Build unit tests" ON)
option(DBWEAVE_BUILD_LOOM  "Build loom/serial-port support (requires Qt6SerialPort)" OFF)

# Qt 6.5 LTS minimum.
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
if(DBWEAVE_BUILD_LOOM)
    find_package(Qt6 6.5 REQUIRED COMPONENTS SerialPort)
endif()
if(DBWEAVE_BUILD_TESTS)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    enable_testing()
endif()

qt_standard_project_setup()

add_subdirectory(src)

if(DBWEAVE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Phase 10: install rules + per-platform bundle glue used by the packaging
# scripts under packaging/. Running `cmake --install build --prefix <dir>`
# lays out a relocatable tree that linuxdeploy / windeployqt / macdeployqt
# can consume.
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

if(APPLE)
    # On macOS the executable is a bundle (MACOSX_BUNDLE ON); install it as
    # Contents/MacOS/dbweave inside the .app, and wire in an Info.plist
    # template so macdeployqt picks up the bundle identifier and version.
    set_target_properties(dbweave PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST       "${CMAKE_SOURCE_DIR}/resources/macos/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME      "DB-WEAVE"
        MACOSX_BUNDLE_GUI_IDENTIFIER   "ch.brunoldsoftware.dbweave"
        MACOSX_BUNDLE_BUNDLE_VERSION   "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE        "dbweave.icns"
    )
    # Drop the docs + icon inside the .app so macdeployqt carries them into
    # the .dmg automatically.
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/macos/dbweave.icns")
        target_sources(dbweave PRIVATE
            "${CMAKE_SOURCE_DIR}/resources/macos/dbweave.icns")
        set_source_files_properties(
            "${CMAKE_SOURCE_DIR}/resources/macos/dbweave.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()
    install(TARGETS dbweave BUNDLE DESTINATION .)
elseif(WIN32)
    install(TARGETS dbweave RUNTIME DESTINATION .)
else()
    # Linux: use the FHS layout. packaging/linux/build_appimage.sh inverts
    # this into an AppDir via linuxdeploy.
    install(TARGETS dbweave RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES resources/linux/dbweave.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES resources/dbweave.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()

# Generic files shipped on every platform.
install(FILES LICENSE dbw_manual.pdf dbw_handbuch.pdf
        DESTINATION ${CMAKE_INSTALL_DOCDIR} OPTIONAL)
