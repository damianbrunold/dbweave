/*  DB-WEAVE Qt 6 port - lancée and threading wizard regression tests.

    Lancée: after an incomplete last run of original threads the
    spreading copied thread b + 1 in place of the inserted empty one
    (visible as a duplicated thread with a selection).
    Threading wizard: the falling straight draft added the first warp
    number to the shafts.
*/

#include <QApplication>
#include <QTest>

#include "datamodule.h"
#include "einzugassistentdialog.h"
#include "mainwindow.h"

class TestLancierung : public QObject
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

    void warp_lancee_inserts_after_an_incomplete_last_run()
    {
        /*  Warps 0..3 on shafts 1..4, warp i woven on pick i. */
        for (int i = 0; i < 4; i++) {
            DBWFRM->einzug.feld.Set(i, (short)(i + 1));
            DBWFRM->gewebe.feld.Set(i, i, (char)1);
        }
        DBWFRM->kette = SZ(0, 3);
        DBWFRM->schuesse = SZ(0, 3);
        /*  Select warps 0..2; ratio 2:1 leaves warp 2 as an incomplete
            last run. */
        DBWFRM->selection.begin = PT(0, 0);
        DBWFRM->selection.end = PT(2, 3);
        DBWFRM->selection.feld = GEWEBE;
        const int vv[6] = { 2, 1, 0, 0, 0, 0 };
        DBWFRM->KettLancierung(vv);

        const short expected[7] = { 1, 2, 0, 3, 0, 4, 0 };
        for (int i = 0; i < 7; i++)
            QCOMPARE(DBWFRM->einzug.feld.Get(i), expected[i]);
        /*  Warp 3 (pick 3) moved to position 5 and exists only once. */
        QCOMPARE(DBWFRM->gewebe.feld.Get(5, 3), (char)1);
        QCOMPARE(DBWFRM->gewebe.feld.Get(4, 3), (char)0);
    }

    void weft_lancee_inserts_after_an_incomplete_last_run()
    {
        /*  Pick j weaves warp j. */
        for (int j = 0; j < 4; j++) {
            DBWFRM->einzug.feld.Set(j, (short)(j + 1));
            DBWFRM->gewebe.feld.Set(j, j, (char)1);
        }
        DBWFRM->kette = SZ(0, 3);
        DBWFRM->schuesse = SZ(0, 3);
        DBWFRM->selection.begin = PT(0, 0);
        DBWFRM->selection.end = PT(3, 2);
        DBWFRM->selection.feld = GEWEBE;
        const int vv[6] = { 2, 1, 0, 0, 0, 0 };
        DBWFRM->SchussLancierung(vv);

        /*  Picks: warp 0, warp 1, empty, warp 2, empty, warp 3. */
        const int warpOfPick[6] = { 0, 1, -1, 2, -1, 3 };
        for (int j = 0; j < 6; j++) {
            for (int i = 0; i < 4; i++) {
                const char want = (warpOfPick[j] == i) ? 1 : 0;
                QCOMPARE(DBWFRM->gewebe.feld.Get(i, j), want);
            }
        }
    }

    void wizard_falling_straight_from_warp_3()
    {
        EinzugassistentDialog dlg(DBWFRM);
        dlg.createGerade(/*firstKettfaden=*/3, /*firstSchaft=*/2,
                         /*schaefte=*/3, /*steigend=*/false);
        QCOMPARE(DBWFRM->einzug.feld.Get(1), (short)0);
        QCOMPARE(DBWFRM->einzug.feld.Get(2), (short)4);
        QCOMPARE(DBWFRM->einzug.feld.Get(3), (short)3);
        QCOMPARE(DBWFRM->einzug.feld.Get(4), (short)2);
        QCOMPARE(DBWFRM->einzug.feld.Get(5), (short)0);
    }

    void wizard_rising_straight_from_warp_3()
    {
        EinzugassistentDialog dlg(DBWFRM);
        dlg.createGerade(3, 2, 3, true);
        QCOMPARE(DBWFRM->einzug.feld.Get(2), (short)2);
        QCOMPARE(DBWFRM->einzug.feld.Get(3), (short)3);
        QCOMPARE(DBWFRM->einzug.feld.Get(4), (short)4);
    }
};

QTEST_MAIN(TestLancierung)
#include "test_lancierung.moc"
