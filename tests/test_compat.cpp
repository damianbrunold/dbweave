/*  DB-WEAVE - compat-helper tests. Exercises colors_compat.h's
    TColor <-> QColor converter. */

#include <QCoreApplication>
#include <QTest>

#include "colors_compat.h"

class TestCompat : public QObject
{
    Q_OBJECT

private slots:

    /* ----- colors_compat.h ----- */

    void tcolor_byte_order_matches_vcl()
    {
        /* VCL encodes pure blue as 0x00FF0000 (BBGGRR). */
        const TColor pureBlue = 0x00FF0000;
        QColor qc = qColorFromTColor(pureBlue);
        QCOMPARE(qc.red(), 0);
        QCOMPARE(qc.green(), 0);
        QCOMPARE(qc.blue(), 255);

        /* Round-trip via QColor. */
        QColor src(17, 131, 240);
        TColor tc = tColorFromQColor(src);
        QCOMPARE(qColorFromTColor(tc), src);

        /* Convenience constructor. */
        QCOMPARE(tColorFromRGB(10, 20, 30), TColor(0x001E140A));
    }
};

QTEST_MAIN(TestCompat)
#include "test_compat.moc"
