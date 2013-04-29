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

#include <QtQml/QQmlEngine>

#include <kdeclarative/kdeclarative_export.h>

#include <QStringList>

class QQmlEngine;
class QScriptEngine;

class KDeclarativePrivate;

class KDECLARATIVE_EXPORT KDeclarative
{
public:
    explicit KDeclarative();
    ~KDeclarative();

    void initialize();
    void setupBindings();

    void setDeclarativeEngine(QQmlEngine *engine);
    QQmlEngine *declarativeEngine() const;

    /**
     * This method must be called very early at startup time to ensure the
     * QQuickDebugger is enabled. Ideally it should be called in main(),
     * after command-line options are defined.
     */
    static void setupQmlJsDebugger();

    /**
     * @return the runtime platform, e.g. "desktop" or "tablet, touch". The first entry/ies in
     *         the list relate to the platform formfactor and the last is the input method
     *         specialization. If the string is empty, there is no specified runtime platform
     *         and a traditional desktop environment may be assumed
     * @since 4.10
     */
    static QStringList runtimePlatform();

    /**
     * @return the QML components target, based on the runtime platform. e.g. touch or desktop
     * @since 4.10
     */
    static QString componentsTarget();

    /**
     * @return the default components target; can be used to compare against the returned value
     *         from @see componentsTarget()
     * @since 4.10
     */
    static QString defaultComponentsTarget();

private:
    KDeclarativePrivate *const d;
};

#endif
