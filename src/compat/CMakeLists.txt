# Small residual helper layer, header-only. What remains:
#   - colors_compat.h  TColor <-> QColor helpers (BBGGRR byte order;
#                      used by rangecolors + palette, matches the .dbw
#                      on-disk encoding).
#   - shift_compat.h   Bit-flag stand-in for the historical TShiftState;
#                      bridges Qt::KeyboardModifiers into the cursor
#                      dispatch in cursor.{h,cpp} + kbdhandling.cpp.
#   - assert_compat.h  dbw3_assert wrapping Q_ASSERT / qDebug.

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
