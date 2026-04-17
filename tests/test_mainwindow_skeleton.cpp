/*  DB-WEAVE Qt 6 port - Phase 5 skeleton test.

    Verifies the foundation layer of the main-window reconstruction:
      - TData and TDBWFRM can be constructed and wired to the legacy
        global pointers (Data, DBWFRM);
      - each Feld* type constructs with the legacy DEFAULT_* dimensions;
      - Clear() invokes without crashing and seeds the expected values
        (the blatteinzug "zero every 4th thread" pattern being the
        easiest to spot-check).

    A QApplication is needed because TDBWFRM inherits QMainWindow.
    Nothing is shown on screen -- construction alone is sufficient. */

#include <QApplication>
#include <QTest>

#include "dbw3_base.h"
#include "datamodule.h"
#include "mainwindow.h"

class TestMainwindowSkeleton : public QObject
{
	Q_OBJECT

private slots:

	void init()
	{
		Data   = new TData();
		DBWFRM = new TDBWFRM();
	}

	void cleanup()
	{
		delete DBWFRM; DBWFRM = nullptr;
		delete Data;   Data   = nullptr;
	}

	void data_module_has_default_dimensions()
	{
		QCOMPARE((int)Data->MAXX1, DEFAULT_MAXX1);
		QCOMPARE((int)Data->MAXX2, DEFAULT_MAXX2);
		QCOMPARE((int)Data->MAXY1, DEFAULT_MAXY1);
		QCOMPARE((int)Data->MAXY2, DEFAULT_MAXY2);
		QVERIFY(Data->palette != nullptr);
	}

	void mainwindow_scroll_offsets_zero_initialised()
	{
		QCOMPARE(DBWFRM->scroll_x1, 0);
		QCOMPARE(DBWFRM->scroll_x2, 0);
		QCOMPARE(DBWFRM->scroll_y1, 0);
		QCOMPARE(DBWFRM->scroll_y2, 0);
	}

	void feld_blatteinzug_clear_seeds_default_pattern()
	{
		FeldBlatteinzug bz;
		QCOMPARE(bz.SizeX(), DEFAULT_MAXX1);
		QCOMPARE(bz.SizeY(), 1);
		bz.Clear();
		/*  Clear sets the blatteinzug to the default "straight draw"
		    pattern where every 4th thread pair is occupied. */
		QCOMPARE((int)bz.feld.Get(0), 0);
		QCOMPARE((int)bz.feld.Get(1), 0);
		QCOMPARE((int)bz.feld.Get(2), 1);  /* (3%4==0) -> 1 */
		QCOMPARE((int)bz.feld.Get(3), 1);
		QCOMPARE((int)bz.feld.Get(4), 0);
	}

	void feld_einzug_clear_zero_fills()
	{
		FeldEinzug ez;
		QCOMPARE(ez.SizeX(), DEFAULT_MAXX1);
		QCOMPARE((int)ez.maxy, DEFAULT_MAXY1);
		QCOMPARE(ez.darstellung, STRICH);
		for (int i = 0; i < 10; i++) ez.feld.Set(i, (short)(i + 1));
		ez.Clear();
		for (int i = 0; i < 10; i++)
			QCOMPARE(ez.feld.Get(i), (short)0);
	}

	void feld_trittfolge_clear_resets_isempty()
	{
		FeldTrittfolge tf;
		QCOMPARE(tf.einzeltritt, true);
		QCOMPARE(tf.darstellung, PUNKT);
		tf.isempty.Set(5, false);
		tf.feld.Set(0, 0, (char)7);
		tf.Clear();
		QCOMPARE(tf.isempty.Get(5), true);
		QCOMPARE(tf.feld.Get(0, 0), (char)0);
	}

	void feld_scrollx_reads_mainwindow_offsets()
	{
		FeldEinzug ez;
		FeldAufknuepfung ak;
		DBWFRM->scroll_x1 = 17;
		DBWFRM->scroll_x2 = 29;
		DBWFRM->scroll_y1 = 42;
		DBWFRM->scroll_y2 = 53;
		QCOMPARE(ez.ScrollX(), 17);
		QCOMPARE(ez.ScrollY(), 42);
		QCOMPARE(ak.ScrollX(), 29);
		QCOMPARE(ak.ScrollY(), 42);
	}

	void feld_aufknuepfung_defaults()
	{
		FeldAufknuepfung ak;
		QCOMPARE(ak.darstellung, KREUZ);
		QCOMPARE(ak.pegplanstyle, false);
		QCOMPARE(ak.SizeX(), DEFAULT_MAXX2);
		QCOMPARE(ak.SizeY(), DEFAULT_MAXY1);
	}

	void feld_gewebe_dimensions()
	{
		FeldGewebe g;
		QCOMPARE(g.SizeX(), DEFAULT_MAXX1);
		QCOMPARE(g.SizeY(), DEFAULT_MAXY2);
		g.feld.Set(5, 5, (char)9);
		g.Clear();
		QCOMPARE(g.feld.Get(5, 5), (char)0);
	}
};

QTEST_MAIN(TestMainwindowSkeleton)
#include "test_mainwindow_skeleton.moc"
