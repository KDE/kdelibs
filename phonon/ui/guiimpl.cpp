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

#include "guiimpl.h"

#include <QString>
#include <QObject>

#include <knotification.h>
#include <kcomponentdata.h>

#include "../factory.h"
#include "kiofallbackimpl.h"

namespace Phonon
{

QVariant GuiImpl::icon(const QString &name)
{
    return KIcon(name);
}

void GuiImpl::notification(const char *notificationName, const QString &text, const QStringList &actions, QObject *receiver, const char *actionSlot)
{
    KNotification *notification = new KNotification(notificationName);
    notification->setComponentData(Factory::componentData());
    notification->setText(text);
    //notification->setPixmap(...);
    notification->addContext(QLatin1String("Application"), KGlobal::mainComponent().componentName());
    if (!actions.isEmpty() && receiver && actionSlot) {
        notification->setActions(actions);
        QObject::connect(notification, SIGNAL(activated(unsigned int)), receiver, actionSlot);
    }
    notification->sendEvent();
}

KioFallback *GuiImpl::newKioFallback(MediaObject *parent)
{
    return new KioFallbackImpl(parent);
}

static int initGuiImpl()
{
    _kde_internal_GuiInterface = new GuiImpl;
    return 0; // just something
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(initGuiImpl)
#else
static int _dummyGuiImpl = initGuiImpl();
#endif

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
