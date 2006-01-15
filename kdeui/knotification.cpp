/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

   code from KNotify/KNotifyClient
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "knotification.h"
#include "knotificationmanager.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kpassivepopup.h>
#include <kactivelabel.h>
#include <kprocess.h>
#include <kdialog.h>
#include <kmacroexpander.h>
#include <kwin.h>
#include <kdebug.h>
#include <kvbox.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QLabel>
#include <QTimer>
#include <QTabWidget>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaEnum>



struct KNotification::Private
{
	QString eventId;
	unsigned int id;
	int ref;
	
	QWidget *widget;
	QString text;
	QStringList actions;
	QPixmap pixmap;
	ContextList contexts;
	QString appname;

	
	Private() :  id(0), ref(1) , widget(0l) {}
};

KNotification::KNotification(QObject *parent) :
		QObject(parent) , d(new Private)
{
}

KNotification::~KNotification()
{
	if(d ->id != 0)
		KNotificationManager::self()->remove( d->id );
	delete d;
}

QString KNotification::eventId() const
{
	return d->eventId;
}

QString KNotification::text() const
{
	return d->text;
}

QWidget *KNotification::widget() const
{
	return d->widget;
}



void KNotification::activate(unsigned int action)
{
	if(action==0)
		emit activated();
	emit activated(action);
	kdDebug() << k_funcinfo << d->id << " " << action << endl;
	deleteLater();
}


void KNotification::close()
{
	if(d->id != 0)
		KNotificationManager::self()->close( d->id );
	emit closed();
	deleteLater();
}


void KNotification::raiseWidget()
{
	if(!d->widget)
		return;

	raiseWidget(d->widget);
}


void KNotification::raiseWidget(QWidget *w)
{
	//TODO  this funciton is far from finished.
	if(w->isTopLevel())
	{
		w->raise();
#ifdef Q_WS_WIN
		w->activateWindow();
#else
		KWin::activateWindow( w->winId() );
#endif
	}
	else
	{
		QWidget *pw=w->parentWidget();
		raiseWidget(pw);

		if( QTabWidget *tab_widget=dynamic_cast<QTabWidget*>(pw))
		{
			tab_widget->showPage(w);
		}
	}
}


KNotification *KNotification::event( const QString& eventid , const QString& text,
			const QPixmap& pixmap, QWidget *widget, const QStringList &actions,
			ContextList contexts, NotificationFlags flags, const KInstance *instance)
{
	KNotification *notify=new KNotification(widget);
	notify->d->widget=widget;
	notify->d->text=text;
	notify->d->actions=actions;
	notify->d->eventId=eventid;
	notify->d->contexts=contexts;
	notify->d->pixmap=pixmap;

	QString appname; 

	if(flags & DefaultEvent)
		appname = QLatin1String("kde");
	else if(instance)
		appname = QString::fromLatin1(instance->instanceName());
	else
		appname = QString::fromLatin1(kapp->instanceName());
	
	notify->d->appname=appname;

	if(! flags & NoTimeout)
	{
		QTimer::singleShot(6*1000, notify, SLOT(close()));
	}

	QTimer::singleShot(0,notify,SLOT(sendEvent()));
	
	return notify;
}


KNotification *KNotification::event( StandardEvent eventid , const QString& text,
		const QPixmap& pixmap, QWidget *widget, NotificationFlags flags)
{
	QString message;
	switch ( eventid ) {
		case Warning:
			message = QLatin1String("warning");
			break;
		case Error:
			message = QLatin1String("fatalerror");
			break;
		case Catastrophe:
			message = QLatin1String("catastrophe");
			break;
		case Notification: // fall through
		default:
			message = QLatin1String("notification");
			break;
	}
	return event( message, text, pixmap, widget , QStringList() , ContextList() , flags | DefaultEvent );
}

void KNotification::ref()
{
	d->ref++;
}

void KNotification::deref()
{
	d->ref--;
	if(d->ref==0)
		close();
}

void KNotification::beep( const QString & reason, QWidget * widget )
{
	event( QLatin1String("beep"), reason, QPixmap(), widget , QStringList() , ContextList() , CloseOnTimeout | DefaultEvent );
}

void KNotification::sendEvent()
{
	d->id=KNotificationManager::self()->notify( this , d->pixmap , d->actions , d->contexts , d->appname );
	if(d->id>0)
		ref();
//	kdDebug() << k_funcinfo << d->id << endl;
	
	//after a small timeout, the notification will be deleted if all presentation are finished
	QTimer::singleShot(1000, this, SLOT(deref()));


}

#include "knotification.moc"
