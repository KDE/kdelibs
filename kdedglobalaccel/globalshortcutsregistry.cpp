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

#include "kdedglobalaccel_p.h"
#include "kdebug.h"
#include "kglobal.h"

#include <QtGui/QKeySequence>

GlobalShortcutsRegistry::GlobalShortcutsRegistry()
    :   _active_keys()
        ,_components()
        ,_manager(NULL)
    {}


GlobalShortcutsRegistry::~GlobalShortcutsRegistry()
    {}


Component *GlobalShortcutsRegistry::addComponent(Component *component)
    {
    return *(_components.insert(component->uniqueName(), component));
    }


QList<Component*> GlobalShortcutsRegistry::allMainComponents() const
    {
    return _components.values();
    }


GlobalShortcut *GlobalShortcutsRegistry::getActiveShortcutByKey(int key) const
    {
    return _active_keys.value(key);
    }


Component *GlobalShortcutsRegistry::getComponent(const QString &uniqueName)
    {
    return _components.value(uniqueName);
    }


GlobalShortcut *GlobalShortcutsRegistry::getShortcutByKey(int key) const
    {
    Q_FOREACH (Component *component, _components)
        {
        GlobalShortcut *rc = component->getShortcutByKey(key);
        if (rc) return rc;
        }
    return NULL;
    }


GlobalShortcutsRegistry * GlobalShortcutsRegistry::instance()
    {
    K_GLOBAL_STATIC( GlobalShortcutsRegistry, self );
    return self;
    }


void GlobalShortcutsRegistry::registerKey(int key, GlobalShortcut *shortcut)
    {
    kDebug(125) << shortcut->uniqueName() << QKeySequence(key).toString();
    Q_ASSERT(!_active_keys.value(key));
    _active_keys.insert(key, shortcut);
    _manager->grabKey(key, true);
    }


void GlobalShortcutsRegistry::setAccelManager(KGlobalAccelImpl *manager)
    {
    _manager = manager;
    }


void GlobalShortcutsRegistry::setInactive()
    {
    kDebug(125);

    // Unregister all currently registered actions. Enables the module to be
    // loaded / unloaded by kded.
    Q_FOREACH (Component *component, _components)
        {
        component->setInactive();
        }
    }


void GlobalShortcutsRegistry::unregisterKey(int key, GlobalShortcut *shortcut)
    {
    kDebug(125) << shortcut->uniqueName() << QKeySequence(key).toString();

    Q_ASSERT(_active_keys.value(key)==shortcut);

    _manager->grabKey(key, false);
    _active_keys.take(key);
    }


