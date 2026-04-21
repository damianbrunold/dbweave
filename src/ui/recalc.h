/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#ifndef DBWEAVE_UI_RECALC_H
#define DBWEAVE_UI_RECALC_H


class TDBWFRM;
class TData;

/*  Full einzug / trittfolge / aufknuepfung rebuilder. Ported
    verbatim from legacy/recalc.cpp. Consumed by TDBWFRM wrappers
    (RecalcAll / RecalcSchlagpatrone / RecalcTrittfolgeAufknuepfung
    / RecalcFixEinzug) and by the bereiche / importbmp /
    insertbindung / rapportieren paths that mutate gewebe
    wholesale. */
class RcRecalcAll
{
protected:
    TData* data;
    TDBWFRM* frm;
    bool schlagpatrone;
    int k1, k2;
    int s1, s2;

public:
    RcRecalcAll(TDBWFRM* _frm, TData* _data, bool _schlagpatrone = false);

    void Recalc();
    void RecalcEinzugFixiert();
    void RecalcSchlagpatrone();
    void RecalcTrittfolgeAufknuepfung();
    void RecalcAufknuepfung();
    void CalcK();
    void CalcS();

protected:
    void RecalcEinzug();
    void RecalcTrittfolge();

    bool KettfadenEqual(int _a, int _b);
    bool SchussfadenEqual(int _a, int _b);

    bool KettfadenEmpty(int _a);
    bool SchussfadenEmpty(int _a);

    short GetSchaft(int _a);
};

#endif
