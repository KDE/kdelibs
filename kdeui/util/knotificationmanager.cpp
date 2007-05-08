/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

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

#include "knotificationmanager_p.h"
#include "knotification.h"

#include <QHash>
#include <QWidget>
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

typedef QHash<QString,QString> Dict;

struct KNotificationManager::Private
{
    QHash<int , KNotification*> notifications;
    QDBusInterface *knotify;
};

KNotificationManager * KNotificationManager::self()
{
    K_GLOBAL_STATIC(KNotificationManager, s_self)
    return s_self;
}


KNotificationManager::KNotificationManager()
    : d(new Private)
{
    d->knotify =
        new QDBusInterface(QLatin1String("org.kde.knotify"), QLatin1String("/Notify"), QLatin1String("org.kde.KNotify"), QDBusConnection::sessionBus(), this);
    if (d->knotify->isValid())
    {
        QObject::connect(d->knotify, SIGNAL(notificationClosed(int)),
                         this, SLOT(notificationClosed(int)));
        QObject::connect(d->knotify, SIGNAL(notificationActivated(int,int)),
                         this, SLOT(notificationActivated(int,int)));
    }
}


KNotificationManager::~KNotificationManager()
{
    delete d->knotify;
    delete d;
}

void KNotificationManager::notificationActivated( int id, int action )
{
    kDebug(299) << k_funcinfo << id << " " << action << endl;
    if(d->notifications.contains(id))
    {
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->activate( action );
    }
}

void KNotificationManager::notificationClosed( int id )
{
    kDebug( 299 ) << k_funcinfo << id  << endl;
    if(d->notifications.contains(id))
    {
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->close();
    }
}


void KNotificationManager::close( int id )
{
    QDBusReply<void> reply = d->knotify->call("closeNotification", id);
    if (!reply.isValid())
    {
        kWarning(299) << "error while contacting knotify server" << endl;
    }
}

unsigned int KNotificationManager::notify( KNotification* n, const QPixmap &pix,
                                           const QStringList &actions,
                                           const KNotification::ContextList & contexts,
                                           const QString &appname)
{
    WId winId=n->widget() ? n->widget()->topLevelWidget()->winId()  : 0;

    QByteArray pixmapData;

    {
        QDataStream arg(&pixmapData, QIODevice::WriteOnly);
        arg << pix;
    }

    QVariantList contextList;
    typedef QPair<QString,QString> Context;
    foreach (const Context& ctx, contexts)
    {
        QVariantList vl;
        vl << ctx.first << ctx.second;
        contextList << vl;
    }

    QDBusReply<int> reply =
        d->knotify->call(QDBus::Block, "event", n->eventId(),
                         (appname.isEmpty() ? KGlobal::mainComponent().componentName() : appname),
                         contextList, n->text(), pixmapData, actions, qlonglong(winId));
    if (!reply.isValid())
    {
        kWarning(299) << "error while contacting knotify server" << endl;
    }
    else
    {
        d->notifications.insert(reply, n);
        return reply;
    }
    return 0;
}


void KNotificationManager::remove( int id)
{
    d->notifications.remove(id);
}

void KNotificationManager::update(KNotification * n, int id)
{
	if(id <= 0)
		return;

	QByteArray pixmapData;
	QDataStream arg(&pixmapData, QIODevice::WriteOnly);
	arg << n->pixmap();

	d->knotify->call(/*QDBus::NoWaitForReply,*/"update", id, n->text(), pixmapData , n->actions() );
}

void KNotificationManager::reemit(KNotification * n, int id)
{
	QVariantList contextList;
	typedef QPair<QString,QString> Context;
	foreach (const Context& ctx, n->contexts())
	{
//		kDebug(299) << k_funcinfo << "add context " << ctx.first << "-" << ctx.second  << endl;
		QVariantList vl;
		vl << ctx.first << ctx.second;
		contextList << vl;
	}

	d->knotify->call(/*QDBus::NoWaitForReply,*/"reemit", id, contextList);
}


#include "knotificationmanager_p.moc"
