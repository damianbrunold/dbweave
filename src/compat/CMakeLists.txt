# Compatibility shim making legacy VCL-style source compile under Qt 6.
# This module is header-only and is deleted in Phase 12.
#
# It is deliberately incomplete: drawing code (TCanvas, TBitmap at the
# pixel level) is NOT shimmed. Those call sites must be rewritten against
# QPainter / QImage during the per-module port.

add_library(dbweave_compat INTERFACE)

target_include_directories(dbweave_compat
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbweave_compat
    INTERFACE
        Qt6::Core
        Qt6::Gui
)
