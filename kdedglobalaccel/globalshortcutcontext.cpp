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

#include "globalshortcutcontext.h"

#include "globalshortcut.h"


GlobalShortcutContext::GlobalShortcutContext(
        const QString &uniqueName,
        const QString &friendlyName,
        KdeDGlobalAccel::Component *component)

        :   _uniqueName(uniqueName),
            _friendlyName(friendlyName),
            _component(component),
            _actions()
    {}


GlobalShortcutContext::~GlobalShortcutContext()
    {
    }


void GlobalShortcutContext::addShortcut(GlobalShortcut *shortcut)
    {
    _actions.insert(shortcut->uniqueName(), shortcut);
    }


KdeDGlobalAccel::Component const *GlobalShortcutContext::component() const
    {
    return _component;
    }


KdeDGlobalAccel::Component *GlobalShortcutContext::component()
    {
    return _component;
    }


QString GlobalShortcutContext::friendlyName() const
    {
    return _friendlyName;
    }


GlobalShortcut *GlobalShortcutContext::takeShortcut(GlobalShortcut *shortcut)
    {
    // Try to take the shortcut. Result could be null if the shortcut doesn't
    // belong ti this component.
    return _actions.take(shortcut->uniqueName());
    }


QString GlobalShortcutContext::uniqueName() const
    {
    return _uniqueName;
    }
