/*  DB-WEAVE Qt 6 port - domain/palette tests */

#include <QTemporaryDir>
#include <QTest>

#include "palette.h"
#include "fileformat.h"

class TestPalette : public QObject
{
    Q_OBJECT

private slots:

    void default_palette_has_known_entries()
    {
        Palette p;
        /*  First entry is the legacy bright-red anchor. */
        QCOMPARE(p.GetColor(0), (COLORREF)RGB(0xff, 0x00, 0x00));
    }

    void set_color_round_trip()
    {
        Palette p;
        const COLORREF c = RGB(17, 131, 240);
        p.SetColor(42, c);
        QCOMPARE(p.GetColor(42), c);
    }

    void palette2_differs_from_palette1()
    {
        Palette p;
        COLORREF c0 = p.GetColor(0);
        p.SetPaletteType(true);
        QVERIFY(p.GetColor(0) != c0);
    }

    void save_load_round_trip_format37()
    {
        Palette out;
        out.SetColor(0, RGB(10, 20, 30));
        out.SetColor(100, RGB(40, 50, 60));
        out.SetColor(MAX_PAL_ENTRY - 1, RGB(70, 80, 90));

        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QByteArray path = tmp.filePath(QStringLiteral("pal.dbw")).toLocal8Bit();
        {
            FfWriter w;
            QVERIFY(w.Open(path.constData()));
            w.WriteSignature();
            out.Save(&w, /*_format37=*/true);
            w.Close();
        }

        Palette in;
        FfReader r;
        QVERIFY(r.Open(path.constData()));
        delete r.GetToken(); /* signature */
        FfToken* sec = r.GetToken();
        QCOMPARE(sec->GetType(), FfSection);
        QVERIFY(IsTokenEqual(sec, "palette"));
        delete sec;
        in.Load(&r);
        r.Close();

        for (int i = 0; i < MAX_PAL_ENTRY; i++)
            QCOMPARE(in.GetColor(i), out.GetColor(i));
    }

    void save_load_round_trip_legacy_format()
    {
        Palette out;
        out.SetColor(5, RGB(11, 22, 33));
        out.SetColor(235, RGB(99, 88, 77));

        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QByteArray path = tmp.filePath(QStringLiteral("pal_legacy.dbw")).toLocal8Bit();
        {
            FfWriter w;
            QVERIFY(w.Open(path.constData()));
            w.WriteSignature();
            out.Save(&w, /*_format37=*/false);
            w.Close();
        }

        Palette in;
        FfReader r;
        QVERIFY(r.Open(path.constData()));
        delete r.GetToken(); /* signature */
        delete r.GetToken(); /* section */
        in.Load(&r);
        r.Close();

        for (int i = 0; i < MAX_PAL_ENTRY; i++)
            QCOMPARE(in.GetColor(i), out.GetColor(i));
    }
};

QTEST_APPLESS_MAIN(TestPalette)
#include "test_palette.moc"
