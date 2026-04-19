/*  DB-WEAVE Qt 6 port - domain/zentralsymm tests */

#include <QTest>

#include "zentralsymm.h"

class TestZentralSymm : public QObject
{
    Q_OBJECT

private slots:

    void already_symmetric_1x1()
    {
        ZentralSymmChecker z(1, 1);
        z.SetData(0, 0, 1);
        QVERIFY(z.IsAlreadySymmetric());
    }

    void symmetric_3x3_checkerboard()
    {
        /*  Point-symmetric pattern (value at (i,j) equals value at
            (n-1-i, n-1-j)). */
        ZentralSymmChecker z(3, 3);
        const char p[3][3] = {
            { 1, 2, 3 },
            { 4, 5, 4 },
            { 3, 2, 1 },
        };
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                z.SetData(i, j, p[i][j]);
        QVERIFY(z.IsAlreadySymmetric());
    }

    void not_symmetric_but_shiftable()
    {
        /*  A horizontally-shifted palindrome pattern: not symmetric
            in-place, but SearchSymmetry() rolls it left until
            data becomes [0,1,1,0] which IS point-symmetric (a==d,
            b==c). */
        ZentralSymmChecker z(4, 1);
        z.SetData(0, 0, 1);
        z.SetData(1, 0, 1);
        z.SetData(2, 0, 0);
        z.SetData(3, 0, 0);
        QVERIFY(!z.IsAlreadySymmetric());
        QVERIFY(z.SearchSymmetry());
    }

    void asymmetric_never_becomes_symmetric()
    {
        /*  No rotation of (1, 2, 3, 0) is point-symmetric. */
        ZentralSymmChecker z(4, 1);
        z.SetData(0, 0, 1);
        z.SetData(1, 0, 2);
        z.SetData(2, 0, 3);
        z.SetData(3, 0, 0);
        QVERIFY(!z.IsAlreadySymmetric());
        QVERIFY(!z.SearchSymmetry());
    }
};

QTEST_APPLESS_MAIN(TestZentralSymm)
#include "test_zentralsymm.moc"
