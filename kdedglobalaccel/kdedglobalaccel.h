/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

/*
data per main component:
-unique name
-i18n name
-(KActions)

data per KAction:
-unique name
-i18n name
-shortcut
-default shortcut (special purpose API for KCMs)

 */

#ifndef KDEDGLOBALACCEL_H
#define KDEDGLOBALACCEL_H

#include <kdedmodule.h>
#include "kaction.h"
#include <QtCore/QStringList>
#include <QtCore/QList>

class KdedGlobalAccelPrivate;

Q_DECLARE_METATYPE(QList<QStringList>)
Q_DECLARE_METATYPE(QList<int>)

class KdedGlobalAccel : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KdedGlobalAccel")
    Q_ENUMS(SetShortcutFlag)

public:

    enum SetShortcutFlag
    {
        SetPresent = 2,
        NoAutoloading = 4,
        IsDefault = 8
    };
    Q_FLAGS(SetShortcutFlags)

    KdedGlobalAccel(QObject*, const QList<QVariant>&);
    ~KdedGlobalAccel();

//All of the following public methods and signals are part of the DBus interface
public Q_SLOTS:

    Q_SCRIPTABLE QList<QStringList> allMainComponents() const;

    Q_SCRIPTABLE QList<QStringList> allActionsForComponent(const QStringList &actionId) const;

    Q_SCRIPTABLE QStringList action(int key) const;

    //to be called by main components not owning the action
    Q_SCRIPTABLE QList<int> shortcut(const QStringList &actionId) const;

    //to be called by main components not owning the action
    Q_SCRIPTABLE QList<int> defaultShortcut(const QStringList &actionId) const;

    //to be called by main components owning the action
    Q_SCRIPTABLE QList<int> setShortcut(const QStringList &actionId,
                           const QList<int> &keys, uint flags);

    //this is used if application A wants to change shortcuts of application B
    Q_SCRIPTABLE void setForeignShortcut(const QStringList &actionId, const QList<int> &keys);

    //to be called when a KAction is destroyed. The shortcut stays in the data structures for
    //conflict resolution but won't trigger.
    Q_SCRIPTABLE void setInactive(const QStringList &actionId);

    Q_SCRIPTABLE void doRegister(const QStringList &actionId);

    Q_SCRIPTABLE void unRegister(const QStringList &actionId);

Q_SIGNALS:

    // this is qlonglong because manually written adaptor is used and just long doesn't work
    Q_SCRIPTABLE void invokeAction(const QStringList &actionId, qlonglong timestamp);
    Q_SCRIPTABLE void yourShortcutGotChanged(const QStringList &actionId, const QList<int> &newKeys);

private Q_SLOTS:

    void writeSettings() const;

private:

    void loadSettings();
    void scheduleWriteSettings() const;

    QList<int> keysFromString(const QString &str) const;
    QString stringFromKeys(const QList<int> &keys) const;


    friend class KGlobalAccelImpl;

    //called by the implementation to inform us about key presses
    //returns true if the key was handled
    bool keyPressed(int keyQt);


    KdedGlobalAccelPrivate *const d;
};

#endif //KDEDGLOBALACCEL_H
