# Unit tests are added here as each module is ported.
# Phase 0 ships a single smoke test to prove the build pipeline works.

qt_add_executable(test_smoke
    test_smoke.cpp
)

target_link_libraries(test_smoke
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(NAME smoke COMMAND test_smoke)

# ---- Phase 1: compat shim tests -------------------------------------

qt_add_executable(test_compat
    test_compat.cpp
)

target_link_libraries(test_compat
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
        dbweave_compat
)

add_test(NAME compat COMMAND test_compat)

# ---- Phase 2: domain unit tests -------------------------------------

qt_add_executable(test_colors
    test_colors.cpp
)

target_link_libraries(test_colors
    PRIVATE
        Qt6::Core
        Qt6::Test
        dbweave_domain
)

add_test(NAME colors COMMAND test_colors)

qt_add_executable(test_fileformat
    test_fileformat.cpp
)

target_link_libraries(test_fileformat
    PRIVATE
        Qt6::Core
        Qt6::Test
        dbweave_io
)

add_test(NAME fileformat COMMAND test_fileformat)

qt_add_executable(test_felddef
    test_felddef.cpp
)

target_link_libraries(test_felddef
    PRIVATE
        Qt6::Core
        Qt6::Test
        dbweave_domain
)

add_test(NAME felddef COMMAND test_felddef)

qt_add_executable(test_zentralsymm
    test_zentralsymm.cpp
)
target_link_libraries(test_zentralsymm PRIVATE Qt6::Core Qt6::Test dbweave_domain)
add_test(NAME zentralsymm COMMAND test_zentralsymm)

qt_add_executable(test_palette
    test_palette.cpp
)
target_link_libraries(test_palette PRIVATE Qt6::Core Qt6::Test dbweave_domain)
add_test(NAME palette COMMAND test_palette)

qt_add_executable(test_settings
    test_settings.cpp
)
target_link_libraries(test_settings PRIVATE Qt6::Core Qt6::Test dbweave_domain)
add_test(NAME settings COMMAND test_settings)

# ---- Phase 4: rendering tests ---------------------------------------

qt_add_executable(test_draw_cell
    test_draw_cell.cpp
)
target_link_libraries(test_draw_cell PRIVATE Qt6::Core Qt6::Gui Qt6::Test dbweave_ui)
add_test(NAME draw_cell COMMAND test_draw_cell)
