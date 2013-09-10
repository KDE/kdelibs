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

#include "knewstuffaction.h"

#include <QAction>
#include <klocalizedstring.h>
#include <kactioncollection.h>

using namespace KNS3;

QAction *KNS3::standardAction(const QString &what,
                             const QObject *receiver,
                             const char *slot, KActionCollection *parent,
                             const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QString(name), action);
    action->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}

QAction *KNS3::standardActionUpload(const QString &what,
                             const QObject *receiver,
                             const char *slot, KActionCollection *parent,
                             const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QString(name), action);
    // FIXME: Get a specific upload icon!
    action->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}
