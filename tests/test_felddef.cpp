/*  DB-WEAVE Qt 6 port - domain/felddef tests */

#include <QTemporaryDir>
#include <QTest>

#include "felddef.h"
#include "fileformat.h"

class TestFelddef : public QObject
{
	Q_OBJECT

	/*  Round-trip helper: write the given object into a @dbw3:file with
	    one named section, read it back into `into`, and assert the
	    file-level handshake succeeded. The actual data equivalence is
	    checked by the caller. */
	template <typename T, typename Default>
	static void roundTrip (T& out, const char* section, T& into, Default def)
	{
		QTemporaryDir tmp; QVERIFY(tmp.isValid());
		const QByteArray path = tmp.filePath(QStringLiteral("ff.dbw")).toLocal8Bit();
		{
			FfWriter w;
			QVERIFY(w.Open(path.constData()));
			w.WriteSignature();
			out.Write(section, &w);
			w.Close();
		}

		FfReader r;
		QVERIFY(r.Open(path.constData()));
		FfToken* sig = r.GetToken();
		QCOMPARE(sig->GetType(), FfSignature);
		delete sig;
		FfToken* sec = r.GetToken();
		QCOMPARE(sec->GetType(), FfSection);
		QVERIFY(IsTokenEqual(sec, section));
		delete sec;
		into.Read(&r, def);
		r.Close();
	}

private slots:

	void feld_vector_char_init_resize_get_set()
	{
		FeldVectorChar v(5, 7);
		QCOMPARE(v.Size(), 5);
		for (int i = 0; i < 5; i++) QCOMPARE(v.Get(i), (unsigned char)7);

		v.Set(0, 1);  v.Set(2, 3);  v.Set(4, 5);
		QCOMPARE(v.Get(0), (unsigned char)1);
		QCOMPARE(v.Get(2), (unsigned char)3);
		QCOMPARE(v.Get(4), (unsigned char)5);

		v.Resize(8, 9);
		QCOMPARE(v.Size(), 8);
		QCOMPARE(v.Get(0), (unsigned char)1);
		QCOMPARE(v.Get(4), (unsigned char)5);
		QCOMPARE(v.Get(5), (unsigned char)9);
		QCOMPARE(v.Get(7), (unsigned char)9);

		v.Resize(3, 0);
		QCOMPARE(v.Size(), 3);
		QCOMPARE(v.Get(0), (unsigned char)1);
		QCOMPARE(v.Get(2), (unsigned char)3);
	}

	void feld_vector_char_assignment()
	{
		FeldVectorChar a(4, 0), b(2, 0);
		a.Set(0, 10); a.Set(1, 20); a.Set(2, 30); a.Set(3, 40);
		b = a;
		QCOMPARE(b.Size(), 4);
		QCOMPARE(b.Get(3), (unsigned char)40);
	}

	void feld_vector_char_round_trip()
	{
		FeldVectorChar out(6, 0);
		for (int i = 0; i < 6; i++) out.Set(i, (unsigned char)(i*13 + 5));

		FeldVectorChar in(1, 0);
		roundTrip(out, "vec", in, (unsigned char)0);

		QCOMPARE(in.Size(), 6);
		for (int i = 0; i < 6; i++)
			QCOMPARE(in.Get(i), (unsigned char)(i*13 + 5));
	}

	void feld_vector_short_round_trip()
	{
		FeldVectorShort out(4, 0);
		out.Set(0,   0);
		out.Set(1,  32000);
		out.Set(2, -32000);
		out.Set(3,  12345);

		FeldVectorShort in(1, 0);
		roundTrip(out, "vs", in, (short)0);

		QCOMPARE(in.Size(), 4);
		QCOMPARE(in.Get(0), (short)0);
		QCOMPARE(in.Get(1), (short)32000);
		QCOMPARE(in.Get(2), (short)-32000);
		QCOMPARE(in.Get(3), (short)12345);
	}

	void feld_vector_bool_round_trip()
	{
		FeldVectorBool out(5, false);
		out.Set(0, true);
		out.Set(2, true);
		out.Set(4, true);

		FeldVectorBool in(1, false);
		roundTrip(out, "vb", in, false);

		QCOMPARE(in.Size(), 5);
		QCOMPARE(in.Get(0), true);
		QCOMPARE(in.Get(1), false);
		QCOMPARE(in.Get(2), true);
		QCOMPARE(in.Get(3), false);
		QCOMPARE(in.Get(4), true);
	}

	void feld_grid_char_round_trip()
	{
		FeldGridChar out(3, 4, 0);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 4; j++)
				out.Set(i, j, (char)(i*10 + j + 1));

		FeldGridChar in(1, 1, 0);
		roundTrip(out, "grid", in, (char)0);

		QCOMPARE(in.SizeX(), 3);
		QCOMPARE(in.SizeY(), 4);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 4; j++)
				QCOMPARE(in.Get(i, j), (char)(i*10 + j + 1));
	}

	void feld_grid_char_resize_preserves_top_left()
	{
		FeldGridChar g(3, 3, 0);
		g.Set(0,0,1); g.Set(1,1,2); g.Set(2,2,3);

		g.Resize(5, 2, 9);
		QCOMPARE(g.SizeX(), 5);
		QCOMPARE(g.SizeY(), 2);
		QCOMPARE(g.Get(0,0), (char)1);
		QCOMPARE(g.Get(1,1), (char)2);
		QCOMPARE(g.Get(3,0), (char)9);
		QCOMPARE(g.Get(4,1), (char)9);
	}
};

QTEST_APPLESS_MAIN(TestFelddef)
#include "test_felddef.moc"
