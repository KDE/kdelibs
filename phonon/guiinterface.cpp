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

#include "guiinterface.h"
#include "kiofallback.h"

namespace Phonon
{

GuiInterface *_kde_internal_GuiInterface;
GuiInterface *GuiInterface::s_instance = 0;

GuiInterface *GuiInterface::instance()
{
    if (_kde_internal_GuiInterface) {
        return _kde_internal_GuiInterface;
    }
    if (!s_instance) {
        s_instance = new GuiInterface();
    }
    return s_instance;
}

GuiInterface::~GuiInterface()
{
    if (s_instance == this) {
        s_instance = 0;
    }
}

void GuiInterface::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver, const char *actionSlot)
{
    Q_UNUSED(notificationName);
    Q_UNUSED(text);
    Q_UNUSED(actions);
    Q_UNUSED(receiver);
    Q_UNUSED(actionSlot);
}

KioFallback *GuiInterface::newKioFallback(MediaObject *parent)
{
    Q_UNUSED(parent);
    return 0;
}

} // namespace Phonon
// vim: sw=4 sts=4 et tw=100
