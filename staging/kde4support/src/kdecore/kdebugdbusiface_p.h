/* This file is part of the KDE libraries
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEBUGDBUSIFACE_H
#define KDEBUGDBUSIFACE_H

#include <kdecore_export.h>
#include <QtCore/qobject.h>

/**
 * @short D-Bus interface to KDebug.
 **/
class KDebugDBusIface: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KDebug")
public:
    KDebugDBusIface();
    ~KDebugDBusIface();

public Q_SLOTS:
    /**
     * The kdebugrc has been changed and should be reparsed now.
     * This will simply call kClearDebugConfig
     **/
    Q_SCRIPTABLE void notifyKDebugConfigChanged();

    /**
     * Print out a kBacktrace. Useful when trying to understand why
     * a dialog is popping up, without having to launch gdb
     */
    Q_SCRIPTABLE void printBacktrace();
};

#endif
