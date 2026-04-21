/*  DB-WEAVE - TColor <-> QColor helpers
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  TColor is a 32-bit integer in 0x00BBGGRR byte order (low byte red,
    then green, then blue, then an unused byte). This matches the .dbw
    on-disk encoding but differs from QRgb (0x00RRGGBB), so we keep
    explicit conversion helpers to avoid silent byte-order bugs.
*/

#ifndef DBWEAVE_COMPAT_COLORS_COMPAT_H
#define DBWEAVE_COMPAT_COLORS_COMPAT_H

#include <QColor>
#include <cstdint>

using TColor = std::int32_t;

/*  Win32 COLORREF has the same 0x00BBGGRR byte layout as VCL TColor but
    is unsigned. Code that serialises palettes to disk relies on this
    binary layout (little-endian uint32 = R,G,B,0 on x86). */
using COLORREF = std::uint32_t;
using BYTE = unsigned char;

#ifndef RGB
#define RGB(r, g, b) \
    ((COLORREF)(((BYTE)(r)) | ((COLORREF)((BYTE)(g)) << 8) | ((COLORREF)((BYTE)(b)) << 16)))
#endif
#ifndef GetRValue
#define GetRValue(c) ((BYTE)((c) & 0xFF))
#endif
#ifndef GetGValue
#define GetGValue(c) ((BYTE)(((c) >> 8) & 0xFF))
#endif
#ifndef GetBValue
#define GetBValue(c) ((BYTE)(((c) >> 16) & 0xFF))
#endif

inline QColor qColorFromTColor(TColor _c)
{
    const int r = (_c) & 0xFF;
    const int g = (_c >> 8) & 0xFF;
    const int b = (_c >> 16) & 0xFF;
    return QColor(r, g, b);
}

inline TColor tColorFromQColor(const QColor& _c)
{
    return static_cast<TColor>((static_cast<std::int32_t>(_c.blue()) << 16)
                               | (static_cast<std::int32_t>(_c.green()) << 8)
                               | static_cast<std::int32_t>(_c.red()));
}

inline TColor tColorFromRGB(int _r, int _g, int _b)
{
    return static_cast<TColor>((_b << 16) | (_g << 8) | _r);
}

#endif /* DBWEAVE_COMPAT_COLORS_COMPAT_H */
