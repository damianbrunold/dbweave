#include <QTest>
#include <QString>

class TestSmoke : public QObject
{
    Q_OBJECT

private slots:
    void qstring_basic()
    {
        QString s = QStringLiteral("DB-WEAVE");
        QCOMPARE(s.length(), 8);
        QVERIFY(s.startsWith(QLatin1String("DB")));
    }
};

QTEST_APPLESS_MAIN(TestSmoke)
#include "test_smoke.moc"
