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
#ifndef QTTEST_GLOBAL_H
#define QTTEST_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QTESTLIB_MAKEDLL

  // We can't just use Q_DECL_EXPORT/Q_DECL_IMPORT. Qt might have no hidden-visibility
  // even when kdelibs has it.
  #include <kdelibs_export.h>
  //# define Q_TESTLIB_EXPORT Q_DECL_EXPORT
  #define Q_TESTLIB_EXPORT KDE_EXPORT

#else

  #define Q_TESTLIB_EXPORT Q_DECL_IMPORT
  // But it's nice if kde's qttestlib is useable for qt-only progs too,
  // so let's not require kdelibs_export.h here.
  //#define Q_TESTLIB_EXPORT KDE_IMPORT

#endif

#if defined (Q_CC_MSVC) || defined (Q_CC_SUN) || defined (Q_CC_XLC) || (defined (Q_CC_GNU) && (__GNUC__ - 0 < 3))
# define QTEST_NO_PARTIAL_SPECIALIZATIONS
#endif

#define QTTEST_VERSION     0x020001
#define QTTEST_VERSION_STR "2.0.1-kde"

namespace QtTest
{
    enum SkipMode { SkipSingle = 1, SkipAll = 2 };
    enum TestFailMode { Abort = 1, Continue = 2 };

    int Q_TESTLIB_EXPORT qt_snprintf(char *str, int size, const char *format, ...);
    int Q_TESTLIB_EXPORT defaultEventDelay();
}

#endif

