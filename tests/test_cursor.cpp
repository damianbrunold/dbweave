/*  DB-WEAVE Qt 6 port - cursor state-management tests */

#include <QApplication>
#include <QTest>

#include "cursor.h"
#include "datamodule.h"
#include "mainwindow.h"

class TestCursor : public QObject
{
    Q_OBJECT

    /*  Because CrFeld uses MAXX = fb.pos.width / fb.gw as the movement
        bound, tests that want to exercise MoveCursor* must first seed
        each field's pos.width / pos.height / gw / gh so MAXX > 0. The
        helper below sets a generous 50-cell grid with 10x10 cell size. */
    static void sizeField(FeldBase& fb, int cells_x, int cells_y)
    {
        fb.gw = 10;
        fb.gh = 10;
        fb.pos.x0 = 0;
        fb.pos.y0 = 0;
        fb.pos.width = cells_x * fb.gw;
        fb.pos.height = cells_y * fb.gh;
    }

    CrCursorHandler* h = nullptr;

private slots:

    void init()
    {
        Data = new TData();
        DBWFRM = new TDBWFRM();
        /*  Seed each field with a visible grid so IsVisible() and
            movement bounds are non-degenerate. */
        sizeField(DBWFRM->gewebe, 50, 50);
        sizeField(DBWFRM->einzug, 50, 10);
        sizeField(DBWFRM->aufknuepfung, 10, 10);
        sizeField(DBWFRM->trittfolge, 10, 50);
        sizeField(DBWFRM->kettfarben, 50, 1);
        sizeField(DBWFRM->schussfarben, 1, 50);
        sizeField(DBWFRM->blatteinzug, 50, 1);
        h = CrCursorHandler::CreateInstance(DBWFRM, Data);
        QVERIFY(h != nullptr);
    }

    void cleanup()
    {
        CrCursorHandler::Release(h);
        h = nullptr;
        delete DBWFRM;
        DBWFRM = nullptr;
        delete Data;
        Data = nullptr;
    }

    void cursor_starts_in_gewebe()
    {
        QCOMPARE(h->CurrentFeld(), GEWEBE);
    }

    void set_cursor_updates_field_and_position()
    {
        h->SetCursor(EINZUG, 7, 2);
        QCOMPARE(h->CurrentFeld(), EINZUG);
        QCOMPARE(DBWFRM->einzug.kbd.i, 7);
        QCOMPARE(DBWFRM->einzug.kbd.j, 2);
        QCOMPARE(DBWFRM->kbd_field, EINZUG);
    }

    void move_right_then_left_returns()
    {
        h->SetCursor(GEWEBE, 0, 0);
        h->MoveCursorRight(5, /*_select=*/false);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 5);
        h->MoveCursorLeft(3, false);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 2);
    }

    void move_up_down()
    {
        h->SetCursor(GEWEBE, 0, 0);
        h->MoveCursorUp(4, false);
        QCOMPARE(DBWFRM->gewebe.kbd.j, 4);
        h->MoveCursorDown(2, false);
        QCOMPARE(DBWFRM->gewebe.kbd.j, 2);
        /*  Down below zero clamps to zero. */
        h->MoveCursorDown(10, false);
        QCOMPARE(DBWFRM->gewebe.kbd.j, 0);
    }

    void move_right_clamps_to_maxx_minus_one()
    {
        h->SetCursor(GEWEBE, 48, 0);
        h->MoveCursorRight(100, false);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 49); /* MAXX = 50 -> cap at 49 */
    }

    void righttoleft_flips_left_right_for_gewebe()
    {
        DBWFRM->righttoleft = true;
        h->SetCursor(GEWEBE, 10, 0);
        /*  With righttoleft active, MoveCursorRight on GEWEBE should
            actually decrease the i coordinate. */
        h->MoveCursorRight(3, false);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 7);
        h->MoveCursorLeft(2, false);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 9);
        DBWFRM->righttoleft = false;
    }

    void toptobottom_flips_up_down_for_einzug()
    {
        DBWFRM->toptobottom = true;
        h->SetCursor(EINZUG, 0, 5);
        h->MoveCursorUp(2, false);
        QCOMPARE(DBWFRM->einzug.kbd.j, 3);
        DBWFRM->toptobottom = false;
    }

    void goto_next_field_cycles_through_visible()
    {
        h->SetCursor(GEWEBE, 0, 0);
        /*  Hide everything except GEWEBE and EINZUG by zeroing width. */
        DBWFRM->aufknuepfung.pos.width = 0;
        DBWFRM->trittfolge.pos.width = 0;
        DBWFRM->kettfarben.pos.width = 0;
        DBWFRM->schussfarben.pos.width = 0;
        DBWFRM->blatteinzug.pos.width = 0;
        h->GotoNextField();
        QCOMPARE(h->CurrentFeld(), EINZUG);
        h->GotoNextField();
        QCOMPARE(h->CurrentFeld(), GEWEBE);
    }

    void shared_coord_synchronises_x_across_warp_fields()
    {
        /*  GEWEBE, EINZUG, KETTFARBEN, BLATTEINZUG all share x1. */
        h->SetCursor(GEWEBE, 12, 3);
        QCOMPARE(DBWFRM->gewebe.kbd.i, 12);
        h->GotoField(EINZUG);
        QCOMPARE(DBWFRM->einzug.kbd.i, 12);
    }

    void cursor_direction_applies_to_all_fields()
    {
        h->SetCursorDirection(CD_DOWN);
        QCOMPARE(h->GetCursorDirection(), (CURSORDIRECTION)CD_DOWN);
    }

    void locked_mode_snaps_to_rapport()
    {
        DBWFRM->rapport.kr = SZ(10, 19); /* 10-wide rapport from 10..19 */
        DBWFRM->rapport.sr = SZ(0, 9);
        h->SetCursor(GEWEBE, 0, 0);
        DBWFRM->gewebe.kbd.i = 5;
        DBWFRM->gewebe.kbd.j = 3;
        h->SetCursorLocked(true);
        /*  CheckLocked reprojects i: with scroll_x1=0, rapport=[10,19],
            rx=10, x0=10, i=5 -> ii=5-10=-5 -> ii+=10=5 -> newi=5%10+10=15,
            clamped to scroll_x1 (0) -> stays 15.                          */
        QCOMPARE(DBWFRM->gewebe.kbd.i, 15);
    }
};

QTEST_MAIN(TestCursor)
#include "test_cursor.moc"
