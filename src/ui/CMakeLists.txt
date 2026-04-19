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
    palettepanel.cpp
    recalc.cpp
    insert.cpp
    delete.cpp
    move.cpp
    insertbindung.cpp
    rapportdialog.cpp
    cursordirdialog.cpp
    cursorgotodialog.cpp
    entervvdialog.cpp
    loadpartsdialog.cpp
    aboutdialog.cpp
    techinfodialog.cpp
    envoptionsdialog.cpp
    xoptionsdialog.cpp
    properties.cpp
    propertiesdialog.cpp
    entwurfsinfodialog.cpp
    einzugassistentdialog.cpp
    fixeinzugdialog.cpp
    farbverlaufdialog.cpp
    blockmuster.cpp
    blockmusterdialog.cpp
    choosecolordialog.cpp
    selcolordialog.cpp
    userdef.cpp
    userdefdialogs.cpp
    importwif.cpp
    importbmpdialog.cpp
    exportwif.cpp
    exportbitmap.cpp
    bereiche.cpp
    setcolors.cpp
    tools.cpp
    print.cpp
    printinit.cpp
    printextent.cpp
    printdraw.cpp
    printprint.cpp
    pagesetupdialog.cpp
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
        Qt6::PrintSupport
        dbweave_compat
        dbweave_domain
)
