qt_add_executable(test_smoke
    test_smoke.cpp
)

target_link_libraries(test_smoke
    PRIVATE
        Qt6::Core
        Qt6::Test
)

add_test(NAME smoke COMMAND test_smoke)

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

qt_add_executable(test_draw_cell
    test_draw_cell.cpp
)
target_link_libraries(test_draw_cell PRIVATE Qt6::Core Qt6::Gui Qt6::Test dbweave_ui)
add_test(NAME draw_cell COMMAND test_draw_cell)

qt_add_executable(test_mainwindow_skeleton
    test_mainwindow_skeleton.cpp
)
target_link_libraries(test_mainwindow_skeleton
    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME mainwindow_skeleton COMMAND test_mainwindow_skeleton)

qt_add_executable(test_language
    test_language.cpp
)
target_link_libraries(test_language PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME language COMMAND test_language)

qt_add_executable(test_undoredo
    test_undoredo.cpp
)
target_link_libraries(test_undoredo PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME undoredo COMMAND test_undoredo)

qt_add_executable(test_cursor
    test_cursor.cpp
)
target_link_libraries(test_cursor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME cursor COMMAND test_cursor)

qt_add_executable(test_rapport
    test_rapport.cpp
)
target_link_libraries(test_rapport PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME rapport COMMAND test_rapport)

qt_add_executable(test_einzug
    test_einzug.cpp
)
target_link_libraries(test_einzug PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME einzug COMMAND test_einzug)

qt_add_executable(test_trittfolge_ops
    test_trittfolge_ops.cpp
)
target_link_libraries(test_trittfolge_ops PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME trittfolge_ops COMMAND test_trittfolge_ops)

qt_add_executable(test_setops
    test_setops.cpp
)
target_link_libraries(test_setops PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME setops COMMAND test_setops)

qt_add_executable(test_rendering
    test_rendering.cpp
)
target_link_libraries(test_rendering PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME rendering COMMAND test_rendering)

qt_add_executable(test_fileload
    test_fileload.cpp
)
target_link_libraries(test_fileload PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME fileload_samples COMMAND test_fileload)

qt_add_executable(test_input
    test_input.cpp
)
target_link_libraries(test_input PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME input COMMAND test_input)

qt_add_executable(test_wif_roundtrip
    test_wif_roundtrip.cpp
)
target_link_libraries(test_wif_roundtrip PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test dbweave_ui)
add_test(NAME wif_roundtrip COMMAND test_wif_roundtrip)
