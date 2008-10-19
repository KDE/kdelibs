#ifndef COMPONENT_H
#define COMPONENT_H
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

#include "globalshortcut.h"
#include "kglobalshortcutinfo.h"

#include "kconfiggroup.h"

#include <QtCore/QObject>
#include <QtCore/QHash>

class GlobalShortcut;
class GlobalShortcutContext;
class GlobalShortcutsRegistry;


Q_DECLARE_METATYPE(QStringList)

namespace KdeDGlobalAccel {

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class Component : public QObject
    {
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.kdedglobalaccel.Component")

    Q_SCRIPTABLE Q_PROPERTY( QString friendlyName READ friendlyName )
    Q_SCRIPTABLE Q_PROPERTY( QString uniqueName READ uniqueName )

public:

    //! Creates a new component. The component will be registered with @p
    //! registry if specified and registered with dbus.
    Component( 
            const QString &uniqueName,
            const QString &friendlyName,
            GlobalShortcutsRegistry *registry = NULL);

    ~Component();

    bool activateGlobalShortcutContext(const QString &uniqueName);

    void activateShortcuts();

    //! Returns all shortcuts in context @context
    QList<GlobalShortcut *> allShortcuts(const QString &context = "default") const;

    //! Creates the new global shortcut context @p context
    bool createGlobalShortcutContext(const QString &context, const QString &friendlyName);

    //! Return the current context
    GlobalShortcutContext* currentContext();

    //! Deactivate all currently active shortcuts
    void deactivateShortcuts();

    //! Returns the friendly name
    QString friendlyName() const;

    //! Returns the currently active shortcut for key
    GlobalShortcut *getShortcutByKey(int key) const;

    /**
     * Returns the list of shortcuts (different context) registered with @p
     * key.
     */
    QList<GlobalShortcut *> getShortcutsByKey(int key) const;

    //! Returns the shortcut by unique name. Only the active context is
    //! searched.
    GlobalShortcut *getShortcutByName(const QString &uniqueName) const;

    /**
     * Check if @a key is available for component @p component
     */
    bool isShortcutAvailable(int key, const QString &component) const;

    //! Load the settings from config group @p config
    void loadSettings(KConfigGroup &config);

    //! Sets the human readable name for this component.
    void setFriendlyName(const QString &);

    QString uniqueName() const;

    void writeSettings(KConfigGroup &config) const;

public Q_SLOTS:
    // For dbus Q_SCRIPTABLE has to be on slots. Scriptable methods are not
    // exported.

    Q_SCRIPTABLE QStringList shortcutNames(const QString &context = "default") const;

    //! Returns all shortcut in context @context
    Q_SCRIPTABLE QList<KGlobalShortcutInfo> allShortcutInfos(const QString &context = "default") const;

#if 0
    //! Returns all shortcut in context @context
    Q_SCRIPTABLE KGlobalShortcutInfo shortcutInfo(const QString &name, const QString &context = "default") const;
#endif

    //! Returns the shortcut contexts available for the component.
    Q_SCRIPTABLE QStringList getShortcutContexts() const;

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


#endif /* #ifndef COMPONENT_H */
