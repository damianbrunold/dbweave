/*  DB-WEAVE Qt 6 port - compatibility-shim tests (Phase 1) */

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "vcl_compat.h"
#include "colors_compat.h"
#include "tstringlist_compat.h"
#include "tbitmap_compat.h"
#include "registry_compat.h"

class TestCompat : public QObject
{
	Q_OBJECT

private slots:

	/* ----- vcl_compat.h ----- */

	void ansistring_is_qstring()
	{
		AnsiString a = QStringLiteral("hello");
		QCOMPARE(a.length(), 5);
		QVERIFY(a == QStringLiteral("hello"));

		/* True / False macros */
		QVERIFY(True == true);
		QVERIFY(False == false);
	}

	/* ----- colors_compat.h ----- */

	void tcolor_byte_order_matches_vcl()
	{
		/* VCL encodes pure blue as 0x00FF0000 (BBGGRR). */
		const TColor pureBlue = 0x00FF0000;
		QColor qc = qColorFromTColor(pureBlue);
		QCOMPARE(qc.red(),   0);
		QCOMPARE(qc.green(), 0);
		QCOMPARE(qc.blue(),  255);

		/* Round-trip via QColor. */
		QColor src(17, 131, 240);
		TColor tc = tColorFromQColor(src);
		QCOMPARE(qColorFromTColor(tc), src);

		/* Convenience constructor. */
		QCOMPARE(tColorFromRGB(10, 20, 30), TColor(0x001E140A));
	}

	/* ----- tstringlist_compat.h ----- */

	void tstringlist_add_count_index()
	{
		TStringList sl;
		sl.Add(QStringLiteral("alpha"));
		sl.Add(QStringLiteral("beta"));
		sl.Add(QStringLiteral("gamma"));

		QCOMPARE(sl.Count(), 3);
		QCOMPARE(sl[1],              QStringLiteral("beta"));
		QCOMPARE(sl.Strings[2],      QStringLiteral("gamma"));

		sl.Clear();
		QCOMPARE(sl.Count(), 0);
	}

	void tstringlist_file_round_trip()
	{
		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		const QString path = tmp.filePath(QStringLiteral("list.txt"));

		TStringList out;
		out.Add(QStringLiteral("one"));
		out.Add(QStringLiteral("zwei"));
		out.Add(QStringLiteral("trois"));
		QVERIFY(out.SaveToFile(path));

		TStringList in;
		QVERIFY(in.LoadFromFile(path));
		QCOMPARE(in.Count(), 3);
		QCOMPARE(in[0], QStringLiteral("one"));
		QCOMPARE(in[1], QStringLiteral("zwei"));
		QCOMPARE(in[2], QStringLiteral("trois"));
	}

	/* ----- tbitmap_compat.h ----- */

	void tbitmap_sizes_and_io()
	{
		TBitmap bmp;
		bmp.SetSize(16, 9);
		QCOMPARE(bmp.Width(),  16);
		QCOMPARE(bmp.Height(),  9);

		QTemporaryDir tmp;
		QVERIFY(tmp.isValid());
		const QString path = tmp.filePath(QStringLiteral("b.png"));
		QVERIFY(bmp.SaveToFile(path, "PNG"));

		TBitmap loaded;
		QVERIFY(loaded.LoadFromFile(path));
		QCOMPARE(loaded.Width(),  16);
		QCOMPARE(loaded.Height(),  9);
	}

	/* ----- registry_compat.h ----- */

	void tregistry_round_trip()
	{
		/* Use an isolated QSettings scope for the duration of the test. */
		QCoreApplication::setOrganizationName(QStringLiteral("DbweaveTest"));
		QCoreApplication::setApplicationName  (QStringLiteral("RegistryCompat"));

		{
			TRegistry r;
			r.RootKey = HKEY_CURRENT_USER;
			QVERIFY(r.OpenKey(QStringLiteral("Software\\dbw\\General"), true));
			r.WriteInteger(QStringLiteral("zoom"), 400);
			r.WriteString (QStringLiteral("lang"), QStringLiteral("de"));
			r.CloseKey();
		}
		{
			TRegistry r;
			QVERIFY(r.OpenKey(QStringLiteral("Software/dbw/General"), false));
			QVERIFY(r.ValueExists(QStringLiteral("zoom")));
			QCOMPARE(r.ReadInteger(QStringLiteral("zoom")), 400);
			QCOMPARE(r.ReadString (QStringLiteral("lang")), QStringLiteral("de"));
			QVERIFY(!r.ValueExists(QStringLiteral("missing")));
			r.CloseKey();
		}
	}
};

QTEST_MAIN(TestCompat)
#include "test_compat.moc"
