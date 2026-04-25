/*  DB-WEAVE Qt 6 port - rapport algorithm tests.

    Exercises CalcRapport / IsInRapport / EinzugEqual / TrittfolgeEqual
    against a populated TDBWFRM. Rendering paths (ClearRapport,
    DrawRapport, DrawDifferences) are stubs and are not tested here.
*/

#include <QApplication>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"
#include "rapport.h"

class TestRapport : public QObject
{
    Q_OBJECT

private slots:

    void init()
    {
        Data = new TData();
        DBWFRM = new TDBWFRM();
    }

    void cleanup()
    {
        delete DBWFRM;
        DBWFRM = nullptr;
        delete Data;
        Data = nullptr;
    }

    void is_in_rapport_checks_both_axes()
    {
        DBWFRM->rapport.kr = SZ(5, 9);
        DBWFRM->rapport.sr = SZ(10, 14);
        QCOMPARE(DBWFRM->IsInRapport(7, 12), true);
        QCOMPARE(DBWFRM->IsInRapport(4, 12), false); /* outside kr */
        QCOMPARE(DBWFRM->IsInRapport(7, 9), false);  /* outside sr */
    }

    void calc_rapport_finds_simple_warp_repeat()
    {
        /*  kette = [0, 7]. Set einzug to repeating pattern 1,2,3,4.
            Expect kettrapport = [0, 3]. */
        DBWFRM->kette = SZ(0, 7);
        DBWFRM->schuesse = SZ(0, 0); /* no weft data, any gewebe ok */
        for (int i = 0; i < 8; i++)
            DBWFRM->einzug.feld.Set(i, (short)((i % 4) + 1));
        /*  clear gewebe so EinzugEqual's schuss-comparison is always
            equal (rows of zeros). */
        for (int i = 0; i < 8; i++)
            DBWFRM->gewebe.feld.Set(i, 0, (char)0);

        DBWFRM->CalcRapport();

        QCOMPARE(DBWFRM->rapport.kr.a, 0);
        QCOMPARE(DBWFRM->rapport.kr.b, 3);
    }

    void calc_rapport_empty_kette_yields_empty_kr()
    {
        DBWFRM->kette = SZ(-1, -1);
        DBWFRM->CalcRapport();
        QCOMPARE(DBWFRM->rapport.kr.a, 0);
        QCOMPARE(DBWFRM->rapport.kr.b, -1);
    }

    void calc_rapport_finds_full_weft_repeat()
    {
        /*  schuesse = [0, 5]. trittfolge column-major: pattern
            repeats every 3. */
        DBWFRM->kette = SZ(0, 0);
        DBWFRM->schuesse = SZ(0, 5);

        /*  trittfolge.feld is Data->MAXX2 rows x MAXY2 cols; use
            column 0 as the active treadle. */
        for (int j = 0; j < 6; j++) {
            char v = (char)((j % 3) + 1);
            DBWFRM->trittfolge.feld.Set(0, j, v);
        }
        DBWFRM->CalcRapport();

        QCOMPARE(DBWFRM->rapport.sr.a, 0);
        QCOMPARE(DBWFRM->rapport.sr.b, 2);
    }

    void calc_rapport_entire_range_when_not_repeating()
    {
        DBWFRM->kette = SZ(0, 4);
        DBWFRM->schuesse = SZ(0, 0);
        /*  Strictly increasing einzug -- no repeat at all. */
        for (int i = 0; i < 5; i++)
            DBWFRM->einzug.feld.Set(i, (short)(i + 1));
        DBWFRM->CalcRapport();
        QCOMPARE(DBWFRM->rapport.kr.a, 0);
        QCOMPARE(DBWFRM->rapport.kr.b, 4);
    }
};

QTEST_MAIN(TestRapport)
#include "test_rapport.moc"
