/*  DB-WEAVE Qt 6 port - io/fileformat round-trip tests */

#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <cstring>

#include "fileformat.h"

/*  Helper: slurp a file into a QByteArray for byte-level assertions. */
static QByteArray slurp (const QString& _path)
{
	QFile f(_path);
	if (!f.open(QIODevice::ReadOnly)) return {};
	return f.readAll();
}

class TestFileformat : public QObject
{
	Q_OBJECT

private slots:

	void writer_emits_crlf_signature_and_section()
	{
		QTemporaryDir tmp; QVERIFY(tmp.isValid());
		const QString path = tmp.filePath(QStringLiteral("a.dbw"));

		{
			FfWriter w;
			QVERIFY(w.Open(path.toLocal8Bit().constData()));
			w.WriteSignature();
			w.BeginSection("sec");
			w.WriteFieldInt("n", 42);
			w.EndSection();
			w.Close();
		}

		const QByteArray data = slurp(path);

		/*  Signature at offset 0 including CR LF. */
		QVERIFY(data.startsWith(QByteArrayLiteral("@dbw3:file\r\n")));

		/*  Section syntax: \sec{...}  followed by CRLF after the
		    opening brace, closing } on its own line. */
		QVERIFY(data.contains(QByteArrayLiteral("\\sec{\r\n")));
		QVERIFY(data.contains(QByteArrayLiteral("n==42\r\n")));
		QVERIFY(data.contains(QByteArrayLiteral("}\r\n")));
	}

	void reader_parses_writer_output()
	{
		QTemporaryDir tmp; QVERIFY(tmp.isValid());
		const QString path = tmp.filePath(QStringLiteral("b.dbw"));

		{
			FfWriter w;
			QVERIFY(w.Open(path.toLocal8Bit().constData()));
			w.WriteSignature();
			w.BeginSection("header");
			w.WriteFieldInt   ("version", 7);
			w.WriteField      ("name",    "Siropa");
			w.WriteFieldDouble("pi",      3.14159265);
			w.EndSection();
			w.Close();
		}

		FfReader r;
		QVERIFY(r.Open(path.toLocal8Bit().constData()));

		/*  Token 1: signature. */
		FfToken* t = r.GetToken();
		QVERIFY(t != nullptr);
		QCOMPARE(t->GetType(), FfSignature);
		delete t;

		/*  Token 2: section "header". */
		t = r.GetToken();
		QVERIFY(t != nullptr);
		QCOMPARE(t->GetType(), FfSection);
		QVERIFY(IsTokenEqual(t, "header"));
		delete t;

		/*  Fields in order: version, name, pi. */
		auto expectFieldValue = [&](const char* field, const char* value) {
			FfToken* fld = r.GetToken();
			QVERIFY(fld != nullptr);
			QCOMPARE(fld->GetType(), FfField);
			QVERIFY2(IsTokenEqual(fld, field),
			         qPrintable(QStringLiteral("expected field ") + QLatin1String(field)));
			delete fld;
			FfToken* val = r.GetToken();
			QVERIFY(val != nullptr);
			QCOMPARE(val->GetType(), FfValue);
			QVERIFY2(IsTokenEqual(val, value),
			         qPrintable(QStringLiteral("expected value ") + QLatin1String(value)));
			delete val;
		};
		expectFieldValue("version", "7");
		expectFieldValue("name",    "Siropa");
		expectFieldValue("pi",      "3.14159265");

		/*  Token: end-section. */
		t = r.GetToken();
		QVERIFY(t != nullptr);
		QCOMPARE(t->GetType(), FfEndSection);
		delete t;

		r.Close();
	}

	void binary_field_hex_encoding_round_trip()
	{
		QTemporaryDir tmp; QVERIFY(tmp.isValid());
		const QString path = tmp.filePath(QStringLiteral("c.dbw"));

		/*  72 bytes ensures the writer's 70-char chunking kicks in. */
		const int N = 72;
		unsigned char src[N];
		for (int i = 0; i < N; i++) src[i] = (unsigned char)(i * 7 + 3);

		{
			FfWriter w;
			QVERIFY(w.Open(path.toLocal8Bit().constData()));
			w.WriteSignature();
			w.BeginSection("data");
			w.WriteFieldBinary("blob", src, N);
			w.EndSection();
			w.Close();
		}

		/*  The file should contain 2 hex chars per byte plus some
		    chunking markers. Decode by hand to verify round-trip. */
		FfReader r;
		QVERIFY(r.Open(path.toLocal8Bit().constData()));

		delete r.GetToken(); /* signature */
		delete r.GetToken(); /* section */
		FfToken* field = r.GetToken();
		QCOMPARE(field->GetType(), FfField);
		QVERIFY(IsTokenEqual(field, "blob"));
		delete field;
		FfToken* value = r.GetToken();
		QCOMPARE(value->GetType(), FfValue);

		/*  Hex-decode the value and compare against src. */
		FfTokenBase* base = static_cast<FfTokenBase*>(value);
		const char* hex = static_cast<const char*>(base->data);
		QCOMPARE(base->length, 2 * N);
		unsigned char decoded[N];
		for (int i = 0; i < N; i++) {
			auto nib = [](char c) {
				if (c >= '0' && c <= '9') return c - '0';
				return 10 + (c - 'a');
			};
			decoded[i] = (unsigned char)((nib(hex[2*i]) << 4) | nib(hex[2*i+1]));
		}
		QCOMPARE(std::memcmp(decoded, src, N), 0);

		delete value;
		r.Close();
	}
};

QTEST_APPLESS_MAIN(TestFileformat)
#include "test_fileformat.moc"
