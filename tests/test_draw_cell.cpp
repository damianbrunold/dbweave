/*  DB-WEAVE Qt 6 port - ui/draw_cell golden-pixel tests.

    Antialiasing is disabled inside PaintCell so output is deterministic
    across platforms. Each test renders ONE cell into a small QImage and
    checks a handful of pixels that are invariant with respect to font
    rendering (NUMBER is tested separately with looser assertions). The
    cell occupies [0, SZ) x [0, SZ) in image coordinates. */

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QTest>

#include "draw_cell.h"
#include "enums.h"

class TestDrawCell : public QObject
{
    Q_OBJECT

    static constexpr int SZ = 20;
    static constexpr QRgb BG = 0xFFFFFFFF; /* white background */
    static constexpr QRgb FG = 0xFF000000; /* opaque black foreground */

    /*  Render one PaintCell call into a fresh SZ x SZ QImage filled
        with pure white. Returns the image. */
    static QImage render(DARSTELLUNG d, int number = -1)
    {
        QImage img(SZ, SZ, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        PaintCell(p, d, 0, 0, SZ, SZ, QColor(Qt::black), /*dontclear=*/false, number,
                  QColor(Qt::white));
        return img;
    }

    /*  Count foreground (black) pixels in the image. Useful as a
        sanity check that *something* was drawn without pinning down
        exact pixel layout. */
    static int countFG(const QImage& img)
    {
        int n = 0;
        for (int y = 0; y < img.height(); y++)
            for (int x = 0; x < img.width(); x++)
                if (img.pixel(x, y) == FG)
                    n++;
        return n;
    }

private slots:

    void clear_cell_fills_background()
    {
        QImage img(SZ, SZ, QImage::Format_ARGB32);
        img.fill(Qt::red);
        {
            QPainter p(&img);
            ClearCell(p, 0, 0, SZ, SZ, QColor(Qt::white));
        }
        /*  ClearCell covers (1, 1) .. (SZ-1, SZ-1) exclusive; the
            top and left edge rows stay red (legacy inclusive-exclusive
            rectangle semantics). */
        QCOMPARE(img.pixel(0, 0), (QRgb)0xFFFF0000); /* untouched */
        QCOMPARE(img.pixel(1, 1), BG);
        QCOMPARE(img.pixel(SZ - 2, SZ - 2), BG);
    }

    void ausgefuellt_paints_interior_block()
    {
        QImage img = render(AUSGEFUELLT);
        QCOMPARE(img.pixel(SZ / 2, SZ / 2), FG); /* centre filled */
        QCOMPARE(img.pixel(0, 0), BG);           /* outer ring not */
    }

    void strich_paints_two_center_vertical_lines()
    {
        QImage img = render(STRICH);
        const int cx = SZ / 2;
        QCOMPARE(img.pixel(cx, SZ / 2), FG);
        QCOMPARE(img.pixel(cx + 1, SZ / 2), FG);
        QCOMPARE(img.pixel(cx - 3, SZ / 2), BG);
        QCOMPARE(img.pixel(cx + 4, SZ / 2), BG);
    }

    void kreuz_paints_two_diagonals()
    {
        QImage img = render(KREUZ);
        /*  Both diagonals cross near the centre. */
        const int fg = countFG(img);
        QVERIFY2(fg > 2 * (SZ - 4),
                 qPrintable(
                     QString("expected at least %1 FG pixels, got %2").arg(2 * (SZ - 4)).arg(fg)));
        /*  Corners outside the diagonals stay background. */
        QCOMPARE(img.pixel(0, 0), BG);
        QCOMPARE(img.pixel(SZ - 1, 0), BG);
    }

    void punkt_paints_small_center_dash()
    {
        QImage img = render(PUNKT);
        const int cx = SZ / 2;
        const int cy = SZ / 2;
        QCOMPARE(img.pixel(cx, cy), FG);
        QCOMPARE(img.pixel(cx + 1, cy), FG);
        QCOMPARE(img.pixel(cx + 2, cy), FG);
        /*  Well away from the dash is background. */
        QCOMPARE(img.pixel(1, 1), BG);
        QCOMPARE(img.pixel(SZ - 2, SZ - 2), BG);
    }

    void kreis_paints_ellipse_outline()
    {
        QImage img = render(KREIS);
        /*  Centre of the inscribed ellipse is empty (outline only). */
        QCOMPARE(img.pixel(SZ / 2, SZ / 2), BG);
        /*  Something was drawn. */
        QVERIFY(countFG(img) > 0);
    }

    void steigend_paints_single_rising_diagonal()
    {
        QImage img = render(STEIGEND);
        /*  Rising diagonal: low-left corner pixel lit, high-right
            corner pixel lit, falling diagonal corners not. */
        const int fg = countFG(img);
        QVERIFY(fg > 0);
        QCOMPARE(img.pixel(2, SZ - 2), FG);
        QCOMPARE(img.pixel(SZ - 2, SZ - 2), BG); /* bottom-right not on rising line */
    }

    void fallend_paints_single_falling_diagonal()
    {
        QImage img = render(FALLEND);
        const int fg = countFG(img);
        QVERIFY(fg > 0);
        QCOMPARE(img.pixel(2, 2), FG);
        QCOMPARE(img.pixel(SZ - 2, 2), BG); /* top-right not on falling line */
    }

    void smallkreuz_large_cell_draws_smaller_x()
    {
        /*  At SZ=20 the "large" branch is taken. */
        QImage img = render(SMALLKREUZ);
        QVERIFY(countFG(img) > 0);
        /*  The corners of the full 20x20 cell are NOT drawn on
            (the smaller x ends at _xx-3 = 17). */
        QCOMPARE(img.pixel(2, SZ - 2), BG);
    }

    void smallkreis_large_cell_draws_inset_ellipse()
    {
        QImage img = render(SMALLKREIS);
        /*  Inset ellipse: centre empty, interior pixels off the
            outline are background. */
        QCOMPARE(img.pixel(SZ / 2, SZ / 2), BG);
        QVERIFY(countFG(img) > 0);
    }

    void number_negative_falls_back_to_filled_rect()
    {
        /*  _number < 0 is the legacy fallback that paints a solid
            block (same visual as AUSGEFUELLT).                   */
        QImage img = render(NUMBER, /*number=*/-1);
        QCOMPARE(img.pixel(SZ / 2, SZ / 2), FG);
    }
};

QTEST_MAIN(TestDrawCell)
#include "test_draw_cell.moc"
