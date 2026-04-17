/*  DB-WEAVE Qt 6 port - dbw3_assert / dbw3_trace compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Legacy dbw3_assert / dbw3_trace mapped onto the Qt logging primitives.

    The original implementations called Win32 ::MessageBox and
    ::OutputDebugString behind an #ifdef _DEBUG guard. For the port we
    route assertions through Q_ASSERT (which aborts in debug builds and
    compiles out in release) and traces through qDebug() so they are
    visible in the Qt logging category framework.

    This header sits in compat/ because it exists only to let legacy code
    include "assert.h" unchanged. It will be deleted in Phase 12 once
    call sites are migrated to idiomatic Q_ASSERT / qDebug directly.
*/

#ifndef DBWEAVE_COMPAT_ASSERT_COMPAT_H
#define DBWEAVE_COMPAT_ASSERT_COMPAT_H

#include <QtGlobal>
#include <QDebug>

#define dbw3_assert(c)      Q_ASSERT(c)
#define dbw3_trace(c)       do { qDebug() << (c); } while (0)
#define dbw3_trace2(lvl, c) do { qDebug() << "[" << (lvl) << "]" << (c); } while (0)

#endif /* DBWEAVE_COMPAT_ASSERT_COMPAT_H */
