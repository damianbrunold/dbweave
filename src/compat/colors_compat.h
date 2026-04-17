/*  DB-WEAVE Qt 6 port - VCL TColor compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  VCL encodes TColor as a 32-bit integer in 0x00BBGGRR byte order (the
    low-order byte is red, then green, then blue, then an unused byte).
    This differs from both QRgb (0x00RRGGBB) and Win32 COLORREF (which
    matches VCL). Provide explicit conversion helpers so legacy code that
    stores TColor values in the file format and in the palette can keep
    using the same representation without silent byte-order bugs.
*/

#ifndef DBWEAVE_COMPAT_COLORS_COMPAT_H
#define DBWEAVE_COMPAT_COLORS_COMPAT_H

#include <QColor>
#include <cstdint>

using TColor = std::int32_t;

inline QColor qColorFromTColor (TColor _c)
{
	const int r = (_c)       & 0xFF;
	const int g = (_c >>  8) & 0xFF;
	const int b = (_c >> 16) & 0xFF;
	return QColor(r, g, b);
}

inline TColor tColorFromQColor (const QColor& _c)
{
	return static_cast<TColor>(
		(static_cast<std::int32_t>(_c.blue())  << 16) |
		(static_cast<std::int32_t>(_c.green()) <<  8) |
		 static_cast<std::int32_t>(_c.red())
	);
}

inline TColor tColorFromRGB (int _r, int _g, int _b)
{
	return static_cast<TColor>((_b << 16) | (_g << 8) | _r);
}

#endif /* DBWEAVE_COMPAT_COLORS_COMPAT_H */
