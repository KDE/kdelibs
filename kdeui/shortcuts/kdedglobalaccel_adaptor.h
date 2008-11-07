/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef KDEDGLOBALACCEL_ADAPTOR_H
#define KDEDGLOBALACCEL_ADAPTOR_H

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusArgument>

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QStringList>)

class KdedGlobalAccelAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KdedGlobalAccel")

public:
    KdedGlobalAccelAdaptor(KdedGlobalAccel *parent)
     : QDBusAbstractAdaptor(parent)
    {
        Q_ASSERT(parent);
        qDBusRegisterMetaType<QList<int> >();
        qDBusRegisterMetaType<QList<QStringList> >();
        connect(parent, SIGNAL(invokeAction(const QStringList &, qlonglong)),
                SIGNAL(invokeAction(const QStringList &, qlonglong)));
        connect(parent, SIGNAL(yourShortcutGotChanged(const QStringList &, const QList<int> &)),
                SIGNAL(yourShortcutGotChanged(const QStringList &, const QList<int> &)));
    }

private:
    inline KdedGlobalAccel *p()
        { return static_cast<KdedGlobalAccel *>(parent()); }
public Q_SLOTS:
    inline QList<QStringList> allMainComponents()
        { return p()->allComponents(); }
    inline QList<QStringList> allActionsForComponent(const QStringList &actionId)
        { return p()->allActionsForComponent(actionId); }
    //get all registered keys (mainly for debugging)
    inline QList<int> allKeys()
        { return p()->allKeys(); }
    //get all registered keys as strings (mainly for debugging)
    inline QStringList allKeysAsString()
        { return p()->allKeysAsString(); }
    //get the action that has key registered for it
    inline QStringList action(int key)
        { return p()->actionId(key); }
    //get the keys registered to  action
    inline QList<int> shortcut(const QStringList &actionId)
        { return p()->shortcut(actionId); }
    //get the default keys registered to action
    inline QList<int> defaultShortcut(const QStringList &actionId)
        { return p()->defaultShortcut(actionId); }
    //to be called by main components owning the action
    inline QList<int> setShortcut(const QStringList &actionId, const QList<int> &keys, uint flags)
        { return p()->setShortcut(actionId, keys, flags); }
    //this is used if application A wants to change shortcuts of application B
    inline void setForeignShortcut(const QStringList &actionId, const QList<int> &keys)
        { return p()->setForeignShortcut(actionId, keys); }
    //to be called when a KAction is destroyed or setGlobalShortcutAllowed(false) is called.
    //The shortcut stays in our data structures for conflict resolution but won't trigger
    //until woken up again by calling setShortcut().
    inline void setInactive(const QStringList &actionId)
        { return p()->setInactive(actionId); }
    inline void doRegister(const QStringList &actionId)
        { return p()->doRegister(actionId); }
    inline void unRegister(const QStringList &actionId)
        { return p()->unRegister(actionId); }

Q_SIGNALS:
    void invokeAction(const QStringList &actionId, qlonglong timestamp);
    void yourShortcutGotChanged(const QStringList &action, const QList<int> &newShortcut);
};

#endif //KDEDGLOBALACCEL_ADAPTOR_H
