# Phase 2: pure-domain logic. No UI dependencies; links against QtCore
# (for QString/QSettings via the compat layer) but never QtWidgets.

add_library(dbweave_domain STATIC
    colors.cpp
    felddef.cpp
    palette.cpp
    rangecolors.cpp
    settings.cpp
    zentralsymm.cpp
)

target_include_directories(dbweave_domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbweave_domain
    PUBLIC
        Qt6::Core
        dbweave_compat
        dbweave_io
)
