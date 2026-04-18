/*  DB-WEAVE Qt 6 port - input handler tests.
    Exercises Physical2Logical hit-testing, left-click-to-paint via
    handleCanvasMousePress, and arrow / digit / space key dispatch
    via handleCanvasKeyPress. */

#include <QApplication>
#include <Qt>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"
#include "patterncanvas.h"
#include "cursor.h"

class TestInput : public QObject
{
	Q_OBJECT

	/*  Lay out a small canvas manually so pixel-to-cell math is
	    predictable. Gewebe at (0,0) 80x80 px, 10 px cells, 8x8
	    cells. Einzug 80x40 above it. Aufknuepfung & trittfolge
	    are zeroed (kept out of the hit-test path). */
	static constexpr int CELL = 10;
	static constexpr int GEW_W = 80;
	static constexpr int GEW_H = 80;

	void layoutFields()
	{
		DBWFRM->gewebe.gw = CELL; DBWFRM->gewebe.gh = CELL;
		DBWFRM->gewebe.pos.x0 = 0;
		DBWFRM->gewebe.pos.y0 = 50;
		DBWFRM->gewebe.pos.width  = GEW_W;
		DBWFRM->gewebe.pos.height = GEW_H;

		DBWFRM->einzug.gw = CELL; DBWFRM->einzug.gh = CELL;
		DBWFRM->einzug.pos.x0 = 0;
		DBWFRM->einzug.pos.y0 = 0;
		DBWFRM->einzug.pos.width  = GEW_W;
		DBWFRM->einzug.pos.height = 40;

		/*  Park the other fields off-canvas so HitCheck skips them. */
		DBWFRM->aufknuepfung.pos  = GRIDPOS();
		DBWFRM->trittfolge.pos    = GRIDPOS();
		DBWFRM->blatteinzug.pos   = GRIDPOS();
		DBWFRM->kettfarben.pos    = GRIDPOS();
		DBWFRM->schussfarben.pos  = GRIDPOS();
	}

private slots:

	void init()
	{
		Data   = new TData();
		DBWFRM = new TDBWFRM();
		DBWFRM->kette    = SZ(0, GEW_W/CELL - 1);
		DBWFRM->schuesse = SZ(0, GEW_H/CELL - 1);
		layoutFields();
	}

	void cleanup()
	{
		delete DBWFRM; DBWFRM = nullptr;
		delete Data;   Data   = nullptr;
	}

	void physical_to_logical_identifies_gewebe_cell()
	{
		FELD f; int i, j;
		/*  Gewebe origin y0=50, height=80, cell=10.
		    Cell (3, 5): x_px = 30..40, y_px = y0+height - (5+1)*10 = 70..80
		    (HitCheck uses strict <, pick interior pixel 35, 75). */
		DBWFRM->Physical2Logical(35, 75, f, i, j);
		QCOMPARE((int)f, (int)GEWEBE);
		QCOMPARE(i, 3);
		QCOMPARE(j, 5);
	}

	void physical_to_logical_misses_outside_return_invalid()
	{
		FELD f; int i, j;
		DBWFRM->Physical2Logical(500, 500, f, i, j);
		QCOMPARE((int)f, (int)INVALID);
	}

	void left_click_on_gewebe_starts_selection_at_cell()
	{
		/*  Legacy semantic: a click on the main grids starts a
		    rubber-band selection; it does not paint the cell. The
		    cell is painted via a subsequent Range1..9 digit press or
		    drag. The cursor follows the click regardless. */
		DBWFRM->currentrange = 3;
		DBWFRM->handleCanvasMousePress(25, 75, /*shift=*/false);
		QCOMPARE(DBWFRM->cursorhandler->CurrentFeld(), GEWEBE);
		QVERIFY(DBWFRM->selection.Valid());
		QCOMPARE(DBWFRM->selection.feld, GEWEBE);
		QCOMPARE(DBWFRM->selection.begin.i, 2);
		QCOMPARE(DBWFRM->selection.begin.j, 5);
		/*  Cell itself untouched by the click. */
		QCOMPARE((int)DBWFRM->gewebe.feld.Get(2, 5), 0);
	}

	void drag_on_gewebe_grows_selection()
	{
		DBWFRM->handleCanvasMousePress(25, 75, /*shift=*/false);
		/*  Drag to cell (4, 3): x=45, y = 50+80-(3+1)*10 = 90+5 = 95. */
		DBWFRM->handleCanvasMouseMove (45, 95, /*shift=*/false);
		QVERIFY(DBWFRM->selection.Valid());
		QCOMPARE(DBWFRM->selection.end.i, 4);
		QCOMPARE(DBWFRM->selection.end.j, 3);
	}

	void digit_fills_selection_with_range()
	{
		DBWFRM->handleCanvasMousePress(25, 75, /*shift=*/false);
		DBWFRM->handleCanvasMouseMove (45, 95, /*shift=*/false);
		DBWFRM->handleCanvasMouseRelease();
		DBWFRM->handleCanvasKeyPress(Qt::Key_4, 0);
		/*  Normalised selection spans i=2..4, j=3..5. Every cell
		    should now carry range 4.                          */
		for (int i = 2; i <= 4; i++)
			for (int j = 3; j <= 5; j++)
				QCOMPARE((int)DBWFRM->gewebe.feld.Get(i, j), 4);
	}

	void arrow_key_moves_cursor()
	{
		DBWFRM->cursorhandler->SetCursor(GEWEBE, 3, 3);
		DBWFRM->handleCanvasKeyPress(Qt::Key_Right, 0);
		QCOMPARE(DBWFRM->gewebe.kbd.i, 4);
		DBWFRM->handleCanvasKeyPress(Qt::Key_Up, 0);
		QCOMPARE(DBWFRM->gewebe.kbd.j, 4);
	}

	void digit_key_sets_currentrange()
	{
		DBWFRM->currentrange = 1;
		DBWFRM->handleCanvasKeyPress(Qt::Key_7, 0);
		QCOMPARE(DBWFRM->currentrange, 7);
	}

	void digit_zero_clears_cell_under_cursor()
	{
		DBWFRM->cursorhandler->SetCursor(GEWEBE, 2, 2);
		DBWFRM->gewebe.feld.Set(2, 2, (char)4);
		DBWFRM->handleCanvasKeyPress(Qt::Key_0, 0);
		QVERIFY((int)DBWFRM->gewebe.feld.Get(2, 2) <= 0);
	}
};

QTEST_MAIN(TestInput)
#include "test_input.moc"
