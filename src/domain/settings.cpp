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
#include "vcl_compat.h"
#include "assert_compat.h"
/*-----------------------------------------------------------------*/
#include "settings.h"
#include "regbase.h"
/*-----------------------------------------------------------------*/
Settings::Settings()
{
    category = QStringLiteral("General");
    try {
        registry = new TRegistry;
        registry->RootKey = HKEY_CURRENT_USER;
    } catch (...) {
        registry = NULL;
    }
}
/*-----------------------------------------------------------------*/
Settings::~Settings()
{
    delete registry;
}
/*-----------------------------------------------------------------*/
int Settings::Load(const AnsiString& _name, int _default /*=0*/)
{
    dbw3_assert(registry);
    int value = _default;
    try {
        AnsiString key = AnsiString(DBW_REGBASE) + category;
        if (registry->OpenKey(key, false)) {
            if (registry->ValueExists(_name))
                value = registry->ReadInteger(_name);
        }
    } catch (...) {
    }
    registry->CloseKey();
    return value;
}
/*-----------------------------------------------------------------*/
AnsiString Settings::Load(const AnsiString& _name, const AnsiString& _default /*=""*/)
{
    dbw3_assert(registry);
    AnsiString value = _default;
    try {
        AnsiString key = AnsiString(DBW_REGBASE) + category;
        if (registry->OpenKey(key, false)) {
            if (registry->ValueExists(_name))
                value = registry->ReadString(_name);
        }
    } catch (...) {
    }
    registry->CloseKey();
    return value;
}
/*-----------------------------------------------------------------*/
void Settings::Save(const AnsiString& _name, int _value)
{
    try {
        AnsiString key = AnsiString(DBW_REGBASE) + category;
        if (registry->OpenKey(key, true)) {
            registry->WriteInteger(_name, _value);
        }
    } catch (...) {
    }
    registry->CloseKey();
}
/*-----------------------------------------------------------------*/
void Settings::Save(const AnsiString& _name, const AnsiString& _value)
{
    try {
        AnsiString key = AnsiString(DBW_REGBASE) + category;
        if (registry->OpenKey(key, true)) {
            registry->WriteString(_name, _value);
        }
    } catch (...) {
    }
    registry->CloseKey();
}
/*-----------------------------------------------------------------*/
