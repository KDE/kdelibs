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

#include "kconfiggroup.h"

#include <QtCore/QObject>
#include <QtCore/QHash>

class GlobalShortcut;
class GlobalShortcutContext;
class GlobalShortcutsRegistry;

namespace KdeDGlobalAccel {

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class Component : public QObject
    {
    Q_OBJECT

    Q_PROPERTY( QString friendlyName READ friendlyName )
    Q_PROPERTY( QString uniqueName READ uniqueName )

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

    //! Returns the currently active shortcut for key
    GlobalShortcut *getShortcutByKey(int key);

    GlobalShortcut *getShortcutByName(const QString &uniqueName);

    //! Check if @a key is available for component @p component
    bool isKeyAvailable(int key, const QString &component) const;

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


#endif /* #ifndef COMPONENT_H */
