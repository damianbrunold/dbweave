# Residual helper layer after Stage 10 shim removal. What remains:
#   - colors_compat.h  TColor <-> QColor helpers (VCL BBGGRR byte
#                      order; used by rangecolors + palette).
#   - shift_compat.h   Bit-flag stand-in for VCL TShiftState; used by
#                      cursor.{h,cpp} + kbdhandling.cpp to bridge
#                      Qt::KeyboardModifiers into the cursor dispatch.
#   - assert_compat.h  dbw3_assert wrapping Q_ASSERT / qDebug.
# Header-only INTERFACE library. AnsiString / TStringList / TRegistry
# / TBitmap / __fastcall / __published were dropped in Stage 10 once
# every call site had been rewritten against direct Qt types.

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
