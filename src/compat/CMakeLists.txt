# Residual compatibility layer after Phase 12 cleanup. What remains:
#   - vcl_compat.h     AnsiString / String typedef aliases for QString.
#   - colors_compat.h  TColor <-> QColor helpers with VCL BBGGRR byte order.
#   - registry_compat.h  TRegistry wrapper over QSettings (used by
#                      domain/settings).
#   - tbitmap_compat.h  TBitmap wrapper over QImage (used by the bitmap
#                      import path).
#   - shift_compat.h   VCL TShiftState stand-in (used by cursor.cpp).
#   - assert_compat.h  dbw3_assert mapped to Q_ASSERT.
# Header-only INTERFACE library; __fastcall / __published / True /
# False / tstringlist_compat.h were removed in Phase 12 because no
# port code still references them.

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
