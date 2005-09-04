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
#ifndef QTESTASSERT_H
#define QTESTASSERT_H

#include <QtCore/qglobal.h>

#define QTEST_ASSERT(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)

#define QTEST_ASSERT_X(cond, where, what) do {if(!(cond))qt_assert_x(where, what,__FILE__,__LINE__);} while (0)

#endif

