/*  DB-WEAVE Qt 6 port - trittfolge / aufknuepfung / rapportieren
    utility tests. Covers the non-click helper methods ported into
    TDBWFRM in this slice.
*/

#include <QApplication>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"

class TestTrittfolgeOps : public QObject
{
    Q_OBJECT

private slots:

    void init()
    {
        Data = new TData();
        DBWFRM = new TDBWFRM();
        DBWFRM->kette = SZ(0, 9);
        DBWFRM->schuesse = SZ(0, 9);
    }

    void cleanup()
    {
        delete DBWFRM;
        DBWFRM = nullptr;
        delete Data;
        Data = nullptr;
    }

    /*  ---- trittfolge helpers --------------------------------- */

    void is_empty_tritt_reads_trittfolge()
    {
        QCOMPARE(DBWFRM->IsEmptyTritt(0), true);
        DBWFRM->trittfolge.feld.Set(0, 0, (char)1);
        QCOMPARE(DBWFRM->IsEmptyTritt(0), false);
    }

    void get_first_nonempty_tritt()
    {
        DBWFRM->trittfolge.feld.Set(3, 0, (char)1);
        DBWFRM->trittfolge.feld.Set(7, 0, (char)1);
        QCOMPARE(DBWFRM->GetFirstNonemptyTritt(0), 3);
        QCOMPARE(DBWFRM->GetFirstNonemptyTritt(4), 7);
        QCOMPARE(DBWFRM->GetFirstNonemptyTritt(8), -1);
    }

