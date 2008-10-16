#ifndef KDEDGLOBALACCEL_P_H
#define KDEDGLOBALACCEL_P_H

/*
    This file is part of the KDE libraries

    Copyright (c) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "kconfiggroup.h"
#include "ksharedconfig.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>

#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#else
#include "kglobalaccel_qws.h"
#endif

namespace KdeDGlobalAccel {
class Component;
}

class GlobalShortcutContext;

/**
 * Represents a global shortcut.
 *
 * @internal
 *
 * \note This class can handle multiple keys (default and active). This
 * feature isn't used currently. kde4 only allows setting one key per global
 * shortcut.
 */
class GlobalShortcut
    {
public:

    GlobalShortcut(const QString &uniqueName, const QString &friendlyName, KdeDGlobalAccel::Component *component);

    ~GlobalShortcut();

    //! Returns the component the shortcuts belongs to
    KdeDGlobalAccel::Component *component();

    //! Returns the default keys for this shortcut.
    QList<int> defaultKeys() const;

    //! Return the friendly display name for this shortcut.
    QString friendlyName() const;

    //! Check if the shortcut is active. It's keys are grabbed
    bool isActive() const { return _isPresent;  }

    //! Check if the shortcut is fresh/new. Is an internal state
    bool isFresh() const;

    //! Returns a list of keys associated with this shortcut.
    QList<int> keys() const;

    //! Activates the shortcut. The keys are grabbed.
    void setActive();

    //! Sets the default keys for this shortcut.
    void setDefaultKeys(QList<int>);

    //! Sets the friendly name for the shortcut. For display.
    void setFriendlyName(const QString &);

    //! Sets the shortcut inactive. No longer grabs the keys.
    void setInactive();

    void setIsFresh(bool);

    //! Sets the keys activated with this shortcut. The old keys are freed.
    void setKeys(QList<int>);

    //! Returns the unique name aka id for the shortcuts.
    QString uniqueName() const;

private:

    //! means the associated application is active.
    bool _isPresent:1;

    //! means the shortcut is new
    bool _isFresh:1;

    //! The Component the shortcut belongs to.
    KdeDGlobalAccel::Component *_component;

    QString _uniqueName;
    QString _friendlyName; //usually localized

    QList<int> _keys;
    QList<int> _defaultKeys;
    };

class GlobalShortcutsRegistry;

namespace KdeDGlobalAccel {

class Component
    {
public:

    Component( 
            const QString &uniqueName,
            const QString &friendlyName,
            GlobalShortcutsRegistry *registry = NULL);

    ~Component();

    bool activateGlobalShortcutContext(const QString &context);

    void activateShortcuts();

    void addShortcut(GlobalShortcut *shortcut);

    QList<GlobalShortcut *> allShortcuts() const;

    bool createGlobalShortcutContext(const QString &context);

    void deactivateShortcuts();

    QString friendlyName() const;

    GlobalShortcut *getShortcutByKey(int key);

    GlobalShortcut *getShortcutByName(const QString &uniqueName);

    void loadSettings(KConfigGroup &config);

    void setUniqueName(const QString &);

    void setFriendlyName(const QString &);

    GlobalShortcut *takeShortcut(GlobalShortcut *shortcut);

    QString uniqueName() const;

    void writeSettings(KConfigGroup &config) const;

private:

    QString _uniqueName;
    //the name as it would be found in a magazine article about the application,
    //possibly localized if a localized name exists.
    QString _friendlyName;

    GlobalShortcutsRegistry *_registry;

    GlobalShortcutContext *_current;
    QHash<QString, GlobalShortcutContext *> _contexts;
    };

}


class KGlobalAccelImpl;


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

    void loadSettings();

    void setAccelManager(KGlobalAccelImpl *manager);

    bool registerKey(int key, GlobalShortcut *shortcut);


    void setInactive();

    bool unregisterKey(int key, GlobalShortcut *shortcut);

    void writeSettings() const;

private:

    friend class KdeDGlobalAccel::Component;

    KdeDGlobalAccel::Component *addComponent(KdeDGlobalAccel::Component *component);
    KdeDGlobalAccel::Component *takeComponent(KdeDGlobalAccel::Component *component);


    GlobalShortcutsRegistry();

    ~GlobalShortcutsRegistry();

    QHash<int, GlobalShortcut*> _active_keys;
    QHash<QString, KdeDGlobalAccel::Component *> _components;

    KGlobalAccelImpl *_manager;

    mutable KConfig _config;
    };

#endif // KDEDGLOBALACCEL_P_H
