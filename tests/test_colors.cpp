/*  DB-WEAVE - domain/colors tests */

#include <QTest>
#include <cmath>

#include "colors.h"

class TestColors : public QObject
{
    Q_OBJECT

    static bool nearly(float a, float b, float tol = 1e-3f)
    {
        return std::fabs(a - b) <= tol;
    }

private slots:

    void rgb_to_hsv_primary_red()
    {
        float h, s, v;
        RGB2HSV(255, 0, 0, h, s, v);
        QVERIFY(nearly(h, 0.0f));
        QVERIFY(nearly(s, 1.0f));
        QVERIFY(nearly(v, 1.0f));
    }

    void rgb_to_hsv_primary_green()
    {
        float h, s, v;
        RGB2HSV(0, 255, 0, h, s, v);
        QVERIFY(nearly(h, 120.0f));
        QVERIFY(nearly(s, 1.0f));
        QVERIFY(nearly(v, 1.0f));
    }

    void rgb_to_hsv_primary_blue()
    {
        float h, s, v;
        RGB2HSV(0, 0, 255, h, s, v);
        QVERIFY(nearly(h, 240.0f));
        QVERIFY(nearly(s, 1.0f));
        QVERIFY(nearly(v, 1.0f));
    }

    void rgb_to_hsv_greyscale_is_undefined()
    {
        float h, s, v;
        RGB2HSV(128, 128, 128, h, s, v);
        QCOMPARE(h, float(UNDEFINED));
        QVERIFY(nearly(s, 0.0f));
        QVERIFY(nearly(v, 128.0f / 255.0f));
    }

    void hsv_to_rgb_primaries_round_trip()
    {
        int r, g, b;
        HSV2RGB(0.0f, 1.0f, 1.0f, r, g, b);
        QCOMPARE(r, 255);
        QCOMPARE(g, 0);
        QCOMPARE(b, 0);
        HSV2RGB(120.0f, 1.0f, 1.0f, r, g, b);
        QCOMPARE(r, 0);
        QCOMPARE(g, 255);
        QCOMPARE(b, 0);
        HSV2RGB(240.0f, 1.0f, 1.0f, r, g, b);
        QCOMPARE(r, 0);
        QCOMPARE(g, 0);
        QCOMPARE(b, 255);
    }

    void hsv_to_rgb_zero_saturation_is_grey()
    {
        int r, g, b;
        HSV2RGB(180.0f, 0.0f, 0.5f, r, g, b);
        QCOMPARE(r, g);
        QCOMPARE(g, b);
        QVERIFY(r > 120 && r < 135);
    }

    void round_trip_rgb_hsv_rgb()
    {
        /*  Pick a handful of non-degenerate colours and verify that
            RGB -> HSV -> RGB is identity within ±1 per channel (the
            legacy code uses integer -> float -> integer conversions
            that lose at most one bit). */
        struct Case {
            int r, g, b;
        } cases[] = {
            { 17, 131, 240 },
            { 200, 50, 90 },
            { 10, 200, 180 },
            { 250, 250, 100 },
        };
        for (auto c : cases) {
            float h, s, v;
            RGB2HSV(c.r, c.g, c.b, h, s, v);
            int r, g, b;
            HSV2RGB(h, s, v, r, g, b);
            QVERIFY2(std::abs(r - c.r) <= 1,
                     qPrintable(QString("r: got %1, want %2").arg(r).arg(c.r)));
            QVERIFY2(std::abs(g - c.g) <= 1,
                     qPrintable(QString("g: got %1, want %2").arg(g).arg(c.g)));
            QVERIFY2(std::abs(b - c.b) <= 1,
                     qPrintable(QString("b: got %1, want %2").arg(b).arg(c.b)));
        }
    }
};

QTEST_APPLESS_MAIN(TestColors)
#include "test_colors.moc"
