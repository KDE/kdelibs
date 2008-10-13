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
#include <kdebug.h>


Component::Component( const QString &uniqueName, const QString &friendlyName)
        :   _uniqueName(uniqueName)
            ,_friendlyName(friendlyName)
            ,_actions()
    {}


Component::~Component()
    {
    qDeleteAll(_actions);
    Q_ASSERT(_actions.isEmpty());
    }


void Component::addShortcut(GlobalShortcut *shortcut)
    {
    kDebug(125) << shortcut->uniqueName();
    _actions.insert(shortcut->uniqueName(), shortcut);
    }


QList<GlobalShortcut*> Component::allShortcuts() const
    {
    return _actions.values();
    }


QString Component::friendlyName() const
    {
    return _friendlyName;
    }


GlobalShortcut *Component::getShortcutByKey(int key)
    {
    Q_FOREACH(GlobalShortcut *sc, _actions)
        {
        if (sc->keys().contains(key)) return sc;
        }
    return NULL;
    }


GlobalShortcut *Component::getShortcutByName(const QString &uniqueName)
    {
    return _actions.value(uniqueName);
    }


void Component::setFriendlyName(const QString &name)
    {
    _friendlyName = name;
    }


QString Component::uniqueName() const
    {
    return _uniqueName;
    }


void Component::setInactive()
    {
    Q_FOREACH (GlobalShortcut *shortcut, _actions)
        {
        shortcut->setInactive();
        }
    }


GlobalShortcut *Component::takeAction(GlobalShortcut *shortcut)
    {
    if (shortcut->component() == this)
        {
        return _actions.take(shortcut->uniqueName());
        }
    return NULL;
    }

