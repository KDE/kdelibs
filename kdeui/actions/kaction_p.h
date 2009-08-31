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
#include <kcomponentdata.h>

class KAction;

namespace KAuth {
    class Action;
}

class KActionPrivate
{
    public:
        KActionPrivate()
            : componentData(KGlobal::mainComponent()), globalShortcutEnabled(false), q(0), authAction(0)
        {
        }

        void slotTriggered();
        void authStatusChanged(int status);

        void init(KAction *q_ptr);
        void setActiveGlobalShortcutNoEnable(const KShortcut &cut);

        void maybeSetComponentData(const KComponentData &kcd)
        {
            if (neverSetGlobalShortcut) {
                componentData = kcd;
            }
        }

        KComponentData componentData;   //this is **way** more lightweight than it looks
        KShortcut globalShortcut, defaultGlobalShortcut;
        KShapeGesture shapeGesture, defaultShapeGesture;
        KRockerGesture rockerGesture, defaultRockerGesture;

        bool globalShortcutEnabled : 1;
        bool neverSetGlobalShortcut : 1;
        KAction *q;

        KAuth::Action *authAction;
        // TODO: Remove whenever QIcon overlays will get fixed
        KIcon oldIcon;
};

#endif
