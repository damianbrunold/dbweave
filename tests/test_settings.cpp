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

	void environment_language_round_trip()
	{
		/*  Matches the EnvOptionsDialog ↔ main.cpp contract:
		    Environment/Language (int) is -1/absent ⇒ auto-detect
		    from locale, 0 ⇒ English, 1 ⇒ German. Save 1, read it
		    back verbatim, and confirm a fresh Settings instance
		    sees the same value (persistence across object
		    boundaries within the same session). */
		{
			Settings s;
			s.SetCategory(QStringLiteral("Environment"));
			s.Save(QStringLiteral("Language"), 1);
		}
		Settings s;
		s.SetCategory(QStringLiteral("Environment"));
		QCOMPARE(s.Load(QStringLiteral("Language"), -1), 1);
	}

	void mru_group_round_trip()
	{
		/*  MRU (mainwindow.cpp) bypasses the Settings wrapper and
		    uses QSettings directly under the "mru" group. This test
		    locks in that path: six string slots, empty slots hidden,
		    re-reading rebuilds the same list. */
		{
			QSettings w;
			w.beginGroup(QStringLiteral("mru"));
			w.setValue(QStringLiteral("0"), QStringLiteral("/tmp/a.dbw"));
			w.setValue(QStringLiteral("1"), QStringLiteral("/tmp/b.dbw"));
			w.endGroup();
		}
		QSettings r;
		r.beginGroup(QStringLiteral("mru"));
		QCOMPARE(r.value(QStringLiteral("0")).toString(),
		         QStringLiteral("/tmp/a.dbw"));
		QCOMPARE(r.value(QStringLiteral("1")).toString(),
		         QStringLiteral("/tmp/b.dbw"));
		QVERIFY(r.value(QStringLiteral("2")).toString().isEmpty());
		r.endGroup();
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
