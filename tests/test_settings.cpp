/*  DB-WEAVE Qt 6 port - domain/settings tests */

#include <QCoreApplication>
#include <QSettings>
#include <QTest>

#include "settings.h"

class TestSettings : public QObject
{
	Q_OBJECT

private slots:

	void init()
	{
		QCoreApplication::setOrganizationName(QStringLiteral("DbweaveTest"));
		QCoreApplication::setApplicationName  (QStringLiteral("Settings"));
		/*  Start from a clean slate each test. */
		QSettings().clear();
	}

	void default_category_is_general()
	{
		Settings s;
		QCOMPARE(s.Category(), QStringLiteral("General"));
	}

	void save_and_load_integer()
	{
		Settings s;
		s.Save(QStringLiteral("zoom"), 400);
		QCOMPARE(s.Load(QStringLiteral("zoom"), 0), 400);
	}

	void save_and_load_string()
	{
		Settings s;
		s.Save(QStringLiteral("lang"), QStringLiteral("de"));
		QCOMPARE(s.Load(QStringLiteral("lang"), QStringLiteral("")),
		         QStringLiteral("de"));
	}

	void default_returned_for_missing_key()
	{
		Settings s;
		QCOMPARE(s.Load(QStringLiteral("missing_int"), 77), 77);
		QCOMPARE(s.Load(QStringLiteral("missing_str"), QStringLiteral("fallback")),
		         QStringLiteral("fallback"));
	}

	void category_namespaces_values()
	{
		Settings s;
		s.SetCategory(QStringLiteral("Foo"));
		s.Save(QStringLiteral("k"), 1);

		s.SetCategory(QStringLiteral("Bar"));
		s.Save(QStringLiteral("k"), 2);

		s.SetCategory(QStringLiteral("Foo"));
		QCOMPARE(s.Load(QStringLiteral("k"), 0), 1);
		s.SetCategory(QStringLiteral("Bar"));
		QCOMPARE(s.Load(QStringLiteral("k"), 0), 2);
	}
};

QTEST_MAIN(TestSettings)
#include "test_settings.moc"
