#ifndef GLOBAL_SHORTCUTS_REGISTRY_H
#define GLOBAL_SHORTCUTS_REGISTRY_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "ksharedconfig.h"

#include <QtCore/QObject>
#include <QtCore/QHash>


class GlobalShortcut;
class KGlobalAccelImpl;

namespace KdeDGlobalAccel
    {
    class Component;
    }


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class GlobalShortcutsRegistry : public QObject
    {
    Q_OBJECT

public:

    QList<KdeDGlobalAccel::Component *> allMainComponents() const;

    /**
     * Get the shortcut corresponding to key. Only active shortcut are
     * considered.
     */
    GlobalShortcut *getActiveShortcutByKey(int key) const;

    KdeDGlobalAccel::Component *getComponent(const QString &uniqueName);

    /**
     * Get the shortcut corresponding to key. All shortcuts are
     * considered.
     */
    GlobalShortcut *getShortcutByKey(int key) const;

    static GlobalShortcutsRegistry *self();

    void setAccelManager(KGlobalAccelImpl *manager);

    bool registerKey(int key, GlobalShortcut *shortcut);

    void setInactive();

    bool unregisterKey(int key, GlobalShortcut *shortcut);

Q_SIGNALS:

    void invokeAction(const QStringList &actionId, qlonglong timestamp);

public Q_SLOTS:

    void loadSettings();

    void writeSettings() const;

private:

    friend class KdeDGlobalAccel::Component;
    friend class KGlobalAccelImpl;

    KdeDGlobalAccel::Component *addComponent(KdeDGlobalAccel::Component *component);
    KdeDGlobalAccel::Component *takeComponent(KdeDGlobalAccel::Component *component);

    //called by the implementation to inform us about key presses
    //returns true if the key was handled
    bool keyPressed(int keyQt);

    GlobalShortcutsRegistry();

    ~GlobalShortcutsRegistry();

    QHash<int, GlobalShortcut*> _active_keys;
    QHash<QString, KdeDGlobalAccel::Component *> _components;

    KGlobalAccelImpl *_manager;

    mutable KConfig _config;
    };


#endif /* #ifndef GLOBAL_SHORTCUTS_REGISTRY_H */
