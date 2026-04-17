/*  DB-WEAVE Qt 6 port - first rendering slice tests.

    Exercises PatternCanvas + TDBWFRM::DrawGewebe by rendering the
    canvas widget to an off-screen QImage and checking pixel colours
    at known positions. Antialiasing is off in PaintCell/DrawGewebe
    so the comparisons are stable. */

#include <QApplication>
#include <QImage>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"
#include "patterncanvas.h"
#include "rangecolors.h"
#include "colors_compat.h"

class TestRendering : public QObject
{
	Q_OBJECT

	static constexpr int CELL = 10;   /* each gewebe cell is 10x10 px */
	static constexpr int COLS = 8;
	static constexpr int ROWS = 8;
	static constexpr int W    = COLS * CELL;
	static constexpr int H    = ROWS * CELL;

	QImage renderCanvas()
	{
		/*  The gewebe grid occupies the full canvas area. */
		DBWFRM->gewebe.pos.x0     = 0;
		DBWFRM->gewebe.pos.y0     = 0;
		DBWFRM->gewebe.pos.width  = W;
		DBWFRM->gewebe.pos.height = H;
		DBWFRM->gewebe.gw = CELL;
		DBWFRM->gewebe.gh = CELL;

		DBWFRM->pattern_canvas->resize(W, H);

		QImage img(W, H, QImage::Format_ARGB32);
		img.fill(Qt::magenta);          /* any colour != expected */
		DBWFRM->pattern_canvas->render(&img);
		return img;
	}

	static QColor rangeQC (int _r)
	{
		return qColorFromTColor(GetRangeColor(_r));
	}

private slots:

	void init()
	{
		Data   = new TData();
		DBWFRM = new TDBWFRM();
		DBWFRM->kette    = SZ(0, COLS - 1);
		DBWFRM->schuesse = SZ(0, ROWS - 1);

		/*  Mark every warp and weft as "non-empty" so DrawGewebe
		    doesn't early-return with a blank cell. */
		for (int i = 0; i < COLS; i++) DBWFRM->einzug.feld.Set(i, (short)1);
		for (int j = 0; j < ROWS; j++) DBWFRM->trittfolge.isempty.Set(j, false);
	}

	void cleanup()
	{
		delete DBWFRM; DBWFRM = nullptr;
		delete Data;   Data   = nullptr;
	}

	void empty_gewebe_renders_button_face_background()
	{
		/*  All cells zero => DrawGewebe hits the "empty / negative"
		    branch and paints bkground. The whole image should be
		    flat button-face colour (except the 1-px border of each
		    cell that the legacy intentionally leaves).            */
		QImage img = renderCanvas();

		const QColor bkg = DBWFRM->pattern_canvas->palette().color(QPalette::Button);
		/*  Interior pixel of cell (3, 3): local y = rows - 3 - 1 = 4
		    -> y_px = 4*CELL = 40. The interior is at (30+5, 40+5). */
		QCOMPARE(img.pixelColor(35, 45), bkg);
	}

	void gewebe_cell_with_range_is_painted_in_range_color()
	{
		/*  Set gewebe(2, 3) = range 1 (i.e. AUSHEBUNG's neighbour;
		    range table index 0 = black). */
		DBWFRM->gewebe.feld.Set(2, 3, (char)1);

		QImage img = renderCanvas();

		/*  Cell (2, 3) in local coords: i=2, j=3. x_px = 2*10 = 20.
		    Local j becomes: y_px = height - (3+1)*10 = 80 - 40 = 40.
		    The cell occupies [20..30) x [40..50). Its interior
		    pixel lies at (25, 45).                              */
		QCOMPARE(img.pixelColor(25, 45), rangeQC(1));
	}

	void gewebe_multiple_ranges_use_different_colors()
	{
		DBWFRM->gewebe.feld.Set(0, 0, (char)1);
		DBWFRM->gewebe.feld.Set(1, 1, (char)2);
		DBWFRM->gewebe.feld.Set(2, 2, (char)3);

		QImage img = renderCanvas();

		/*  Cell (0, 0): x_px=0, y_px=H-10=70 -> interior (5, 75).
		    Cell (1, 1): x_px=10, y_px=60 -> interior (15, 65).
		    Cell (2, 2): x_px=20, y_px=50 -> interior (25, 55). */
		QCOMPARE(img.pixelColor(5,  75), rangeQC(1));
		QCOMPARE(img.pixelColor(15, 65), rangeQC(2));
		QCOMPARE(img.pixelColor(25, 55), rangeQC(3));

		/*  Cell (5, 5) is untouched -> bkground. */
		const QColor bkg = DBWFRM->pattern_canvas->palette().color(QPalette::Button);
		QCOMPARE(img.pixelColor(55, 25), bkg);
	}

