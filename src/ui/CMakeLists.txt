# Phase 4: rendering core. No widget dependency yet -- only QtGui so the
# library can be linked from headless tests that render into QImage.

add_library(dbweave_ui STATIC
    draw_cell.cpp
)

target_include_directories(dbweave_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbweave_ui
    PUBLIC
        Qt6::Core
        Qt6::Gui
        dbweave_compat
        dbweave_domain
)
