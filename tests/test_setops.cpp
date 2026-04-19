/*  DB-WEAVE Qt 6 port - state-apply operation tests.
    Exercises the Set* / DoSet* operations and their helpers
    (range calc, empty predicates, free shaft/tritt allocation).
    Rendering side-effects are stubbed; tests only check data state.
*/

#include <QApplication>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"

class TestSetops : public QObject
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

    /*  ---- range / emptiness helpers --------------------------- */

    void is_empty_kette_reads_einzug_and_gewebe()
    {
        QCOMPARE(DBWFRM->IsEmptyKette(0), true);
        DBWFRM->einzug.feld.Set(0, (short)3);
        QCOMPARE(DBWFRM->IsEmptyKette(0), false);
    }

    void is_empty_schuss_reads_isempty_and_gewebe()
    {
        QCOMPARE(DBWFRM->IsEmptySchuss(4), true);
        DBWFRM->trittfolge.isempty.Set(4, false);
        QCOMPARE(DBWFRM->IsEmptySchuss(4), false);
    }

    void calc_range_kette_finds_occupied_span()
    {
        DBWFRM->einzug.feld.Set(3, (short)2);
        DBWFRM->einzug.feld.Set(7, (short)4);
        DBWFRM->CalcRangeKette();
        QCOMPARE(DBWFRM->kette.a, 3);
        QCOMPARE(DBWFRM->kette.b, 7);
    }

    void calc_range_schuesse_finds_occupied_span()
    {
        DBWFRM->trittfolge.isempty.Set(2, false);
        DBWFRM->trittfolge.isempty.Set(6, false);
        DBWFRM->CalcRangeSchuesse();
        QCOMPARE(DBWFRM->schuesse.a, 2);
        QCOMPARE(DBWFRM->schuesse.b, 6);
    }

    void update_range_grow_and_shrink()
    {
        /*  Grow on set */
        DBWFRM->kette = SZ(-1, -1);
        DBWFRM->schuesse = SZ(-1, -1);
        DBWFRM->UpdateRange(/*i=*/5, /*j=*/-1, /*set=*/true);
        QCOMPARE(DBWFRM->kette.a, 5);
        QCOMPARE(DBWFRM->kette.b, 5);
    }

    /*  ---- set ops -------------------------------------------- */

    void set_blatteinzug_toggles_cell_and_snapshots()
    {
        const bool before = DBWFRM->blatteinzug.feld.Get(3);
        DBWFRM->SetBlatteinzug(3);
        QCOMPARE(bool(DBWFRM->blatteinzug.feld.Get(3)), !before);
        /*  A second call toggles back. */
        DBWFRM->SetBlatteinzug(3);
        QCOMPARE(bool(DBWFRM->blatteinzug.feld.Get(3)), before);
    }

    void set_kettfarben_writes_data_color()
    {
        Data->color = (unsigned char)42;
        DBWFRM->SetKettfarben(5);
        QCOMPARE((int)DBWFRM->kettfarben.feld.Get(5), 42);
    }

    void set_schussfarben_writes_data_color()
    {
        Data->color = (unsigned char)77;
        DBWFRM->SetSchussfarben(9);
        QCOMPARE((int)DBWFRM->schussfarben.feld.Get(9), 77);
    }

    void set_einzug_sets_shaft_and_clears_column()
    {
        /*  Pre-mark gewebe column 2. SetEinzug should wipe it
            (DoSetEinzug zeros gewebe[i,*] before rebuilding from
            aufknuepfung*trittfolge, which are empty here).         */
        DBWFRM->gewebe.feld.Set(2, 4, (char)9);
        DBWFRM->SetEinzug(/*_i=*/2, /*_j=*/3); /* shaft = scroll_y1+j+1 = 4 */
        QCOMPARE(DBWFRM->einzug.feld.Get(2), (short)4);
        QCOMPARE(DBWFRM->gewebe.feld.Get(2, 4), (char)0);
    }

    void set_einzug_same_j_toggles_off()
    {
        DBWFRM->SetEinzug(1, 2);
        QCOMPARE(DBWFRM->einzug.feld.Get(1), (short)3); /* set */
        DBWFRM->SetEinzug(1, 2);
        QCOMPARE(DBWFRM->einzug.feld.Get(1), (short)0); /* toggled off */
    }

    void set_aufknuepfung_with_empty_tritt_just_marks_cell()
    {
        /*  With no einzug AND no trittfolge mark at column 3,
            the gewebe recompute bails out (IsEmptySchaft ||
            IsEmptyTritt), and the aufknuepfung cell simply stays
            set at _range (here 5). */
        DBWFRM->SetAufknuepfung(/*_i=*/3, /*_j=*/2, /*_set=*/true, /*_range=*/5);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(3, 2), (char)5);
    }

    void set_aufknuepfung_toggle_off()
    {
        DBWFRM->aufknuepfung.feld.Set(2, 1, (char)3);
        DBWFRM->SetAufknuepfung(2, 1, /*_set=*/false, /*_range=*/0);
        /*  ToggleAufknuepfung on positive oldstate -> -oldstate.
            (scroll_x2=0, scroll_y1=0, so cell (2,1) is addressed.)
            freieschaefte is all-true so IsEmptySchaft(1)=true -> the
            recompute branch returns early.                     */
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(2, 1), (char)-3);
    }

    void set_trittfolge_pegplan_sets_cell()
    {
        DBWFRM->ViewSchlagpatrone->setChecked(true);
        DBWFRM->SetTrittfolge(/*_i=*/4, /*_j=*/5, /*_set=*/true, /*_range=*/7);
        QCOMPARE(DBWFRM->trittfolge.feld.Get(4, 5), (char)7);
        QCOMPARE(DBWFRM->freietritte[4], false);
    }

    void set_trittfolge_non_pegplan_einzeltritt_clears_row()
    {
        /*  ViewSchlagpatrone unchecked + einzeltritt default=true ->
            "einzeltritt" branch: clear the row, then set one cell. */
        DBWFRM->trittfolge.feld.Set(0, 2, (char)1); /* old mark */
        DBWFRM->trittfolge.feld.Set(3, 2, (char)1); /* another */
        DBWFRM->SetTrittfolge(/*_i=*/7, /*_j=*/2, /*_set=*/false, /*_range=*/0);
        /*  Old marks cleared; only column 7 is set. */
        QCOMPARE(DBWFRM->trittfolge.feld.Get(0, 2), (char)0);
        QCOMPARE(DBWFRM->trittfolge.feld.Get(3, 2), (char)0);
        QCOMPARE(DBWFRM->trittfolge.feld.Get(7, 2), (char)1);
    }

    void set_gewebe_basic_toggle_and_range_update()
    {
        /*  Fresh cell -> toggle on sets to currentrange (default 1). */
        DBWFRM->kette = SZ(-1, -1);
        DBWFRM->schuesse = SZ(-1, -1);
        DBWFRM->SetGewebe(/*_i=*/4, /*_j=*/5, /*_set=*/false, /*_range=*/0);
        QCOMPARE(DBWFRM->gewebe.feld.Get(4, 5), (char)1);
        /*  Range expanded to (4, 5). */
        QCOMPARE(DBWFRM->kette.a, 4);
        QCOMPARE(DBWFRM->kette.b, 4);
        QCOMPARE(DBWFRM->schuesse.a, 5);
        QCOMPARE(DBWFRM->schuesse.b, 5);
    }

    void get_free_einzug_allocates_low_index()
    {
        QCOMPARE(DBWFRM->GetFreeEinzug(), (short)1);
        DBWFRM->freieschaefte[0] = false;
        QCOMPARE(DBWFRM->GetFreeEinzug(), (short)2);
    }

    void get_free_tritt_allocates_low_index()
    {
        QCOMPARE(DBWFRM->GetFreeTritt(), (short)0);
        DBWFRM->freietritte[0] = false;
        QCOMPARE(DBWFRM->GetFreeTritt(), (short)1);
    }

    void copy_tritt_duplicates_column_across_all_rows()
    {
        for (int i = 0; i < 5; i++)
            DBWFRM->trittfolge.feld.Set(i, 3, (char)(i + 1));
        DBWFRM->CopyTritt(/*von=*/3, /*nach=*/7);
        for (int i = 0; i < 5; i++)
            QCOMPARE(DBWFRM->trittfolge.feld.Get(i, 7), (char)(i + 1));
    }

    void toggle_gewebe_flips_sign()
    {
        DBWFRM->currentrange = 1;
        DBWFRM->gewebe.feld.Set(2, 2, (char)0);
        DBWFRM->ToggleGewebe(2, 2);
        QCOMPARE(DBWFRM->gewebe.feld.Get(2, 2), (char)1);
        DBWFRM->ToggleGewebe(2, 2);
        QCOMPARE(DBWFRM->gewebe.feld.Get(2, 2), (char)-1);
    }

    void recalc_trittfolge_empty_flips_isempty()
    {
        DBWFRM->trittfolge.feld.Set(3, 4, (char)1);
        DBWFRM->RecalcTrittfolgeEmpty(4);
        QCOMPARE(DBWFRM->trittfolge.isempty.Get(4), false);
        DBWFRM->trittfolge.feld.Set(3, 4, (char)0);
        DBWFRM->RecalcTrittfolgeEmpty(4);
        QCOMPARE(DBWFRM->trittfolge.isempty.Get(4), true);
    }

    void kettfaden_equal_compares_columns()
    {
        for (int j = 0; j < Data->MAXY2; j++) {
            DBWFRM->gewebe.feld.Set(1, j, (char)(j % 3 ? 2 : 0));
            DBWFRM->gewebe.feld.Set(2, j, (char)(j % 3 ? 2 : 0));
        }
        QCOMPARE(DBWFRM->KettfadenEqual(1, 2), true);
        DBWFRM->gewebe.feld.Set(2, 4, (char)9);
        QCOMPARE(DBWFRM->KettfadenEqual(1, 2), false);
    }
};

QTEST_MAIN(TestSetops)
#include "test_setops.moc"
