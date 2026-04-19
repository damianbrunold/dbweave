/*  DB-WEAVE Qt 6 port - residual compat-shim tests (Phase 12) */

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QTest>

#include "vcl_compat.h"
#include "colors_compat.h"
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
		/*  String is the second legacy alias. */
		String b = QStringLiteral("world");
		QCOMPARE(b.length(), 5);
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
