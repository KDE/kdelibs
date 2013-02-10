/* This file is part of the KDE libraries
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KACTION_P_H
#define KACTION_P_H

#include "kglobalaccel.h"
#include "kgesturemap.h"
#include <kshortcut.h>

class KAction;

namespace KAuth {
    class ObjectDecorator;
}

class KActionPrivate
{
    public:
        KActionPrivate()
            : componentName(), globalShortcutEnabled(false), q(0), decorator(0)
        {
        }

        void slotTriggered();
        void authStatusChanged(KAuth::Action::AuthStatus status);

        void init(KAction *q_ptr);
        void setActiveGlobalShortcutNoEnable(const KShortcut &cut);

        // called by KActionCollection
        void maybeSetComponentName(const QString &cname, const QString &dispName)
        {
            if (neverSetGlobalShortcut) {
                componentName = cname;
                componentDisplayName = dispName;
            }
        }

        QString componentName;
        QString componentDisplayName;
        KShortcut globalShortcut, defaultGlobalShortcut;

        bool globalShortcutEnabled : 1;
        bool neverSetGlobalShortcut : 1;
        KAction *q;

        KAuth::ObjectDecorator *decorator;
};

#endif
