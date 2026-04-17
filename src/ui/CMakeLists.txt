# Phase 4-5: rendering primitives + document-window skeleton. Links
# against Qt6::Widgets because TDBWFRM inherits QMainWindow, but is
# still consumed by headless tests via the static-lib boundary.

add_library(dbweave_ui STATIC
    draw_cell.cpp
    datamodule.cpp
    mainwindow.cpp
    init.cpp
    clear.cpp
)

target_include_directories(dbweave_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbweave_ui
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        dbweave_compat
        dbweave_domain
)
