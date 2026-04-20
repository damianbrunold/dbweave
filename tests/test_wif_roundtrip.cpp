/*  DB-WEAVE Qt 6 port - WIF export/import round-trip test.
    Loads a sample .dbw, exports to WIF, imports the WIF into a fresh
    TDBWFRM, and compares the round-tripped domain state against the
    original. */

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>
#include <QVector>

#include "datamodule.h"
#include "loadoptions.h"
#include "mainwindow.h"
#include "palette.h"
#include "properties.h"

class TestWifRoundtrip : public QObject
{
    Q_OBJECT

    QString samplePath(const char* _name)
    {
        const QString base = QString(QT_TESTCASE_SOURCEDIR);
        return QDir(base).absoluteFilePath(QString("../samples/") + _name);
    }

private slots:

    void init()
    {
        Data = new TData();
        DBWFRM = new TDBWFRM();
    }

    void cleanup()
    {
        delete DBWFRM;
        DBWFRM = nullptr;
        delete Data;
        Data = nullptr;
    }

    void round_trip_satin()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString out = tmp.filePath("roundtrip.wif");

        DBWFRM->filename = samplePath("satin.dbw");
        LOADSTAT stat = UNKNOWN_FAILURE;
        QVERIFY(DBWFRM->Load(stat, LOADALL));
        QCOMPARE(stat, FILE_LOADED);

        /*  Snapshot the state we expect to survive the WIF round-trip. */
        const int ka = DBWFRM->kette.a;
        const int kb = DBWFRM->kette.b;
        const int sa = DBWFRM->schuesse.a;
        const int sb = DBWFRM->schuesse.b;
        QVERIFY(ka >= 0 && kb >= ka);
        QVERIFY(sa >= 0 && sb >= sa);

        QVector<int> einzug_ref;
        for (int i = ka; i <= kb; i++)
            einzug_ref.append(DBWFRM->einzug.feld.Get(i));

        QVector<int> kette_col_ref;
        for (int i = ka; i <= kb; i++)
            kette_col_ref.append(DBWFRM->kettfarben.feld.Get(i));

        QVector<int> schuss_col_ref;
        for (int j = sa; j <= sb; j++)
            schuss_col_ref.append(DBWFRM->schussfarben.feld.Get(j));

        QVector<int> blatt_ref;
        for (int i = ka; i <= kb; i++)
            blatt_ref.append(DBWFRM->blatteinzug.feld.Get(i));

        /*  Shafts/treadles used. */
        QVector<QVector<int>> tieup_ref; /* [treadle][shaft] */
        for (int i = 0; i < Data->MAXX2; i++) {
            QVector<int> col;
            for (int j = 0; j < Data->MAXY1; j++)
                if (DBWFRM->aufknuepfung.feld.Get(i, j) > 0)
                    col.append(j);
            tieup_ref.append(col);
        }

        QVector<QVector<int>> treadle_ref; /* [pick][treadle] */
        for (int j = sa; j <= sb; j++) {
            QVector<int> row;
            for (int i = 0; i < Data->MAXX2; i++)
                if (DBWFRM->trittfolge.feld.Get(i, j) > 0)
                    row.append(i);
            treadle_ref.append(row);
        }

        /*  Palette: record the full live slice. */
        QVector<COLORREF> palette_ref;
        for (int i = 0; i < MAX_PAL_ENTRY; i++)
            palette_ref.append(Data->palette->GetColor(i));

        const bool sinking_ref = DBWFRM->sinkingshed;

        /*  Author / remarks should also survive. */
        Data->properties->SetAuthor("Round Trip");
        Data->properties->SetRemarks("Hello from WIF.");

        /*  Export WIF. */
        DBWFRM->DateiExportieren(out);
        QVERIFY(QFileInfo(out).size() > 0);

        /*  Fresh frame, import the WIF. */
        delete DBWFRM;
        delete Data;
        Data = new TData();
        DBWFRM = new TDBWFRM();

        QVERIFY(DBWFRM->DateiImportieren(out));

        /*  Ranges may differ if WIF trims leading empty threads; but
            the exporter wrote kette.a..kette.b, which becomes indices
            0..(kb-ka) on re-import. */
        const int dx = kb - ka + 1;
        const int dy = sb - sa + 1;

        /*  Einzug. */
        for (int i = 0; i < dx; i++) {
            QCOMPARE(int(DBWFRM->einzug.feld.Get(i)), einzug_ref[i]);
        }

        /*  Warp / weft colours. */
        for (int i = 0; i < dx; i++) {
            QCOMPARE(int(DBWFRM->kettfarben.feld.Get(i)), kette_col_ref[i]);
        }
        for (int j = 0; j < dy; j++) {
            QCOMPARE(int(DBWFRM->schussfarben.feld.Get(j)), schuss_col_ref[j]);
        }

        /*  Blatteinzug. */
        for (int i = 0; i < dx; i++) {
            QCOMPARE(int(DBWFRM->blatteinzug.feld.Get(i)), blatt_ref[i]);
        }

        /*  Tie-up: the WIF export trims to firstschaft..lastschaft,
            so compare by shape rather than raw indices. */
        int used_treadles_ref = 0;
        for (int i = 0; i < tieup_ref.size(); i++)
            if (!tieup_ref[i].isEmpty() && i + 1 > used_treadles_ref)
                used_treadles_ref = i + 1;
        int used_treadles_rt = 0;
        for (int i = 0; i < Data->MAXX2; i++) {
            for (int j = 0; j < Data->MAXY1; j++)
                if (DBWFRM->aufknuepfung.feld.Get(i, j) > 0) {
                    if (i + 1 > used_treadles_rt)
                        used_treadles_rt = i + 1;
                }
        }
        QCOMPARE(used_treadles_rt, used_treadles_ref);

