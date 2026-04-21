/*  DB-WEAVE - dbw3_assert / dbw3_trace helpers
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  dbw3_assert / dbw3_trace mapped onto Qt logging primitives:
    Q_ASSERT (aborts in debug, compiles out in release) and qDebug()
    (visible in the Qt logging category framework).
*/

#ifndef DBWEAVE_COMPAT_ASSERT_COMPAT_H
#define DBWEAVE_COMPAT_ASSERT_COMPAT_H

#include <QtGlobal>
#include <QDebug>

#define dbw3_assert(c) Q_ASSERT(c)
#define dbw3_trace(c)    \
    do {                 \
        qDebug() << (c); \
    } while (0)
#define dbw3_trace2(lvl, c)                     \
    do {                                        \
        qDebug() << "[" << (lvl) << "]" << (c); \
    } while (0)

#endif /* DBWEAVE_COMPAT_ASSERT_COMPAT_H */
