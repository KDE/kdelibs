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
#ifndef KNEWSTUFFACTION_H
#define KNEWSTUFFACTION_H

#include <knewstuff2/knewstuff_export.h>

class QObject;
class KAction;
class KActionCollection;

namespace KNS
{
KNEWSTUFF_EXPORT_DEPRECATED KAction *standardAction(const QString &what,
        const QObject *receiver,
        const char *slot,
        KActionCollection *parent,
        const char *name = 0);

/**
 * @brief Standard action for all GHNS workflows.
 *
 * This action can be used to add KNewStuff support to menus and toolbars.
 *
 * @param what text describing what is being downloaded. will be
 *        shown as "Get New <what>..."
 * @param receiver the QObject to connect the triggered(bool) signal to.
 * @param slot the slot to connect the triggered(bool) signal to.
 * @param parent the action's parent collection.
 * @param name The name by which the action will be retrieved again from the collection.
 */
KNEWSTUFF_EXPORT KAction *standardAction(const QString &what,
        const QObject *receiver,
        const char *slot,
        KActionCollection *parent,
        const QString &name = QString());
}

#endif // KNEWSTUFFACTION_H
