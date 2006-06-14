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
	QDBus::sessionBus().registerObject("/Notify", this, QDBusConnection::ExportAdaptors 
	 /*|  QDBusConnection::ExportSlots |  QDBusConnection::ExportSignals*/ );
}

KNotify::~KNotify()
{
}


void KNotify::loadConfig() 
{
	qDeleteAll(m_plugins);
	m_plugins.clear();
	addPlugin(new NotifyBySound(&parent_workaround));
	addPlugin(new NotifyByPopup(&parent_workaround));
	addPlugin(new NotifyByExecute(&parent_workaround));
	addPlugin(new NotifyByLogfile(&parent_workaround));
	addPlugin(new NotifyByTaskbar(&parent_workaround));
}

void KNotify::addPlugin( KNotifyPlugin * p )
{
	m_plugins[p->optionName()]=p;
	connect(p,SIGNAL(finished( int )) , this , SLOT(slotPluginFinished( int ) ));
	connect(p,SIGNAL(actionInvoked( int , int )) , this , SIGNAL(actionInvoked( int , int ) ));
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

int KNotify::event( const QString & event, const QString & appname, const ContextList & contexts, const QString & text, const QPixmap & pixmap, const QStringList & actions, int winId )
{
	m_counter++;
	KNotifyConfig config(appname , contexts , event );
	config.text=text;
	config.actions=actions;
	config.pix=pixmap;
	config.winId=(WId)winId;
	
	Event e;
	e.id = m_counter;
	e.ref = 1;
	m_notifications[m_counter]=e;
	
	QString presentstring=config.readEntry("Action");
	QStringList presents=presentstring.split ("|");
	
    kDebug(300) << k_funcinfo << event << " ("<< m_counter  << ") : " << presents << endl;
	
	foreach(const QString & action , presents)
	{
		if(!m_plugins.contains(action))
			continue;
		KNotifyPlugin *p=m_plugins[action];
		p->notify(m_counter,&config);
		m_notifications[m_counter].ref++;
	}
	m_notifications[m_counter].ref--;
	if(m_notifications[m_counter].ref==0)
	{
		m_notifications.remove(m_counter);
		return 0;
	}
	return m_counter;
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
	static_cast<KNotify *>(object())->reconfigure();
}

void KNotifyAdaptor::closeNotification(int id)
{
	static_cast<KNotify *>(object())->closeNotification(id);
}

void KNotifyAdaptor::event(const QString &event, const QString &fromApp, const QVariantList& contexts,
						   const QString &text, const QByteArray& image,  const QStringList& actions , int winId)
//						  const QDBusMessage & , int _return )
								  
{
	ContextList contextlist;
	foreach( QVariant v , contexts)
	{
		QVariantList vl=v.toList();
		if(vl.count() != 2)
		{
			kWarning(300) << k_funcinfo << "Bad structure passed as argument" << endl;
			continue;
		}
		contextlist << qMakePair(vl[0].toString() , vl[1].toString());
	}
	
	QPixmap pixmap;
	pixmap.loadFromData(image);
	 static_cast<KNotify *>(object())->event(event, fromApp, contextlist, text, pixmap, actions, winId);
}

#include "knotify.moc"

// vim: sw=4 sts=4 ts=8 et
