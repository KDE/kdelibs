/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef KGLOBALACCEL_P_H
#define KGLOBALACCEL_P_H

#include <QtCore/QHash>
#include <QtCore/QStringList>

#include "kdedglobalaccel_interface.h"

class KAction;
class KShortcut;

class KGlobalAccelPrivate
{
public:
    KGlobalAccelPrivate();

    ///Propagate any shortcut changes to the KDED module that does the bookkeeping
    ///and the key grabbing.
    ///If this is called with an action that has an empty active global shortcut and
    ///an empty default shortcut, the record of that action will be deleted.
    void updateGlobalShortcut(KAction *action, /*KAction::ShortcutTypes*/uint flags);

    ///Register or unregister the action in this class, and notify the KDED module
    void updateGlobalShortcutAllowed(KAction *action, /*KAction::ShortcutTypes*/uint flags);

    QList<int> intListFromShortcut(const KShortcut &cut);
    KShortcut shortcutFromIntList(const QList<int> &list);

    void _k_invokeAction(const QStringList&);
    void _k_shortcutGotChanged(const QStringList&, const QList<int>&);

    //for all actions with (isEnabled() && globalShortcutAllowed())
    QHash<QString, KAction *> nameToAction;
    QHash<KAction *, QString> actionToName;

    QString mainComponentName;
    bool isUsingForeignComponentName;
    bool enabled;

    org::kde::KdedGlobalAccelInterface iface;
};

#endif
