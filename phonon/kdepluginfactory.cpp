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

#include "kdepluginfactory.h"
#include "kiomediastream.h"
#include <QtPlugin>
#include <kicon.h>
#include <knotification.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <QCoreApplication>
#include <kaboutdata.h>

namespace Phonon
{

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, mainComponentData, (QCoreApplication::applicationName().toUtf8()))
K_GLOBAL_STATIC_WITH_ARGS(KComponentData, phononComponentData, ("phonon"))

static const KComponentData &componentData()
{
    if (!KGlobal::hasMainComponent()) {
        // a pure Qt application does not have a KComponentData object,
        // we'll give it one.
        *mainComponentData;
        qAddPostRoutine(mainComponentData.destroy);
    }
    Q_ASSERT(KGlobal::hasMainComponent());
    return *phononComponentData;
}

AbstractMediaStream *KdePluginFactory::createKioMediaStream(const QUrl &url, QObject *parent)
{
    return new KioMediaStream(url, parent);
}

QIcon KdePluginFactory::icon(const QString &name)
{
    return KIcon(name);
}

void KdePluginFactory::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver,
        const char *actionSlot)
{
    KNotification *notification = new KNotification(notificationName);
    notification->setComponentData(componentData());
    notification->setText(text);
    //notification->setPixmap(...);
    notification->addContext(QLatin1String("Application"), KGlobal::mainComponent().componentName());
    if (!actions.isEmpty() && receiver && actionSlot) {
        notification->setActions(actions);
        QObject::connect(notification, SIGNAL(activated(unsigned int)), receiver, actionSlot);
    }
    notification->sendEvent();
}

QString KdePluginFactory::applicationName() const
{
    const KAboutData *ad = KGlobal::mainComponent().aboutData();
    if (ad) {
        return ad->programName();
    }
    return KGlobal::mainComponent().componentName();
}

} // namespace Phonon

Q_EXPORT_PLUGIN2(phonon_platform_kde, Phonon::KdePluginFactory)

#include "kdepluginfactory.moc"
// vim: sw=4 sts=4 et tw=100
