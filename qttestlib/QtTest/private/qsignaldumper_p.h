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
#ifndef QSIGNALDUMPER_H
#define QSIGNALDUMPER_H

class QByteArray;

class QSignalDumper
{
public:
    static void startDump();
    static void endDump();

    static void ignoreClass(const QByteArray &klass);
    static void clearIgnoredClasses();
};

#endif

