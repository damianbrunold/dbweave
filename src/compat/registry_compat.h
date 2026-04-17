/*  DB-WEAVE Qt 6 port - VCL TRegistry compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Thin TRegistry-compatible wrapper backed by QSettings. The legacy
    dbweave code uses a small subset: RootKey assignment, OpenKey,
    CloseKey, ValueExists, ReadInteger / WriteInteger, ReadString /
    WriteString. That is all that is implemented here.

    QSettings stores per-platform: Windows registry, Linux .conf under
    $XDG_CONFIG_HOME, macOS plist. This means on Windows the port keeps
    using the registry (same location, via the application's organisation
    /product name), while Linux and macOS gain native conventions -- the
    user-visible behaviour is unchanged on Windows.

    RootKey is accepted but ignored: we always store under the application
    scope set via QApplication::setOrganizationName / setApplicationName
    in main.cpp. This matches the spirit of the legacy HKEY_CURRENT_USER
    default.
*/

#ifndef DBWEAVE_COMPAT_REGISTRY_COMPAT_H
#define DBWEAVE_COMPAT_REGISTRY_COMPAT_H

#include <QSettings>
#include <QString>
#include <QVariant>

/*  Opaque HKEY placeholder -- the legacy code assigns HKEY_CURRENT_USER
    to RootKey; we accept any value and store nothing about it. */
using DbwHKEY = unsigned long;
static constexpr DbwHKEY HKEY_CURRENT_USER_COMPAT = 0x80000001UL;
#ifndef HKEY_CURRENT_USER
#   define HKEY_CURRENT_USER HKEY_CURRENT_USER_COMPAT
#endif

class TRegistry
{
public:
	DbwHKEY RootKey = HKEY_CURRENT_USER_COMPAT;

	TRegistry() = default;
	~TRegistry() = default;

	bool OpenKey (const QString& _key, bool _createIfMissing = false)
	{
		(void)_createIfMissing;
		currentKey = normalise(_key);
		return true;
	}

	void CloseKey()
	{
		currentKey.clear();
	}

	bool ValueExists (const QString& _name) const
	{
		return settings.contains(fullKey(_name));
	}

	int ReadInteger (const QString& _name) const
	{
		return settings.value(fullKey(_name), 0).toInt();
	}

	QString ReadString (const QString& _name) const
	{
		return settings.value(fullKey(_name), QString()).toString();
	}

	void WriteInteger (const QString& _name, int _value)
	{
		settings.setValue(fullKey(_name), _value);
	}

	void WriteString (const QString& _name, const QString& _value)
	{
		settings.setValue(fullKey(_name), _value);
	}

	void Sync() { settings.sync(); }

private:
	mutable QSettings settings;
	QString           currentKey;

	static QString normalise (const QString& _key)
	{
		/*  Backslashes are the VCL registry separator; QSettings uses
		    forward slashes. Normalise and strip any leading slash. */
		QString k = _key;
		k.replace(QLatin1Char('\\'), QLatin1Char('/'));
		while (k.startsWith(QLatin1Char('/'))) k.remove(0, 1);
		return k;
	}

	QString fullKey (const QString& _name) const
	{
		return currentKey.isEmpty() ? _name : currentKey + QLatin1Char('/') + _name;
	}
};

#endif /* DBWEAVE_COMPAT_REGISTRY_COMPAT_H */
