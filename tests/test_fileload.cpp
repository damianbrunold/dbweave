/*  DB-WEAVE Qt 6 port - file loader tests.
    Loads real .dbw files from samples/ and verifies the core fields
    (einzug, aufknuepfung, trittfolge, gewebe-derived ranges) are
    populated. Uses the CMake source-dir hint via QT_TESTCASE_SOURCEDIR. */

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

#include "datamodule.h"
#include "mainwindow.h"
#include "hilfslinien.h"
#include "loadoptions.h"

class TestFileload : public QObject
{
    Q_OBJECT

    QString samplePath(const char* _name)
    {
        /*  QT_TESTCASE_SOURCEDIR points at the tests/ dir; the repo
            samples/ sits one level up. */
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

    void load_nonexistent_file_reports_FILE_DOES_NOT_EXIST()
    {
        DBWFRM->filename = QStringLiteral("/nonexistent/file.dbw");
        LOADSTAT stat = FILE_LOADED;
        const bool ok = DBWFRM->Load(stat, LOADALL);
        QCOMPARE(ok, false);
        QCOMPARE(stat, FILE_DOES_NOT_EXIST);
    }

    void load_satin_sample()
    {
        const QString path = samplePath("satin.dbw");
        QVERIFY2(QFileInfo::exists(path), qPrintable(QStringLiteral("sample not found: ") + path));
        DBWFRM->filename = path;

        LOADSTAT stat = UNKNOWN_FAILURE;
        const bool ok = DBWFRM->Load(stat, LOADALL);
        QCOMPARE(ok, true);
        QCOMPARE(stat, FILE_LOADED);

        /*  A satin file should have a non-empty einzug and
            aufknuepfung. Ranges should be computed post-load. */
        bool any_einzug = false;
        for (int i = 0; i < Data->MAXX1 && !any_einzug; i++)
            if (DBWFRM->einzug.feld.Get(i) != 0)
                any_einzug = true;
        QVERIFY2(any_einzug, "einzug is all zero after loading satin.dbw");

        bool any_aufknuepfung = false;
        for (int i = 0; i < Data->MAXX2 && !any_aufknuepfung; i++)
            for (int j = 0; j < Data->MAXY1 && !any_aufknuepfung; j++)
                if (DBWFRM->aufknuepfung.feld.Get(i, j) != 0)
                    any_aufknuepfung = true;
        QVERIFY2(any_aufknuepfung, "aufknuepfung is all zero");

        QVERIFY2(DBWFRM->kette.a >= 0 && DBWFRM->kette.b >= DBWFRM->kette.a,
                 qPrintable(QStringLiteral("invalid kette range: a=%1 b=%2")
                                .arg(DBWFRM->kette.a)
                                .arg(DBWFRM->kette.b)));
        QVERIFY2(DBWFRM->schuesse.a >= 0 && DBWFRM->schuesse.b >= DBWFRM->schuesse.a,
                 qPrintable(QStringLiteral("invalid schuesse range")));
    }

    void load_tromp_as_writ_sample()
    {
        const QString path = samplePath("tromp as writ.dbw");
        QVERIFY(QFileInfo::exists(path));
        DBWFRM->filename = path;

        LOADSTAT stat = UNKNOWN_FAILURE;
        QCOMPARE(DBWFRM->Load(stat, LOADALL), true);
        QCOMPARE(stat, FILE_LOADED);

        /*  "Tromp as writ" is a threading pattern; expect kette to
            span multiple warp threads. */
        QVERIFY(DBWFRM->kette.b >= DBWFRM->kette.a + 2);
    }

    void freieschaefte_reflects_loaded_einzug()
    {
        DBWFRM->filename = samplePath("satin.dbw");
        LOADSTAT stat;
        QVERIFY(DBWFRM->Load(stat, LOADALL));

        /*  At least one shaft should be marked NOT free (i.e. in
            use) after the load -- otherwise no einzug references it. */
        bool any_used = false;
        for (int j = 0; j < Data->MAXY1 && !any_used; j++)
            if (!DBWFRM->freieschaefte[j])
                any_used = true;
        QVERIFY(any_used);
    }

    void save_then_load_round_trips_fields()
    {
        /*  Load satin.dbw, capture its key state, save to a temp file,
            wipe TDBWFRM, reload from the temp file, compare. */
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        const QString out = tmp.filePath("roundtrip.dbw");

        DBWFRM->filename = samplePath("satin.dbw");
        LOADSTAT stat;
        QVERIFY(DBWFRM->Load(stat, LOADALL));

        /*  Capture expected state. */
        const int maxx1 = Data->MAXX1;
        const int maxy1 = Data->MAXY1;
        const short e0 = DBWFRM->einzug.feld.Get(0);
        const short e5 = DBWFRM->einzug.feld.Get(5);
        const char a00 = DBWFRM->aufknuepfung.feld.Get(0, 0);
        const char t00 = DBWFRM->trittfolge.feld.Get(0, 0);

        /*  Add a guide line so we cover the new hilfslinien round-trip. */
        DBWFRM->hlines.Add(HL_VERT, HL_LEFT, 7);

        DBWFRM->filename = out;
        QVERIFY2(DBWFRM->Save(), "Save() returned false");

        /*  Fresh TDBWFRM, load the file we just wrote. */
        delete DBWFRM;
        delete Data;
        Data = new TData();
        DBWFRM = new TDBWFRM();
        DBWFRM->filename = out;
        QVERIFY(DBWFRM->Load(stat, LOADALL));
        QCOMPARE(stat, FILE_LOADED);

        QCOMPARE(Data->MAXX1, maxx1);
        QCOMPARE(Data->MAXY1, maxy1);
        QCOMPARE(DBWFRM->einzug.feld.Get(0), e0);
        QCOMPARE(DBWFRM->einzug.feld.Get(5), e5);
        QCOMPARE(DBWFRM->aufknuepfung.feld.Get(0, 0), a00);
        QCOMPARE(DBWFRM->trittfolge.feld.Get(0, 0), t00);

        QCOMPARE(DBWFRM->hlines.GetCount(), 1);
        Hilfslinie* h = DBWFRM->hlines.GetLine(0);
        QVERIFY(h);
        QCOMPARE((int)h->typ, (int)HL_VERT);
        QCOMPARE((int)h->feld, (int)HL_LEFT);
        QCOMPARE(h->pos, 7);
    }

    void save_without_filename_returns_false()
    {
        DBWFRM->filename = QString();
        QVERIFY(!DBWFRM->Save());
    }

    void multiple_samples_load_cleanly()
    {
        const char* samples[] = {
            "satin.dbw", "tromp as writ.dbw", "stripes.dbw", "double face.dbw", "Pepita.dbw",
        };
        for (const char* name : samples) {
            /*  Reset TDBWFRM between loads. */
            delete DBWFRM;
            DBWFRM = new TDBWFRM();

            const QString path = samplePath(name);
            if (!QFileInfo::exists(path))
                continue; /* optional */
            DBWFRM->filename = path;
            LOADSTAT stat = UNKNOWN_FAILURE;
            QVERIFY2(DBWFRM->Load(stat, LOADALL),
                     qPrintable(QStringLiteral("failed to load: ") + path));
            QCOMPARE(stat, FILE_LOADED);
        }
    }
};

QTEST_MAIN(TestFileload)
#include "test_fileload.moc"
