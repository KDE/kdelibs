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
#include <kwindowsystem.h>
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
#include <QDBusError>

struct KNotification::Private
{
    QString eventId;
    int id;
    int ref;

    QWidget *widget;
    QString title;
    QString text;
    QStringList actions;
    QPixmap pixmap;
    ContextList contexts;
    NotificationFlags flags;
    KComponentData componentData;

    QTimer updateTimer;
    bool needUpdate;

    Private() : id(0), ref(1), widget(0l), needUpdate(false) {}
    /**
     * recursive function that raise the widget. @p w
     *
     * @see raiseWidget()
     */
    static void raiseWidget(QWidget *w);
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

KNotification::KNotification(
		const QString& eventId,
		const NotificationFlags& flags,
		QObject *parent)
	:	QObject(parent),
		d(new Private)
{
	d->eventId=eventId;
	d->flags=flags;
	connect(&d->updateTimer,SIGNAL(timeout()), this, SLOT(update()));
	d->updateTimer.setSingleShot(true);
	d->updateTimer.setInterval(100);
}


KNotification::~KNotification()
{
	if(d ->id > 0)
		KNotificationManager::self()->close( d->id );
	delete d;
}

QString KNotification::eventId() const
{
	return d->eventId;
}

QString KNotification::title() const
{
    return d->title;
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
    if ( wid && d->flags &  CloseWhenWidgetActivated ) {
        wid->installEventFilter(this);
    }
}

void KNotification::setTitle(const QString &title)
{
    d->needUpdate = true;
    d->title = title;
    if(d->id > 0)
        d->updateTimer.start();
}

void KNotification::setText(const QString &text)
{
    d->needUpdate = true;
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
    d->needUpdate = true;
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
    d->needUpdate = true;
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


void KNotification::setComponentData(const KComponentData &c)
{
    d->componentData = c;
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
	if(d->id != -1)
		deleteLater();
	d->id = -2;
}


void KNotification::close()
{
	if(d->id >= 0)
		KNotificationManager::self()->close( d->id );
	if(d->id != -1) //=-1 mean still waiting for receiving the id
		deleteLater();
	d->id = -2;
	emit closed();
}


void KNotification::raiseWidget()
{
    if ( !d->widget ) {
        return;
    }

    Private::raiseWidget( d->widget );
}


void KNotification::Private::raiseWidget(QWidget *w)
{
	//TODO  this function is far from finished.
	if(w->isTopLevel())
	{
		w->raise();
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
		w->activateWindow();
#else
		KWindowSystem::activateWindow( w->winId() );
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

KNotification *KNotification::event( const QString& eventid , const QString& title, const QString& text,
        const QPixmap& pixmap, QWidget *widget, const NotificationFlags &flags, const KComponentData &componentData)
{
	KNotification *notify=new KNotification(eventid, widget, flags);
	notify->setTitle(title);
	notify->setText(text);
	notify->setPixmap(pixmap);
	notify->setComponentData(componentData);

	QTimer::singleShot(0,notify,SLOT(sendEvent()));

	return notify;
}

KNotification *KNotification::event( const QString& eventid , const QString& text,
        const QPixmap& pixmap, QWidget *widget, const NotificationFlags &flags, const KComponentData &componentData)
{
	return event( eventid, QString(), text, pixmap, widget, flags, componentData );
}


KNotification *KNotification::event( StandardEvent eventid , const QString& title, const QString& text,
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
	return event( message, title, text, pixmap, widget , flags | DefaultEvent );
}

KNotification *KNotification::event( StandardEvent eventid , const QString& text,
        const QPixmap& pixmap, QWidget *widget, const NotificationFlags &flags)
{
    return event( eventid, QString(), text, pixmap, widget , flags );
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
    d->needUpdate = false;
	if(d->id == 0)
	{
		QString appname;

		if(d->flags & DefaultEvent)
			appname = QLatin1String("kde");
		else if(d->componentData.isValid()) {
			appname = d->componentData.componentName();
		} else {
			appname = KGlobal::mainComponent().componentName();
		}

		if (KNotificationManager::self()->notify( this , d->pixmap , d->actions , d->contexts , appname ))
			d->id = -1;
	}
	else if(d->id > 0)
	{
		KNotificationManager::self()->reemit(this , d->id );
	}
    else if(d->id == -1)
    {
        //schedule an update.
        d->needUpdate = true;
    }
}

void KNotification::slotReceivedId(int id)
{
	if(d->id == -2) //we are already closed
	{
		KNotificationManager::self()->close( id, /*force=*/ true );
		deleteLater();
		return;
	}
	d->id=id;
	if(d->id>0)
	{
		KNotificationManager::self()->insert(this,d->id);
        if (d->needUpdate)
            sendEvent();
	}
	else
	{
		//if there is no presentation, delete the object
		QTimer::singleShot(0, this, SLOT(deref()));
	}

}

void KNotification::slotReceivedIdError(const QDBusError& error)
{
	if(d->id == -2) //we are already closed
	{
		deleteLater();
		return;
	}
	kWarning(299) << "Error while contacting notify daemon" << error.message();
	d->id = -3;
	QTimer::singleShot(0, this, SLOT(deref()));
}


void KNotification::update()
{
	KNotificationManager::self()->update(this, d->id);
}

bool KNotification::eventFilter( QObject * watched, QEvent * event )
{
	if( watched == d->widget )
	{
		if( event->type() == QEvent::WindowActivate )
		{
			if( d->flags &  CloseWhenWidgetActivated )
				QTimer::singleShot(500, this, SLOT(close()));
		}
		//kDebug(299) << event->type();
	}

	return false;
}


#include "knotification.moc"
