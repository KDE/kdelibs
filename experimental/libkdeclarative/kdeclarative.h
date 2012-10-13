/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KDECLARATIVE_H
#define KDECLARATIVE_H

#include <kdeclarative_export.h>

class QDeclarativeEngine;
class QScriptEngine;

class KDeclarativePrivate;

class KDECLARATIVE_EXPORT KDeclarative
{
public:
    explicit KDeclarative();
    ~KDeclarative();

    void initialize();
    void setupBindings();

    void setDeclarativeEngine(QDeclarativeEngine *engine);
    QDeclarativeEngine *declarativeEngine() const;

    QScriptEngine *scriptEngine() const;

    /**
     * This method must be called very early at startup time to ensure the
     * QDeclarativeDebugger is enabled. Ideally it should be called in main(),
     * after command-line options are defined.
     */
    static void setupQmlJsDebugger();

private:
    KDeclarativePrivate *const d;
    friend class EngineAccess;
};

#endif
