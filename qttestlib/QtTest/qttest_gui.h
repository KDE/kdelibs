/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#ifndef QTEST_GUI_H
#define QTEST_GUI_H

#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

#include "QtTest/qttestassert.h"
#include "QtTest/qttest.h"
#include "QtTest/qttestevent.h"
#include "QtTest/qttestmouse.h"
#include "QtTest/qttestkeyboard.h"

namespace QtTest
{

template<>
inline bool compare(QIcon const &t1, QIcon const &t2, const char *file, int line)
{
    QTEST_ASSERT(sizeof(QIcon) == sizeof(void *));
    return compare<void *>(*reinterpret_cast<void * const *>(&t1),
                   *reinterpret_cast<void * const *>(&t2), file, line);
}

template<>
inline bool compare(QPixmap const &t1, QPixmap const &t2, const char *file, int line)
{
    return compare(t1.toImage(), t2.toImage(), file, line);
}

}

/* compatibility */

inline static bool pixmapsAreEqual(const QPixmap *actual, const QPixmap *expected)
{
    if (!actual && !expected)
        return true;
    if (!actual || !expected)
        return false;
    if (actual->isNull() && expected->isNull())
        return true;
    if (actual->isNull() || expected->isNull() || actual->size() != expected->size())
        return false;
    return actual->toImage() == expected->toImage();
}

#endif
