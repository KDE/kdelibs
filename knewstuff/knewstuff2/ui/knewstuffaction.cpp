/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <kaction.h>
#include <kicon.h>
#include <klocale.h>
#include <kactioncollection.h>

#include "knewstuff2/ui/knewstuffaction.h"

using namespace KNS;

KAction* KNS::standardAction(const QString& what,
                             const QObject *recvr,
                             const char *slot, KActionCollection* parent,
                             const char *name)
{
    QString data = what;
    if (data.isEmpty()) {
        data = i18nc("Hot new stuff offers to Download New Data. Apps can fill in an individual string here, but Data is the default. This is in an action that gets displayed in the menu or toolbar for example.", "Download New Data...");
    }
    KAction *action = new KAction(data, parent);
    parent->addAction(name, action);
    action->setIcon(KIcon("get-hot-new-stuff"));
    QObject::connect(action, SIGNAL(triggered(bool)), recvr, slot);

    return action;
}

