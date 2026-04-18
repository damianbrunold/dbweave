# Phase 4-5: rendering primitives + document-window skeleton. Links
# against Qt6::Widgets because TDBWFRM inherits QMainWindow, but is
# still consumed by headless tests via the static-lib boundary.

add_library(dbweave_ui STATIC
    draw_cell.cpp
    datamodule.cpp
    mainwindow.cpp
    init.cpp
    clear.cpp
    language.cpp
    undoredo.cpp
    cursor.cpp
    rapport.cpp
    einzug.cpp
    trittfolge.cpp
    aufknuepfung.cpp
    rapportieren.cpp
    range.cpp
    helpers.cpp
    setblatteinzug.cpp
    setfarbe.cpp
    seteinzug.cpp
    setaufknuepfung.cpp
    settrittfolge.cpp
    setgewebe.cpp
    patterncanvas.cpp
    draw.cpp
    fileload.cpp
    filesave.cpp
    filehandling.cpp
    hilfslinien.cpp
    mousehandling.cpp
    kbdhandling.cpp
    selection.cpp
    statusbar.cpp
    edit.cpp
    popupmenu.cpp
    utilities.cpp
    steigung.cpp
)

target_link_libraries(dbweave_ui PUBLIC dbweave_io)

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
