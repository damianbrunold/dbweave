# Serial-loom controllers. Built by default; skipped when
# -DDBWEAVE_NO_LOOM=ON so the rest of the tree still compiles on
# hosts without Qt6SerialPort.

add_library(dbweave_loom STATIC
    serialport.cpp
    serialcontrollers.cpp
)

target_include_directories(dbweave_loom
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/ui      # for loom.h (StWeaveController)
)

target_link_libraries(dbweave_loom
    PUBLIC
        Qt6::Core
        Qt6::SerialPort
        dbweave_compat
        dbweave_domain
        dbweave_ui
)

target_compile_definitions(dbweave_loom PUBLIC DBWEAVE_HAVE_LOOM=1)
