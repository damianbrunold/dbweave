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
