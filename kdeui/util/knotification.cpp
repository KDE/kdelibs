/* This file is part of the KDE libraries
   Copyright (C) 2005-2006 Olivier Goffart <ogoffart at kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "knotification.h"
#include "knotificationmanager_p.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kpassivepopup.h>
#include <kdialog.h>
#include <kmacroexpander.h>
#include <kwin.h>
#include <kdebug.h>
#include <kvbox.h>
#include <kapplication.h>

#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QLabel>
#include <QTimer>
#include <QTabWidget>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>

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
	NotificationFlags flags;
	const KInstance *instance;
	
	QTimer updateTimer;

	Private() :  id(0), ref(1) , widget(0l) , instance(0L) {}
};

KNotification::KNotification(const QString& eventId, QWidget *parent, const NotificationFlags& flags) :
		QObject(parent) , d(new Private)
{
	d->eventId=eventId;
	d->flags=flags;
	setWidget(parent);
	connect(&d->updateTimer,SIGNAL(timeout()), this, SLOT(update()));
	d->updateTimer.setSingleShot(true);
	d->updateTimer.setInterval(100);
}

KNotification::~KNotification()
{
//	kDebug( 299 ) << k_funcinfo << d->id << endl;
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

void KNotification::setWidget(QWidget *wid)
{
	d->widget = wid;
	setParent(wid);
}

void KNotification::setText(const QString &text)
{
	d->text=text;
	if(d->id > 0)
		d->updateTimer.start();
}
	
QPixmap KNotification::pixmap() const
{
	return d->pixmap;
}
	
void KNotification::setPixmap(const QPixmap &pix)
{
	d->pixmap=pix;
	if(d->id > 0)
		d->updateTimer.start();
}

QStringList KNotification::actions() const
{
	return d->actions;
}

void KNotification::setActions(const QStringList& as )
{
	d->actions=as;
	if(d->id > 0)
		d->updateTimer.start();
}

KNotification::ContextList KNotification::contexts() const
{
	return d->contexts;
}
	
void KNotification::setContexts( const KNotification::ContextList &contexts)
{
	d->contexts=contexts;
}

void KNotification::addContext( const KNotification::Context & context)
{
	d->contexts << context;
}

void KNotification::addContext( const QString & context_key, const QString & context_value )
{
	d->contexts << qMakePair( context_key , context_value );
}

KNotification::NotificationFlags KNotification::flags() const
{
    return d->flags;
}

void KNotification::setFlags(const NotificationFlags & flags)
{
    d->flags=flags;
}


void KNotification::setInstance( const KInstance * i)
{
	d->instance=i;
}

void KNotification::activate(unsigned int action)
{
    switch (action)
    {
        case 0:
            emit activated();
            break;
        case 1:
            emit action1Activated();
            break;
        case 2:
            emit action2Activated();
            break;
        case 3:
            emit action3Activated();
            break;
    }
	emit activated(action);
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
	//TODO  this function is far from finished.
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

		if( QTabWidget *tab_widget=qobject_cast<QTabWidget*>(pw))
		{
			tab_widget->setCurrentIndex(tab_widget->indexOf(w));
		}
	}
}


KNotification *KNotification::event( const QString& eventid , const QString& text,
			const QPixmap& pixmap, QWidget *widget, const NotificationFlags &flags, const KInstance *instance)
{
	KNotification *notify=new KNotification(eventid, widget, flags);
	notify->setText(text);
	notify->setPixmap(pixmap);
	notify->setInstance(instance);

	QTimer::singleShot(0,notify,SLOT(sendEvent()));
	
	return notify;
}


KNotification *KNotification::event( StandardEvent eventid , const QString& text,
		const QPixmap& pixmap, QWidget *widget, const NotificationFlags &flags)
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
	return event( message, text, pixmap, widget , flags | DefaultEvent );
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
	event( QLatin1String("beep"), reason, QPixmap(), widget , CloseOnTimeout | DefaultEvent );
}

void KNotification::sendEvent()
{
	QTimer::singleShot(0, this, SLOT(sendEventSync()));
}

void KNotification::sendEventSync()
{
	if(d->id<=0)
	{
		QString appname;
		
		if(d->flags & DefaultEvent)
			appname = QLatin1String("kde");
		else if(d->instance)
			appname = QString::fromLatin1(d->instance->instanceName());
		else
			appname = QString::fromLatin1(KGlobal::instance()->instanceName());
		
		if(!(d->flags & Persistant))
		{
			QTimer::singleShot(6*1000, this, SLOT(close()));
		}
	
		
		d->id=KNotificationManager::self()->notify( this , d->pixmap , d->actions , d->contexts , appname );
		if(d->id>0)
			ref();
//		kDebug(299) << k_funcinfo << d->id << endl;
		
		//after a small timeout, the notification will be deleted if all presentation are finished
		QTimer::singleShot(100, this, SLOT(deref()));
	}
	else
	{
		KNotificationManager::self()->reemit(this , d->id );
	}
}

void KNotification::update()
{
	KNotificationManager::self()->update(this, d->id);
}


#include "knotification.moc"
