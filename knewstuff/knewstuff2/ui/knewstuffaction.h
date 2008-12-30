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

/**
 * @brief Standard action for all GHNS workflows.
 *
 * This action can be used to add KNewStuff support to menus and toolbars.
 */
KNEWSTUFF_EXPORT KAction* standardAction(const QString& what,
        const QObject *recvr,
        const char *slot,
        KActionCollection* parent,
        const char *name = 0);
}

#endif // KNEWSTUFFACTION_H
