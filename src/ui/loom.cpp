/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loom.h"

#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QThread>

/*-----------------------------------------------------------------*/
StWeaveController::StWeaveController() = default;
StWeaveController::~StWeaveController() = default;

bool StWeaveController::Initialize(const INITDATA&)
{
    aborted = false;
    return true;
}

void StWeaveController::Terminate() { }

void StWeaveController::Abort()
{
    aborted = true;
}

void StWeaveController::CheckAbort()
{
    QCoreApplication::processEvents();
    if (aborted)
        throw 0; /* Matches the legacy `throw "aborted"` sentinel. */
}

/*-----------------------------------------------------------------*/
bool StDummyController::Initialize(const INITDATA& _data)
{
    return StWeaveController::Initialize(_data);
}

void StDummyController::Terminate()
{
    /*  No physical port to close. */
}

WEAVE_STATUS StDummyController::WeaveSchuss(std::uint32_t /*_shafts*/)
{
    /*  Simulated loom: wait for waitMs, pumping events every
        20 ms so the UI thread stays responsive and Abort is seen
        promptly. Always reports forward-progress.               */
    QDeadlineTimer deadline(waitMs);
    while (!deadline.hasExpired()) {
        try {
            CheckAbort();
        } catch (...) {
            return WEAVE_REPEAT;
        }
        QThread::msleep(20);
    }
    return WEAVE_SUCCESS_NEXT;
}
