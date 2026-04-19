/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "blockmuster.h"
#include "assert_compat.h"

#include <cstring>

Muster::Muster()
{
    Clear();
}

void Muster::Set(int _i, int _j, char _value)
{
    dbw3_assert(_i < maxx);
    dbw3_assert(_j < maxy);
    if (_i >= maxx || _j >= maxy)
        return;
    feld[_i + maxx * _j] = _value;
}

char Muster::Get(int _i, int _j)
{
    dbw3_assert(_i < maxx);
    dbw3_assert(_j < maxy);
    if (_i >= maxx || _j >= maxy)
        return 0;
    return feld[_i + maxx * _j];
}

void Muster::Clear()
{
    std::memset(feld, 0, maxx * maxy);
}

bool Muster::IsEmpty()
{
    for (int i = 0; i < maxx; i++)
        for (int j = 0; j < maxy; j++)
            if (feld[i + maxx * j] != 0)
                return false;
    return true;
}

Muster& Muster::operator=(const Muster& _m)
{
    if (this == &_m)
        return *this;
    SetData(_m.Data());
    return *this;
}

void Muster::SetData(const char* _data)
{
    std::memcpy(feld, _data, maxx * maxy);
}

int Muster::SizeX()
{
    bool nonempty = false;
    int i1 = maxx - 1, i2 = 0;
    for (int i = 0; i < maxx; i++)
        for (int j = 0; j < maxy; j++)
            if (feld[i + maxx * j] != 0) {
                if (i1 > i)
                    i1 = i;
                if (i2 < i)
                    i2 = i;
                nonempty = true;
            }
    return nonempty ? i2 - i1 + 1 : 0;
}

int Muster::SizeY()
{
    bool nonempty = false;
    int j1 = maxy - 1, j2 = 0;
    for (int j = 0; j < maxy; j++)
        for (int i = 0; i < maxx; i++)
            if (feld[i + maxx * j] != 0) {
                if (j1 > j)
                    j1 = j;
                if (j2 < j)
                    j2 = j;
                nonempty = true;
            }
    return nonempty ? j2 - j1 + 1 : 0;
}

int Muster::FirstX()
{
    for (int i = 0; i < maxx; i++)
        for (int j = 0; j < maxy; j++)
            if (feld[i + maxx * j] != 0)
                return i;
    return 0;
}

int Muster::FirstY()
{
    for (int j = 0; j < maxy; j++)
        for (int i = 0; i < maxx; i++)
            if (feld[i + maxx * j] != 0)
                return j;
    return 0;
}

/*-----------------------------------------------------------------*/
BlockUndo::BlockUndo(PMUSTERARRAY _bindungen, int& _active)
    : bindungen(_bindungen)
    , active(_active)
{
    list = current = nullptr;
    Allocate();
}

BlockUndo::~BlockUndo()
{
    BlockUndoItem* p = list->prev;
    while (p != list) {
        p = p->prev;
        delete p->next;
    }
    delete list;
    bindungen = nullptr;
}

void BlockUndo::Allocate()
{
    try {
        list = current = new BlockUndoItem;
        for (int i = 1; i < 100; i++) {
            current->next = new BlockUndoItem;
            current->next->prev = current;
            current = current->next;
        }
        list->prev = current;
        current->next = list;
    } catch (...) {
    }
}

void BlockUndo::Snapshot()
{
    dbw3_assert(current != nullptr);
    dbw3_assert(list != nullptr);
    if (!current->isempty) {
        if (current == list->prev) {
            list = list->next;
            current = current->next;
        } else {
            current = current->next;
        }
        current->isempty = true;
    }

    if (current && current->isempty) {
        for (int i = 0; i < 10; i++)
            current->bindungen[i] = (*bindungen)[i];
        current->active = active;
        current->isempty = false;
        if (current->next != list)
            current->next->isempty = true;
    }
}

void BlockUndo::Undo()
{
    dbw3_assert(current != nullptr);
    dbw3_assert(list != nullptr);
    if (current != list) {
        current = current->prev;
        if (current->isempty)
            return;
        for (int i = 0; i < 10; i++)
            (*bindungen)[i] = current->bindungen[i];
        active = current->active;
    }
}

void BlockUndo::Redo()
{
    dbw3_assert(current != nullptr);
    dbw3_assert(list != nullptr);
    if (current != list->prev) {
        current = current->next;
        if (current->isempty)
            return;
        for (int i = 0; i < 10; i++)
            (*bindungen)[i] = current->bindungen[i];
        active = current->active;
    }
}

bool BlockUndo::CanUndo()
{
    return (current != list && !current->prev->isempty);
}

bool BlockUndo::CanRedo()
{
    return (current->next != list && !current->next->isempty);
}

void BlockUndo::Clear()
{
    dbw3_assert(current != nullptr);
    dbw3_assert(list != nullptr);
    current = list;
    do {
        current->isempty = true;
        current = current->next;
    } while (current != list);
}
