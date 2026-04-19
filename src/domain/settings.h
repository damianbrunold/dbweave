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
#ifndef DBWEAVE_DOMAIN_SETTINGS_H
#define DBWEAVE_DOMAIN_SETTINGS_H
/*-----------------------------------------------------------------*/
#include "vcl_compat.h"      /* AnsiString -> QString */
#include "registry_compat.h" /* TRegistry -> QSettings */
/*-----------------------------------------------------------------*/
class Settings
{
protected:
    AnsiString category;
    TRegistry* registry;

public:
    Settings();
    virtual ~Settings();

    void SetCategory(const AnsiString& _category)
    {
        category = _category;
    }
    AnsiString Category() const
    {
        return category;
    }

    int Load(const AnsiString& _name, int _default = 0);
    AnsiString Load(const AnsiString& _name, const AnsiString& _default = QString());

    void Save(const AnsiString& _name, int _value);
    void Save(const AnsiString& _name, const AnsiString& _value);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