	void empty_einzug_bypasses_cell_color()
	{
		/*  Warp 4 has no einzug -> IsEmptyEinzug -> cell blanks
		    regardless of gewebe value. */
		DBWFRM->einzug.feld.Set(4, (short)0);
		DBWFRM->gewebe.feld.Set(4, 4, (char)1);   /* would be black otherwise */

		QImage img = renderCanvas();

		const QColor bkg = DBWFRM->pattern_canvas->palette().color(QPalette::Button);
		/*  Cell (4, 4) interior pixel. */
		QCOMPARE(img.pixelColor(45, 35), bkg);
	}

	void pattern_canvas_is_central_widget()
	{
		QVERIFY(DBWFRM->centralWidget() == DBWFRM->pattern_canvas);
	}

	/*  -------- Einzug / Aufknuepfung / Trittfolge rendering --------
	    These tests share a single large canvas that holds all four
	    fields stacked diagonally. Each field is 4 cells wide/tall
	    with 10x10 cells; offsets are fixed so coordinates are easy
	    to reason about. */

	QImage renderMultiField()
	{
		/*  Layout:
		      einzug       @ (0,   0)  W=40 H=40
		      aufknuepfung @ (60,  0)  W=40 H=40
		      trittfolge   @ (60, 60)  W=40 H=40
		      gewebe       @ (0,  60)  W=40 H=40
		    Total canvas 100x100 px.                                 */
		const int CCELL = 10, CN = 4;

		auto lay = [&](FeldBase& fb, int x0, int y0) {
			fb.pos.x0 = x0; fb.pos.y0 = y0;
			fb.pos.width = CN*CCELL; fb.pos.height = CN*CCELL;
			fb.gw = CCELL; fb.gh = CCELL;
		};
		lay(DBWFRM->einzug,       0,  0);
		lay(DBWFRM->aufknuepfung, 60, 0);
		lay(DBWFRM->trittfolge,   60, 60);
		lay(DBWFRM->gewebe,       0,  60);

		DBWFRM->pattern_canvas->resize(100, 100);
		QImage img(100, 100, QImage::Format_ARGB32);
		img.fill(Qt::magenta);
		DBWFRM->pattern_canvas->render(&img);
		return img;
	}

	void einzug_draws_shaft_at_einzug_feld_position()
	{
		/*  einzug[1] = 3  means warp 1 is on shaft 3 (1-based). With
		    a 4x4 grid the cell lit is (i=1, j=2). Local coords to
		    widget pixels: x = 1*10 = 10, y = H - (j+1)*10 = 40-30 = 10.
		    Cell interior pixel at (15, 15).                         */
		DBWFRM->einzug.feld.Set(1, (short)3);

		QImage img = renderMultiField();

		/*  einzug.darstellung defaults to STRICH which paints two
		    vertical center lines. At least one of the two centre
		    pixels should be black.                                  */
		QRgb p = img.pixel(15, 15);
		QVERIFY2(p != 0xFFFFFFFF && p != qRgb(240, 240, 240),
		         "Expected non-background pixel in einzug cell interior");
	}

	void aufknuepfung_paints_range_color_for_positive_cell()
	{
		/*  aufknuepfung.darstellung defaults to KREUZ (an X).
		    Set cell (1, 2) = range 2. Cell position inside
		    aufknuepfung field (offset (60,0)): x = 60 + 1*10 = 70,
		    y = 40 - (2+1)*10 = 10. Interior pixel at (75, 15) --
		    the KREUZ only lights a few diagonal pixels, so we check
		    that *some* pixel in the cell is the range-2 colour.   */
		DBWFRM->aufknuepfung.feld.Set(1, 2, (char)2);

		QImage img = renderMultiField();

		const QColor rc2 = qColorFromTColor(GetRangeColor(2));
		bool seen = false;
		for (int y = 0; y < 10 && !seen; y++)
			for (int x = 0; x < 10 && !seen; x++)
				if (img.pixelColor(70 + x, 10 + y) == rc2) seen = true;
		QVERIFY2(seen, "Range-2 aufknuepfung cell did not paint range colour");
	}

	void trittfolge_non_pegplan_uses_default_darstellung()
	{
		/*  Set trittfolge[2, 1] = 1. In non-pegplan view it paints
		    trittfolge.darstellung (PUNKT default) in black.         */
		DBWFRM->ViewSchlagpatrone->setChecked(false);
		DBWFRM->trittfolge.feld.Set(2, 1, (char)1);

		QImage img = renderMultiField();

		/*  Field offset (60, 60), cell (2, 1): x = 60 + 2*10 = 80,
		    y = 60 + 40 - 20 = 80. PUNKT paints a 2-px dash at centre
		    (x+5..x+7, y+5..y+6). Check a pixel inside that. */
		QCOMPARE(img.pixel(85, 85), qRgb(0, 0, 0));
	}
};

QTEST_MAIN(TestRendering)
#include "test_rendering.moc"
