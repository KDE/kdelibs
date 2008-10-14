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

#include <QtGui/QKeySequence>



GlobalShortcut::GlobalShortcut(const QString &uniqueName, const QString &friendlyName, Component *component)
        :   _isPresent(false)
            ,_isFresh(true)
            ,_component(component)
            ,_uniqueName(uniqueName)
            ,_friendlyName(friendlyName)
            ,_keys()
            ,_defaultKeys()
    {
    component->addShortcut(this);
    }


GlobalShortcut::~GlobalShortcut()
    {
    setInactive();
    component()->takeAction(this);
    }


bool GlobalShortcut::isFresh() const
    {
    return _isFresh;
    }


void GlobalShortcut::setIsFresh(bool value)
    {
    _isFresh = value;
    }


Component *GlobalShortcut::component()
    {
    return _component;
    }


QString GlobalShortcut::uniqueName() const
    {
    return _uniqueName;
    }


QString GlobalShortcut::friendlyName() const
    {
    return _friendlyName;
    }


void GlobalShortcut::setFriendlyName(const QString &name)
    {
    _friendlyName = name;
    }


QList<int> GlobalShortcut::keys() const
    {
    return _keys;
    }


void GlobalShortcut::setKeys(const QList<int> newKeys)
    {
    bool active = _isPresent;
    if (active)
        {
        setInactive();
        }

    _keys = QList<int>();

    Q_FOREACH(int key, newKeys)
        {
        if (key!=0 && !GlobalShortcutsRegistry::self()->getShortcutByKey(key))
            {
            _keys.append(key);
            }
        else
            {
            kDebug() << _uniqueName << "skipping because key is already taken";
            }
        }

    if (active)
        {
        setActive();
        }
    }


QList<int> GlobalShortcut::defaultKeys() const
    {
    return _defaultKeys;
    }


void GlobalShortcut::setDefaultKeys(const QList<int> newKeys)
    {
    _defaultKeys = newKeys;
    }



void GlobalShortcut::setActive()
    {
    if (_isPresent)
        {
        return;
        }

    _isPresent = true;

    Q_FOREACH( int key, _keys)
        {
        if (!GlobalShortcutsRegistry::self()->registerKey(key, this))
            {
            kDebug() << uniqueName() << ": Failed to register " << QKeySequence(key).toString();
            }
        }
    }


void GlobalShortcut::setInactive()
    {
    if (!_isPresent)
        {
        return;
        }

    _isPresent = false;

    Q_FOREACH( int key, _keys)
        {
        if (!GlobalShortcutsRegistry::self()->unregisterKey(key, this))
            {
            kDebug() << uniqueName() << ": Failed to unregister " << QKeySequence(key).toString();
            }
        }
    }

