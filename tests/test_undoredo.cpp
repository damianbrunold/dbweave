/*  DB-WEAVE Qt 6 port - undo/redo ring tests */

#include <QApplication>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"
#include "undoredo.h"

class TestUndoredo : public QObject
{
	Q_OBJECT

private slots:

	void init()
	{
		Data   = new TData();
		DBWFRM = new TDBWFRM();   /* constructor creates DBWFRM->undo */
	}

	void cleanup()
	{
		delete DBWFRM; DBWFRM = nullptr;
		delete Data;   Data   = nullptr;
	}

	void fresh_undo_stack_has_nothing_to_undo_or_redo()
	{
		QVERIFY(DBWFRM->undo != nullptr);
		QCOMPARE(DBWFRM->undo->CanUndo(), false);
		QCOMPARE(DBWFRM->undo->CanRedo(), false);
	}

	void snapshot_then_modify_then_undo_restores_original()
	{
		/*  Undo snapshots the seven input fields (einzug,
		    aufknuepfung, trittfolge, isempty, schussfarben,
		    kettfarben, blatteinzug). The gewebe field is derived
		    via RecalcGewebe() on restore, so assertions target
		    einzug directly.                                   */
		DBWFRM->undo->Snapshot();                 /* state A (empty) */

		DBWFRM->einzug.feld.Set(3, (short)7);
		DBWFRM->undo->Snapshot();                 /* state B */

		DBWFRM->einzug.feld.Set(3, (short)9);
		DBWFRM->undo->Snapshot();                 /* state C */

		/*  Undo restores state B. */
		QVERIFY(DBWFRM->undo->CanUndo());
		QVERIFY(DBWFRM->undo->Undo());
		QCOMPARE((int)DBWFRM->einzug.feld.Get(3), 7);

		/*  Another undo restores state A. */
		QVERIFY(DBWFRM->undo->CanUndo());
		QVERIFY(DBWFRM->undo->Undo());
		QCOMPARE((int)DBWFRM->einzug.feld.Get(3), 0);
	}

	void redo_after_undo_advances_again()
	{
		DBWFRM->undo->Snapshot();                 /* state A (empty) */
		DBWFRM->einzug.feld.Set(0, (short)11);
		DBWFRM->undo->Snapshot();                 /* state B */
		DBWFRM->einzug.feld.Set(0, (short)22);
		DBWFRM->undo->Snapshot();                 /* state C */

		DBWFRM->undo->Undo();                     /* -> B */
		QCOMPARE((int)DBWFRM->einzug.feld.Get(0), 11);

		QVERIFY(DBWFRM->undo->CanRedo());
		DBWFRM->undo->Redo();                     /* -> C */
		QCOMPARE((int)DBWFRM->einzug.feld.Get(0), 22);
	}

	void clear_wipes_all_items()
	{
		DBWFRM->gewebe.feld.Set(1, 1, (char)5);
		DBWFRM->undo->Snapshot();
		DBWFRM->undo->Clear();
		QCOMPARE(DBWFRM->undo->CanUndo(), false);
		QCOMPARE(DBWFRM->undo->CanRedo(), false);
	}

	void snapshot_captures_multiple_felds()
	{
		DBWFRM->einzug.feld.Set(0, (short)3);
		DBWFRM->aufknuepfung.feld.Set(1, 2, (char)4);
		DBWFRM->undo->Snapshot();

		DBWFRM->einzug.feld.Set(0, (short)99);
		DBWFRM->aufknuepfung.feld.Set(1, 2, (char)0);
		DBWFRM->undo->Snapshot();

		DBWFRM->undo->Undo();
		QCOMPARE((int)DBWFRM->einzug.feld.Get(0), 3);
		QCOMPARE((int)DBWFRM->aufknuepfung.feld.Get(1, 2), 4);
	}

	void undo_preserves_einzugstil_action_checked_state()
	{
		/*  Default einzugstil is EzBelassen. Switch to EzGeradeZ
		    before snapshotting, then flip away, then undo: the
		    stored einzugstil should re-check EzGeradeZ.          */
		DBWFRM->EzBelassen->setChecked(false);
		DBWFRM->EzGeradeZ->setChecked(true);
		DBWFRM->undo->Snapshot();

		DBWFRM->EzGeradeZ->setChecked(false);
		DBWFRM->EzBelassen->setChecked(true);
		DBWFRM->undo->Snapshot();

		DBWFRM->undo->Undo();
		QCOMPARE(DBWFRM->EzGeradeZ->isChecked(), true);
	}
};

QTEST_MAIN(TestUndoredo)
#include "test_undoredo.moc"
