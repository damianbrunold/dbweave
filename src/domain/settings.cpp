/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#include "settings.h"
#include "regbase.h"

#include <QSettings>
/*-----------------------------------------------------------------*/
/*  Compose the full QSettings key for `_name` under the current
    category. DBW_REGBASE baked in so ported and legacy binaries
    share the same `Software\Brunold Software\DB-WEAVE\<category>`
    subtree in the Windows registry. Backslashes come from the
    legacy constant; QSettings uses forward slashes, so replace
    once and trim the leading slash.                             */
static QString fullKey(const QString& _category, const QString& _name)
{
    QString k = QStringLiteral(DBW_REGBASE) + _category + QStringLiteral("\\") + _name;
    k.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (k.startsWith(QLatin1Char('/')))
        k.remove(0, 1);
    return k;
}
/*-----------------------------------------------------------------*/
Settings::Settings()
    : category(QStringLiteral("General"))
{
}
/*-----------------------------------------------------------------*/
Settings::~Settings() = default;
/*-----------------------------------------------------------------*/
int Settings::Load(const QString& _name, int _default /*=0*/)
{
    QSettings s;
    return s.value(fullKey(category, _name), _default).toInt();
}
/*-----------------------------------------------------------------*/
QString Settings::Load(const QString& _name, const QString& _default /*=""*/)
{
    QSettings s;
    return s.value(fullKey(category, _name), _default).toString();
}
/*-----------------------------------------------------------------*/
void Settings::Save(const QString& _name, int _value)
{
    QSettings s;
    s.setValue(fullKey(category, _name), _value);
}
/*-----------------------------------------------------------------*/
void Settings::Save(const QString& _name, const QString& _value)
{
    QSettings s;
    s.setValue(fullKey(category, _name), _value);
}
/*-----------------------------------------------------------------*/
