/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "knotificationmanager.h"
#include "knotification.h"

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

#include <QHash>

typedef QHash<QString,QString> Dict;

struct KNotificationManager::Private
{
	QHash<int , KNotification*> notifications;
};

KNotificationManager * KNotificationManager::s_self = 0L;

KNotificationManager * KNotificationManager::self()
{
	static KStaticDeleter<KNotificationManager> deleter;
	if(!s_self)
		deleter.setObject( s_self, new KNotificationManager() );
	return s_self;
}



KNotificationManager::KNotificationManager() : DCOPObject("KNotification") , d(new Private)
{
	
	bool b1=connectDCOPSignal("knotify", "Notify",
					  "notificationClosed(int,int)", 
					  "notificationClosed(int,int)", false);
	bool b2=connectDCOPSignal("knotify", "Notify",
					  "actionInvoked (int,int)", 
					  "notificationActivated(int,int)", false);
	
	kdDebug() << k_funcinfo << b1 << " " << b2 << endl;
}


KNotificationManager::~KNotificationManager()
{
	s_self = 0L;
	delete d;
}


ASYNC KNotificationManager::notificationActivated( int id, int action )
{
	kdDebug() << k_funcinfo << id << " " << action << endl;
	if(d->notifications.contains(id))
	{
		KNotification *n = d->notifications[id];
		kdDebug() << k_funcinfo << n->title() << endl;
		d->notifications.remove(id);
		n->activate( action );
	}
}

ASYNC KNotificationManager::notificationClosed( int id )
{
	if(d->notifications.contains(id))
	{
		KNotification *n = d->notifications[id];
		d->notifications.remove(id);
		n->close();
	}
}


void KNotificationManager::close( int id)
{
	DCOPClient *client=KApplication::dcopClient();
	QByteArray data;
	QDataStream arg(&data, QIODevice::WriteOnly);
	arg << id;
	if (!client->send("knotify", "Notify", "closeNotification( int)", data))
	{
		kdDebug() << k_funcinfo << "error while contacting knotify server" << endl;
	}
}

unsigned int KNotificationManager::notify( KNotification* n , const QPixmap &pix , const QStringList &actions , 
										   const KNotification::ContextList & contexts , int flags)
{
	kdDebug() << k_funcinfo << endl;
	QString appname= (flags & DefaultNotification)  ? QString::fromAscii("kde") : kapp->instanceName();
	WId winId=n->widget() ? n->widget()->topLevelWidget()->winId()  : 0;

	DCOPClient *client=KApplication::dcopClient();
	QByteArray data, replyData;
	DCOPCString replyType;
	QDataStream arg(&data, QIODevice::WriteOnly);
	arg << n->eventId() << appname << contexts << n->text() << pix << actions << winId  ;
	if (!client->call("knotify", "Notify", "event(QString,QString,ContextList,QString,QPixmap,QStringList,int)" ,
		 	 data, replyType, replyData))
	{
		kdDebug() << k_funcinfo << "error while contacting knotify server" << endl;
	}
	else 
	{
		QDataStream reply(&replyData, QIODevice::ReadOnly);
		if (replyType == "int") 
		{
			int result;
			reply >> result;
			d->notifications.insert(result, n);
			kdDebug() << k_funcinfo << "got id " << result << endl;
			return result;
		}
		else
			kdDebug() << k_funcinfo << "bad reply from server" << endl;
	}
	return 0;
}


void KNotificationManager::remove( int id)
{
	d->notifications.remove(id);
}


