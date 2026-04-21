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
/*  Persistent application settings under QSettings. Domain-level
    wrapper that bakes the legacy registry base path into every
    key so ported DB-WEAVE and legacy DB-WEAVE Windows binaries
    share the same HKCU\Software\Brunold Software\DB-WEAVE hive.
    Categorised by a caller-settable string (Environment, LastDir,
    Display, ...); keys under that category are the leaves.      */
/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_DOMAIN_SETTINGS_H
#define DBWEAVE_DOMAIN_SETTINGS_H
/*-----------------------------------------------------------------*/
#include <QString>
/*-----------------------------------------------------------------*/
class Settings
{
protected:
    QString category;

public:
    Settings();
    virtual ~Settings();

    void SetCategory(const QString& _category)
    {
        category = _category;
    }
    QString Category() const
    {
        return category;
    }

    int Load(const QString& _name, int _default = 0);
    QString Load(const QString& _name, const QString& _default = QString());

    void Save(const QString& _name, int _value);
    void Save(const QString& _name, const QString& _value);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
