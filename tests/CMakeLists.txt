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
