/*  DB-WEAVE Qt 6 port - VCL TShiftState compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Minimal stand-in for VCL's TShiftState (a Set<TShiftStateEnum>).
    The legacy code only exercises .Contains(ssShift) and .Contains(ssCtrl)
    so we expose those as bit flags; the full enum (ssAlt, ssLeft, etc.)
    isn't needed until mouse handling lands in a later slice. */

#ifndef DBWEAVE_COMPAT_SHIFT_COMPAT_H
#define DBWEAVE_COMPAT_SHIFT_COMPAT_H

enum VclShiftFlag {
	ssShift = 1,
	ssCtrl  = 2,
	ssAlt   = 4,
	ssLeft  = 8,
	ssRight = 16,
	ssMiddle = 32,
	ssDouble = 64
};

struct TShiftState
{
	int bits = 0;

	TShiftState() = default;
	TShiftState(int _b) : bits(_b) {}

	bool Contains (int _flag) const { return (bits & _flag) != 0; }
};

#endif /* DBWEAVE_COMPAT_SHIFT_COMPAT_H */
