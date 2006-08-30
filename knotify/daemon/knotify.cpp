/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


// KDE headers
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <config.h>

#include "knotify.h"
#include "knotifyconfig.h"
#include "notifybysound.h"
#include "notifybypopup.h"
#include "notifybyexecute.h"
#include "notifybylogfile.h"
#include "notifybytaskbar.h"



KNotify::KNotify( QObject *parent )
    : QObject( parent ),
    m_counter(0)
{
	loadConfig();
	(void)new KNotifyAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/Notify", this, QDBusConnection::ExportAdaptors 
	 /*|  QDBusConnection::ExportScriptableSlots |  QDBusConnection::ExportScriptableSignals*/ );
}

KNotify::~KNotify()
{
}


void KNotify::loadConfig() 
{
	qDeleteAll(m_plugins);
	m_plugins.clear();
	addPlugin(new NotifyBySound(this));
	addPlugin(new NotifyByPopup(this));
	addPlugin(new NotifyByExecute(this));
	addPlugin(new NotifyByLogfile(this));
	addPlugin(new NotifyByTaskbar(this));
}

void KNotify::addPlugin( KNotifyPlugin * p )
{
	m_plugins[p->optionName()]=p;
	connect(p,SIGNAL(finished( int )) , this , SLOT(slotPluginFinished( int ) ));
	connect(p,SIGNAL(actionInvoked( int , int )) , this , SIGNAL(notificationActivated( int , int ) ));
}



void KNotify::reconfigure()
{
	KGlobal::config()->reparseConfiguration();
	loadConfig();
}

void KNotify::closeNotification(int id)
{
	if(!m_notifications.contains(id))
		return;
	Event &e=m_notifications[id];
	if(e.ref>0)
	{
		e.ref++;
		KNotifyPlugin *plugin;
		foreach(plugin , m_plugins)
		{
			plugin->close( id );
		}
	}
	
	notificationClosed(id);
	m_notifications.remove(id);

}

int KNotify::event( const QString & event, const QString & appname, const ContextList & contexts, const QString & text, const QPixmap & pixmap, const QStringList & actions, WId winId )
{
	m_counter++;
	Event e(appname , contexts , event );
	e.id = m_counter;
	e.ref = 1;

	e.config.text=text;
	e.config.actions=actions;
	e.config.pix=pixmap;
	e.config.winId=(WId)winId;
	
	m_notifications.insert(m_counter,e);
	
	emitEvent(e);
	
	e.ref--;
//	kDebug(300) << k_funcinfo << e.id << " ref=" << e.ref << endl;
	if(e.ref==0)
	{
		m_notifications.remove(e.id);
		return 0;
	}
	return m_counter;
}

void KNotify::update(int id, const QString &text, const QPixmap& pixmap,  const QStringList& actions)
{
	if(!m_notifications.contains(id))
		return;

	Event &e=m_notifications[id];
	
	e.config.text=text;
	e.config.pix = pixmap;
	e.config.actions = actions;
	
	foreach(KNotifyPlugin *p, m_plugins)
	{
		p->update(id, &e.config);
	}
}
void KNotify::reemit(int id, const ContextList& contexts)
{
	if(!m_notifications.contains(id))
		return;
	Event &e=m_notifications[id];
	e.config.contexts=contexts;
	
	emitEvent(e);
}

void KNotify::emitEvent(Event &e)
{
	QString presentstring=e.config.readEntry("Action");
	QStringList presents=presentstring.split ("|");
	
	foreach(const QString & action , presents)
	{
		if(!m_plugins.contains(action))
			continue;
		KNotifyPlugin *p=m_plugins[action];
		e.ref++;
		p->notify(e.id,&e.config);
	}
}

void KNotify::slotPluginFinished( int id )
{
	if(!m_notifications.contains(id))
		return;
	Event &e=m_notifications[id];
	e.ref--;
	if(e.ref==0)
		closeNotification( id );
}

KNotifyAdaptor::KNotifyAdaptor(QObject *parent)
	: QDBusAbstractAdaptor(parent)
{
	setAutoRelaySignals(true);
}

void KNotifyAdaptor::reconfigure()
{
	static_cast<KNotify *>(parent())->reconfigure();
}

void KNotifyAdaptor::closeNotification(int id)
{
	static_cast<KNotify *>(parent())->closeNotification(id);
}

int KNotifyAdaptor::event(const QString &event, const QString &fromApp, const QVariantList& contexts,
						   const QString &text, const QByteArray& image,  const QStringList& actions , qlonglong winId)
//						  const QDBusMessage & , int _return )
								  
{
	/* I'm not sure this is the right way to read a a(ss) type,  but it seems to work */
	ContextList contextlist;
	QString context_key;
	foreach( QVariant v , contexts)
	{
		/* this code doesn't work
		QVariantList vl=v.toList();
		if(vl.count() != 2)
		{
			kWarning(300) << k_funcinfo << "Bad structure passed as argument" << endl;
			continue;
		}
		contextlist << qMakePair(vl[0].toString() , vl[1].toString());*/
		QString s=v.toString();
		if(context_key.isEmpty())
			context_key=s;
		else
			contextlist << qMakePair(context_key , s);
	}
	
	QPixmap pixmap;
	QDataStream in(image);
	in >> pixmap;
	return static_cast<KNotify *>(parent())->event(event, fromApp, contextlist, text, pixmap, actions, WId(winId));
}

void KNotifyAdaptor::reemit(int id, const QVariantList& contexts)
{
	ContextList contextlist;
	QString context_key;
	foreach( QVariant v , contexts)
	{
		QString s=v.toString();
		if(context_key.isEmpty())
			context_key=s;
		else
			contextlist << qMakePair(context_key , s);
	}
	static_cast<KNotify *>(parent())->reemit(id, contextlist);
}


void KNotifyAdaptor::update(int id, const QString &text, const QByteArray& image,  const QStringList& actions )
{
	QPixmap pixmap;
	pixmap.loadFromData(image);
	static_cast<KNotify *>(parent())->update(id, text, pixmap, actions);
}

#include "knotify.moc"

// vim: sw=4 sts=4 ts=8 et


