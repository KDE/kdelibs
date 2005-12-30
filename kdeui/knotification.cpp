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
#include <QMap>
#include <QPixmap>
#include <dcopclient.h>
#include <q3cstring.h>
#include <qpointer.h>
#include <q3stylesheet.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <kapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaEnum>



struct KNotification::Private
{
	QWidget *widget;
	QString text;
	QStringList actions;
	int level;
	QString eventId;
	QString title;
	unsigned int id;
	
	int ref;
	
	Private() : widget(0l) , id(0), ref(1) {}
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

QString KNotification::title() const
{
	return d->title;
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
		KWin::activateWindow( w->winId() );
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
			ContextList contexts, unsigned int flags)
{
	QString appname = QString::fromAscii(kapp->instanceName());
	KNotification *notify=new KNotification(widget);
	notify->d->widget=widget;
	notify->d->text=text;
	notify->d->actions=actions;
	notify->d->eventId=eventid;

	notify->d->id=KNotificationManager::self()->notify( notify , pixmap , notify->d->actions , contexts );
	if(notify->d->id>0)
		notify->ref();
//	kdDebug() << k_funcinfo << d->id << endl;

	if(flags & CloseOnTimeout)
	{
		QTimer::singleShot(6*1000, notify, SLOT(close()));
	}

	
	//after a small timeout, the notification will be deleted if all presentation are finished
	QTimer::singleShot(1000, notify, SLOT(deref()));
	
	return notify;
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
	QApplication::beep();
	//TODO use kde configuration
}

#include "knotification.moc"



