/*  DB-WEAVE Qt 6 port - language / dbw3_strings tests */

#include <QTest>

#include "language.h"
#include "dbw3_strings.h"

/*  The ported dbw3_strings.h #defines return (AnsiString).c_str()-
    style temporaries matching the legacy VCL behaviour, which is a
    dangling-pointer hazard if the result is stored in a variable
    rather than used directly in a function argument (matches legacy).
    These tests exercise the LANG_STR macro and a subset of the
    QString-valued #defines where lifetime is safe.                */

class TestLanguage : public QObject
{
    Q_OBJECT

private slots:

    void default_language_is_english()
    {
        /*  Reset each test because active_language is a global. */
        active_language = EN;
        QCOMPARE(active_language, EN);

        const QString s = LANG_STR("hello", "hallo");
        QCOMPARE(s, QStringLiteral("hello"));
    }

    void switching_to_german_picks_ge_branch()
    {
        active_language = EN;
        SwitchLanguage(GE);
        QCOMPARE(active_language, GE);

        const QString s = LANG_STR("hello", "hallo");
        QCOMPARE(s, QStringLiteral("hallo"));

        SwitchLanguage(EN); /* restore */
    }

    void switching_to_same_language_is_noop()
    {
        active_language = EN;
        SwitchLanguage(EN);
        QCOMPARE(active_language, EN);
    }

    void german_umlauts_round_trip_utf8()
    {
        /*  The umlauts get transcoded from legacy Latin-1 to UTF-8
            in the port. Verify they reach QString correctly. */
        active_language = GE;
        QCOMPARE(LANG_STR("Pattern", "Muster für den Grund"),
                 QStringLiteral("Muster für den Grund"));
        QCOMPARE(LANG_STR("Length", "Länge"), QStringLiteral("Länge"));
        QCOMPARE(LANG_STR("unused", "Öffnen"), QStringLiteral("Öffnen"));
        active_language = EN; /* restore */
    }

    void qstring_valued_strings_macro_works()
    {
        active_language = EN;
        QCOMPARE(ENTW_ALLGEMEIN, QStringLiteral("General"));
        QCOMPARE(ENTW_FARBEN, QStringLiteral("Colors"));
        QCOMPARE(CURSORUP, QStringLiteral("up"));

        active_language = GE;
        QCOMPARE(ENTW_ALLGEMEIN, QStringLiteral("Allgemein"));
        QCOMPARE(ENTW_FARBEN, QStringLiteral("Farben"));
        QCOMPARE(ENTW_SCHAFTCOUNT, QStringLiteral("Anzahl Schäfte: "));

        active_language = EN;
    }
};

QTEST_APPLESS_MAIN(TestLanguage)
#include "test_language.moc"