        /*  Treadling. */
        for (int j = 0; j < dy; j++) {
            QVector<int> row;
            for (int i = 0; i < Data->MAXX2; i++)
                if (DBWFRM->trittfolge.feld.Get(i, j) > 0)
                    row.append(i);
            QCOMPARE(row, treadle_ref[j]);
        }

        /*  Palette. */
        for (int i = 0; i < MAX_PAL_ENTRY; i++) {
            QCOMPARE(Data->palette->GetColor(i), palette_ref[i]);
        }

        QCOMPARE(DBWFRM->sinkingshed, sinking_ref);

        QCOMPARE(QString::fromUtf8(Data->properties->Author()), QStringLiteral("Round Trip"));
        QCOMPARE(QString::fromUtf8(Data->properties->Remarks()), QStringLiteral("Hello from WIF."));
    }

    void round_trip_multiple_samples()
    {
        const char* samples[] = {
            "satin.dbw",
            "tromp as writ.dbw",
            "stripes.dbw",
            "Pepita.dbw",
        };

        for (const char* name : samples) {
            delete DBWFRM;
            delete Data;
            Data = new TData();
            DBWFRM = new TDBWFRM();

            const QString path = samplePath(name);
            if (!QFileInfo::exists(path))
                continue;

            DBWFRM->filename = path;
            LOADSTAT stat;
            QVERIFY2(DBWFRM->Load(stat, LOADALL), name);

            const int ka = DBWFRM->kette.a;
            const int kb = DBWFRM->kette.b;
            const int sa = DBWFRM->schuesse.a;
            const int sb = DBWFRM->schuesse.b;
            QVERIFY2(ka >= 0 && kb >= ka && sa >= 0 && sb >= sa, name);

            QVector<int> einzug_ref;
            for (int i = ka; i <= kb; i++)
                einzug_ref.append(DBWFRM->einzug.feld.Get(i));
            QVector<int> kfarb_ref;
            for (int i = ka; i <= kb; i++)
                kfarb_ref.append(DBWFRM->kettfarben.feld.Get(i));
            QVector<int> sfarb_ref;
            for (int j = sa; j <= sb; j++)
                sfarb_ref.append(DBWFRM->schussfarben.feld.Get(j));

            QTemporaryDir tmp;
            QVERIFY(tmp.isValid());
            const QString out = tmp.filePath(QString("rt_%1.wif").arg(name));
            DBWFRM->DateiExportieren(out);

            delete DBWFRM;
            delete Data;
            Data = new TData();
            DBWFRM = new TDBWFRM();
            QVERIFY2(DBWFRM->DateiImportieren(out), name);

            const int dx = kb - ka + 1;
            const int dy = sb - sa + 1;
            for (int i = 0; i < dx; i++)
                QCOMPARE(int(DBWFRM->einzug.feld.Get(i)), einzug_ref[i]);
            for (int i = 0; i < dx; i++)
                QCOMPARE(int(DBWFRM->kettfarben.feld.Get(i)), kfarb_ref[i]);
            for (int j = 0; j < dy; j++)
                QCOMPARE(int(DBWFRM->schussfarben.feld.Get(j)), sfarb_ref[j]);
        }
    }

    /*  Guard against the "drawRect fills with leftover PaintCell
        brush" regression: after exporting a bitmap of a real pattern,
        the gewebe quadrant should contain more than one distinct
        colour. A solid-black image means we've painted the quadrants
        over themselves. */
    void bitmap_export_is_not_solid_black()
    {
        DBWFRM->filename = samplePath("stripes.dbw");
        LOADSTAT stat;
        QVERIFY(DBWFRM->Load(stat, LOADALL));

        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString out = tmp.filePath("stripes.png");
        DBWFRM->DoExportPng(out);

        QImage img(out);
        QVERIFY(!img.isNull());

        QSet<QRgb> colours;
        /*  Sample the middle of the gewebe quadrant. */
        const int cx = img.width() / 4;
        const int cy = img.height() * 3 / 4;
        for (int dy = -20; dy <= 20; dy += 4)
            for (int dx = -20; dx <= 20; dx += 4)
                colours.insert(img.pixel(cx + dx, cy + dy));
        QVERIFY2(colours.size() > 1, "gewebe quadrant is a single solid colour -- drawRect likely "
                                     "filled over fillRect output");
    }

    /*  Smoke-test all four exporters: each must produce a non-empty
        file with a sensible magic header / preamble. */
    void all_formats_produce_valid_files()
    {
        DBWFRM->filename = samplePath("stripes.dbw");
        LOADSTAT stat;
        QVERIFY(DBWFRM->Load(stat, LOADALL));

        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString png = tmp.filePath("x.png");
        const QString jpg = tmp.filePath("x.jpg");
        const QString svg = tmp.filePath("x.svg");
        const QString pdf = tmp.filePath("x.pdf");
        DBWFRM->DoExportPng(png);
        DBWFRM->DoExportJpeg(jpg);
        DBWFRM->DoExportSvg(svg);
        DBWFRM->DoExportPdf(pdf);

        auto head = [](const QString& path, qint64 n) {
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly))
                return QByteArray();
            return f.read(n);
        };
        QVERIFY2(head(png, 4).startsWith("\x89PNG"), "PNG magic");
        QVERIFY2(head(jpg, 2).startsWith("\xFF\xD8"), "JPEG magic");
        QVERIFY2(head(svg, 5).startsWith("<?xml"), "SVG header");
        QVERIFY2(head(pdf, 4).startsWith("%PDF"), "PDF magic");
    }
};

QTEST_MAIN(TestWifRoundtrip)
#include "test_wif_roundtrip.moc"
