/*  DB-WEAVE - TShiftState bit-flag helper
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Minimal stand-in for the original VCL TShiftState (a
    Set<TShiftStateEnum>). Exposes the flags the cursor + kbd/mouse
    dispatch actually consults. */

#ifndef DBWEAVE_COMPAT_SHIFT_COMPAT_H
#define DBWEAVE_COMPAT_SHIFT_COMPAT_H

enum VclShiftFlag {
    ssShift = 1,
    ssCtrl = 2,
    ssAlt = 4,
    ssLeft = 8,
    ssRight = 16,
    ssMiddle = 32,
    ssDouble = 64
};

struct TShiftState {
    int bits = 0;

    TShiftState() = default;
    TShiftState(int _b)
        : bits(_b)
    {
    }

    bool Contains(int _flag) const
    {
        return (bits & _flag) != 0;
    }
};

#endif /* DBWEAVE_COMPAT_SHIFT_COMPAT_H */