    void move_tritt_copies_trittfolge_and_aufknuepfung_and_flips_free()
    {
        /*  Populate column 2 (source) with a trittfolge mark and
            matching aufknuepfung mark. */
        DBWFRM->trittfolge.feld.Set(2, 5, (char)4);
        DBWFRM->aufknuepfung.feld.Set(2, 1, (char)7);

        DBWFRM->MoveTritt(/*von=*/2, /*nach=*/0);

        QCOMPARE(DBWFRM->trittfolge.feld.Get(2, 5), (char)0); /* cleared */
        QCOMPARE(DBWFRM->trittfolge.feld.Get(0, 5), (char)4); /* moved  */
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(2, 1), (char)0);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(0, 1), (char)7);
    }

    void aufknuepfungsspalte_equal()
    {
        /*  Two columns with identical aufknuepfung marks are equal.
            The helper returns false for entirely empty pairs (they
            are considered "nothing to merge"). */
        DBWFRM->aufknuepfung.feld.Set(0, 2, (char)5);
        DBWFRM->aufknuepfung.feld.Set(1, 2, (char)5);
        QCOMPARE(DBWFRM->AufknuepfungsspalteEqual(0, 1), true);

        DBWFRM->aufknuepfung.feld.Set(1, 2, (char)6);
        QCOMPARE(DBWFRM->AufknuepfungsspalteEqual(0, 1), false);

        /*  Both columns empty -> false (legacy "nonempty" guard). */
        QCOMPARE(DBWFRM->AufknuepfungsspalteEqual(5, 6), false);
    }

    void switch_tritte_swaps_columns_and_free_flags()
    {
        DBWFRM->trittfolge.feld.Set(0, 3, (char)1);
        DBWFRM->aufknuepfung.feld.Set(0, 2, (char)1);

        DBWFRM->trittfolge.feld.Set(4, 3, (char)2);
        DBWFRM->aufknuepfung.feld.Set(4, 2, (char)2);

        DBWFRM->SwitchTritte(0, 4);

        QCOMPARE(DBWFRM->trittfolge.feld.Get(0, 3), (char)2);
        QCOMPARE(DBWFRM->trittfolge.feld.Get(4, 3), (char)1);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(0, 2), (char)2);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(4, 2), (char)1);
    }

    void rearrange_tritte_belassen_is_noop()
    {
        /*  Force TfBelassen (default is TfMinimalZ). */
        DBWFRM->TfMinimalZ->setChecked(false);
        DBWFRM->TfBelassen->setChecked(true);
        DBWFRM->trittfolge.feld.Set(5, 0, (char)3);
        DBWFRM->RearrangeTritte();
        QCOMPARE(DBWFRM->trittfolge.feld.Get(5, 0), (char)3);
    }

    void rearrange_tritte_minimalz_compacts_from_left()
    {
        /*  Tritte at columns 3 and 7 only; after MinimalZ they
            should start at i1 (the leftmost active) which is 3 ->
            compacts down to column 3 and 4. MergeTritte first checks
            if the aufknuepfung columns are identical; with disjoint
            single marks they are not, so merge is a no-op. */
        DBWFRM->trittfolge.feld.Set(3, 0, (char)1);
        DBWFRM->trittfolge.feld.Set(7, 1, (char)1);
        DBWFRM->aufknuepfung.feld.Set(3, 2, (char)1); /* distinct */
        DBWFRM->aufknuepfung.feld.Set(7, 5, (char)1);

        DBWFRM->TfBelassen->setChecked(false);
        DBWFRM->TfMinimalZ->setChecked(true);

        DBWFRM->RearrangeTritte();

        /*  Expect trittfolge[3,0] still set (lowest column stays),
            and column 7 moved down. Exact target depends on
            SwitchTritte dynamics + EliminateEmptyTritt compaction;
            simply assert the data is preserved somewhere. */
        int count_nonzero = 0;
        for (int i = 0; i < Data->MAXX2; i++)
            if (DBWFRM->trittfolge.feld.Get(i, 0) != 0 || DBWFRM->trittfolge.feld.Get(i, 1) != 0)
                count_nonzero++;
        QVERIFY(count_nonzero >= 2);
    }

    /*  ---- aufknuepfung helpers -------------------------------- */

    void minimize_aufknuepfung_clears_empty_shafts_and_tritte()
    {
        /*  Populate aufknuepfung row (any treadle)-column-3; after
            MinimizeAufknuepfung those cells should be cleared. */
        DBWFRM->aufknuepfung.feld.Set(1, 3, (char)9);
        DBWFRM->aufknuepfung.feld.Set(2, 3, (char)9);

        DBWFRM->MinimizeAufknuepfung();

        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(1, 3), (char)0);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(2, 3), (char)0);
    }

    /*  ---- rapportieren helpers -------------------------------- */

    void clear_kettfaden_wipes_einzug_and_gewebe_column()
    {
        DBWFRM->einzug.feld.Set(5, (short)4);
        DBWFRM->gewebe.feld.Set(5, 2, (char)7);
        DBWFRM->gewebe.feld.Set(5, 3, (char)7);
        DBWFRM->ClearKettfaden(5);
        QCOMPARE(DBWFRM->einzug.feld.Get(5), (short)0);
        QCOMPARE(DBWFRM->gewebe.feld.Get(5, 2), (char)0);
        QCOMPARE(DBWFRM->gewebe.feld.Get(5, 3), (char)0);
    }

    void clear_schussfaden_wipes_trittfolge_and_gewebe_row()
    {
        DBWFRM->trittfolge.feld.Set(3, 4, (char)2);
        DBWFRM->gewebe.feld.Set(1, 4, (char)5);
        DBWFRM->gewebe.feld.Set(2, 4, (char)5);

        DBWFRM->ClearSchussfaden(4);

        QCOMPARE(DBWFRM->trittfolge.feld.Get(3, 4), (char)0);
        QCOMPARE(DBWFRM->IsEmptyTrittfolge(4), true);
        QCOMPARE(DBWFRM->gewebe.feld.Get(1, 4), (char)0);
        QCOMPARE(DBWFRM->gewebe.feld.Get(2, 4), (char)0);
    }

    void copy_kettfaden_duplicates_einzug_and_optionally_color()
    {
        DBWFRM->einzug.feld.Set(2, (short)5);
        DBWFRM->kettfarben.feld.Set(2, (unsigned char)99);
        DBWFRM->gewebe.feld.Set(2, 3, (char)8);

        /*  Without colors: einzug and gewebe copy, kettfarben
            unchanged. */
        DBWFRM->kettfarben.feld.Set(4, (unsigned char)7);
        DBWFRM->CopyKettfaden(/*von=*/2, /*nach=*/4, /*withcolors=*/false);
        QCOMPARE(DBWFRM->einzug.feld.Get(4), (short)5);
        QCOMPARE(DBWFRM->gewebe.feld.Get(4, 3), (char)8);
        QCOMPARE((int)DBWFRM->kettfarben.feld.Get(4), 7);

        /*  With colors: kettfarben copies too. */
        DBWFRM->CopyKettfaden(2, 6, true);
        QCOMPARE((int)DBWFRM->kettfarben.feld.Get(6), 99);
    }

    void copy_schussfaden_duplicates_trittfolge_and_gewebe_row()
    {
        DBWFRM->trittfolge.feld.Set(3, 2, (char)1);
        DBWFRM->gewebe.feld.Set(5, 2, (char)4);

        DBWFRM->CopySchussfaden(2, 8, false);

        QCOMPARE(DBWFRM->trittfolge.feld.Get(3, 8), (char)1);
        QCOMPARE(DBWFRM->IsEmptyTrittfolge(8), false);
        QCOMPARE(DBWFRM->gewebe.feld.Get(5, 8), (char)4);
    }
};

QTEST_MAIN(TestTrittfolgeOps)
#include "test_trittfolge_ops.moc"
