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

#include "globalshortcutsregistry.h"
#include "component.h"
#include "globalshortcut.h"
#include "globalshortcutcontext.h"

#include "kdebug.h"
#include "kglobal.h"

#include <QtGui/QKeySequence>
#include <QDBusConnection>

#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#include <QtGui/QX11Info>
#include <QtGui/QApplication>
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#else
#include "kglobalaccel_qws.h"
#endif

GlobalShortcutsRegistry::GlobalShortcutsRegistry()
    :   QObject()
        ,_active_keys()
        ,_components()
        ,_manager(new KGlobalAccelImpl(this))
        ,_config("kglobalshortcutsrc", KConfig::SimpleConfig)
    {
    _manager->setEnabled(true);
    }


GlobalShortcutsRegistry::~GlobalShortcutsRegistry()
    {
    _manager->setEnabled(false);
    }


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


bool GlobalShortcutsRegistry::keyPressed(int keyQt)
    {
    // Check if keyQt is one of out global shortcuts. Because other kded
    // modules could receive key events too that is not guaranteed.
    GlobalShortcut *shortcut = getShortcutByKey(keyQt);
    if (!shortcut || !shortcut->isActive())
        {
        // Not one of ours. Or one of ours but not active. It's meant for some
        // other kded module most likely.
        return false;
        }

    // Never print out the received key if it is not one of our active global
    // shortcuts. We could end up printing out kpasswdservers password.
    kDebug() << QKeySequence(keyQt).toString() << "=" << shortcut->uniqueName();

    QStringList data(shortcut->component()->uniqueName());
    data.append(shortcut->uniqueName());
    data.append(shortcut->component()->friendlyName());
    data.append(shortcut->friendlyName());
#ifdef Q_WS_X11
    // pass X11 timestamp
    long timestamp = QX11Info::appTime();
    // Make sure kded has ungrabbed the keyboard after receiving the keypress,
    // otherwise actions in application that try to grab the keyboard (e.g. in kwin)
    // may fail to do so. There is still a small race condition with this being out-of-process.
    qApp->syncX();
#else
    long timestamp = 0;
#endif
    emit invokeAction(data, timestamp);
    return true;
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

        // We previously stored the friendly name in a separate group. migrate
        // that
        QString friendlyName;
        KConfigGroup friendlyGroup(&configGroup, "Friendly Name");
        if (friendlyGroup.isValid())
            {
            friendlyName = friendlyGroup.readEntry("Friendly Name");
            friendlyGroup.deleteGroup();
            }
        else
            {
            friendlyName = configGroup.readEntry("_k_friendly_name");
            }

        // Create the component
        KdeDGlobalAccel::Component *component = new KdeDGlobalAccel::Component(
                groupName,
                friendlyName,
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


#include "moc_globalshortcutsregistry.cpp"
