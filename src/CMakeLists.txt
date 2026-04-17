# Each sub-module will become its own static library as code is ported.
# During Phase 0, only the top-level executable with a placeholder main.cpp
# exists. Sub-directories (domain, io, ui, print, loom, compat) are added
# by later phases.

qt_add_executable(dbweave
    main.cpp
)

target_link_libraries(dbweave
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::PrintSupport
)

set_target_properties(dbweave PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE    ON
)

# Placeholder targets for sub-modules — uncomment as each phase lands.
# add_subdirectory(compat)
# add_subdirectory(domain)
# add_subdirectory(io)
# add_subdirectory(ui)
# add_subdirectory(print)
# if(DBWEAVE_BUILD_LOOM)
#     add_subdirectory(loom)
# endif()
