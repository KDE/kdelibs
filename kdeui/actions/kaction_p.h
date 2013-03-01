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

#include "kaction.h"
#include "kglobalaccel.h"
#include "kgesturemap.h"
#include <kshortcut.h>
#include <QCoreApplication>

class KAction;

namespace KAuth {
    class ObjectDecorator;
}

class KActionPrivate
{
    public:
        KActionPrivate()
            : componentName(QCoreApplication::applicationName()), q(0), decorator(0)
        {
        }

        void slotTriggered();
        void _k_emitActionGlobalShortcutChanged(QAction *action, const QKeySequence &seq);
        void authStatusChanged(KAuth::Action::AuthStatus status);

        void init(KAction *q_ptr);

        QString componentName;
        QString componentDisplayName;

        bool neverSetGlobalShortcut : 1;
        KAction *q;

        KAuth::ObjectDecorator *decorator;
};

#endif
