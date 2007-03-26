/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef UI_GUIIMPL_H
#define UI_GUIIMPL_H

#include <kicon.h>
#include "../guiinterface.h"

namespace Phonon
{
class GuiImpl : public GuiInterface
{
    public:
        QVariant icon(const QString &name);
        void notification(const char *notificationName, const QString &text,
                const QStringList &actions = QStringList(), QObject *receiver = 0,
                const char *actionSlot = 0);

        virtual KioFallback *newKioFallback(MediaObject *parent);
};
} // namespace Phonon

#endif // UI_GUIIMPL_H
