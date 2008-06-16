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

struct actionData;
class KdedGlobalAccelPrivate;

class KdedGlobalAccel : public KDEDModule
{
    Q_OBJECT
public:
    enum SetShortcutFlags
    {
        SetPresent = 2,
        NoAutoloading = 4,
        IsDefault = 8
    };

    KdedGlobalAccel(QObject*, const QList<QVariant>&);
    ~KdedGlobalAccel();

//All of the following public methods and signals are part of the DBus interface
    QList<QStringList> allComponents();
    QList<QStringList> allActionsForComponent(const QStringList &actionId);

    QList<int> allKeys();
    QStringList allKeysAsString();

    QStringList actionId(int key);
    //to be called by main components not owning the action
    QList<int> shortcut(const QStringList &actionId);
    //to be called by main components not owning the action
    QList<int> defaultShortcut(const QStringList &actionId);
    //to be called by main components owning the action
    QList<int> setShortcut(const QStringList &actionId,
                           const QList<int> &keys, uint flags);
    //this is used if application A wants to change shortcuts of application B
    void setForeignShortcut(const QStringList &actionId, const QList<int> &keys);
    //to be called when a KAction is destroyed. The shortcut stays in the data structures for
    //conflict resolution but won't trigger.
    void setInactive(const QStringList &actionId);

    void doRegister(const QStringList &actionId);

    void unRegister(const QStringList &actionId);

    //called by the implementation to inform us about key presses
    //returns true if the key was handled
    bool keyPressed(int keyQt);

Q_SIGNALS:
    // this is qlonglong because manually written adaptor is used and just long doesn't work
    void invokeAction(const QStringList &actionId, qlonglong timestamp);
    void yourShortcutGotChanged(const QStringList &actionId, const QList<int> &newKeys);

private Q_SLOTS:
    void writeSettings();

private:
    void loadSettings();
    void scheduleWriteSettings();
    QList<int> keysFromString(const QString &str);
    QString stringFromKeys(const QList<int> &keys);

    KdedGlobalAccelPrivate *const d;
};

#endif //KDEDGLOBALACCEL_H
