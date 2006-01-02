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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */


// KDE headers
#include <kapplication.h>
#include <dcopclient.h>
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
    : QObject( parent ), DCOPObject("Notify") ,
    m_counter(0)
{
	loadConfig();
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
	connect(p,SIGNAL(actionInvoked( int , int )) , this , SLOT(actionInvoked( int , int ) ));

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
	
	notificatonClosed(id);
	m_notifications.remove(id);

}

int KNotify::event( const QString & event, const QString & appname, const ContextList & contexts, const QString & text, const QPixmap & pixmap, const QStringList & actions, int winId )
{
	m_counter++;
	kdDebug() << k_funcinfo << event << " : " << m_counter << "  winid:" << winId << endl;

	KNotifyConfig config(appname , contexts , event );
	config.text=text;
	config.actions=actions;
	config.pix=pixmap;
	config.winId=winId;
	
	Event e;
	e.id = m_counter;
	e.ref = 1;
	m_notifications[m_counter]=e;
	
	QString presentstring=config.readEntry("Action");
	QStringList presents=presentstring.split ("|");
	
	kdDebug() << k_funcinfo << presentstring << " - " << presents << endl;
	
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


#include "knotify.moc"

// vim: sw=4 sts=4 ts=8 et
