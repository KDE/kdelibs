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
        ,_config("kglobalshortcutsrc", KConfig::SimpleConfig)
    {}


GlobalShortcutsRegistry::~GlobalShortcutsRegistry()
    {}


KdeDGlobalAccel::Component *GlobalShortcutsRegistry::addComponent(KdeDGlobalAccel::Component *component)
    {
    return *(_components.insert(component->uniqueName(), component));
    }


QList<KdeDGlobalAccel::Component*> GlobalShortcutsRegistry::allMainComponents() const
    {
    return _components.values();
    }


GlobalShortcut *GlobalShortcutsRegistry::getActiveShortcutByKey(int key) const
    {
    return _active_keys.value(key);
    }


KdeDGlobalAccel::Component *GlobalShortcutsRegistry::getComponent(const QString &uniqueName)
    {
    return _components.value(uniqueName);
    }


GlobalShortcut *GlobalShortcutsRegistry::getShortcutByKey(int key) const
    {
    Q_FOREACH (KdeDGlobalAccel::Component *component, _components)
        {
        GlobalShortcut *rc = component->getShortcutByKey(key);
        if (rc) return rc;
        }
    return NULL;
    }


GlobalShortcutsRegistry * GlobalShortcutsRegistry::self()
    {
    K_GLOBAL_STATIC( GlobalShortcutsRegistry, self );
    return self;
    }


void GlobalShortcutsRegistry::loadSettings()
    {
    foreach (const QString &groupName, _config.groupList())
        {
        kDebug() << "Loading group " << groupName;

        // Skip the subgroups [Friendly Name] and contexts
        if (groupName.indexOf('\x1d')!=-1)
            {
            continue;
            }

        // loadSettings isn't designed to be called in between. Only at the
        // beginning.
        Q_ASSERT(!getComponent(groupName));

        KConfigGroup configGroup(&_config, groupName);
        KConfigGroup friendlyGroup(&configGroup, "Friendly Name");

        KdeDGlobalAccel::Component *component = new KdeDGlobalAccel::Component(
                groupName,
                friendlyGroup.readEntry("Friendly Name"),
                this);

        // Now load the contexts
        Q_FOREACH(QString context, configGroup.groupList())
            {
            // Skip the friendly name group
            if (context=="Friendly Name") continue;

            KConfigGroup contextGroup(&configGroup, context);
            component->createGlobalShortcutContext(context);
            component->activateGlobalShortcutContext(context);
            component->loadSettings(contextGroup);
            }

        // Load the default context
        component->activateGlobalShortcutContext("default");
        component->loadSettings(configGroup);
        }
    }


bool GlobalShortcutsRegistry::registerKey(int key, GlobalShortcut *shortcut)
    {
    Q_ASSERT(_manager);
    if (key == 0)
        {
        kDebug() << shortcut->uniqueName() << ": Key '" << QKeySequence(key).toString()
                 << "' already taken by " << _active_keys.value(key)->uniqueName() << ".";
        return false;
        }
    else if (_active_keys.value(key))
        {
        kDebug() << shortcut->uniqueName() << ": Attempt to register key 0.";
        return false;
        }

    kDebug() << "Registering key" << QKeySequence(key).toString() << "for"
             << shortcut->component()->uniqueName() << ":" << shortcut->uniqueName();

    _active_keys.insert(key, shortcut);
    return _manager->grabKey(key, true);
    }


void GlobalShortcutsRegistry::setAccelManager(KGlobalAccelImpl *manager)
    {
    Q_ASSERT(_manager==NULL);
    _manager = manager;
    }


void GlobalShortcutsRegistry::setInactive()
    {
    kDebug();

    // Unregister all currently registered actions. Enables the module to be
    // loaded / unloaded by kded.
    Q_FOREACH (KdeDGlobalAccel::Component *component, _components)
        {
        component->activateShortcuts();
        }
    }


KdeDGlobalAccel::Component *GlobalShortcutsRegistry::takeComponent(KdeDGlobalAccel::Component *component)
    {
    return _components.take(component->uniqueName());
    }


bool GlobalShortcutsRegistry::unregisterKey(int key, GlobalShortcut *shortcut)
    {
    Q_ASSERT(_manager);
    if (_active_keys.value(key)!=shortcut)
        {
        // The shortcut doesn't own the key or the key isn't grabbed
        return false;
        }

    kDebug() << "Unregistering key" << QKeySequence(key).toString() << "for"
             << shortcut->component()->uniqueName() << ":" << shortcut->uniqueName();

    _manager->grabKey(key, false);
    _active_keys.take(key);
    return true;
    }


void GlobalShortcutsRegistry::writeSettings() const
    {
    Q_FOREACH(
            const KdeDGlobalAccel::Component *component,
            GlobalShortcutsRegistry::self()->allMainComponents())
        {
        KConfigGroup configGroup(&_config, component->uniqueName());
        component->writeSettings(configGroup);
        }

    _config.sync();
    }


