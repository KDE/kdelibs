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
#include "knotifyeventlist.h"

#include <kdebug.h>

KNotifyEventList::KNotifyEventList(QWidget *parent)
 : KListView(parent)  , config(0l) , loconf(0l)
{
	addColumn("Titre");
	addColumn("Description");
	addColumn("Etat");
	connect(this, SIGNAL(selectionChanged()) , this , SLOT(slotSelectionChanged()));
}


KNotifyEventList::~KNotifyEventList()
{
	delete config;
	delete loconf;
}

void KNotifyEventList::fill( const QString & appname , const QString & context_name ,const QString & context_value )
{
	m_elements.clear();
	clear();
	delete config;
	delete loconf;
	config= new KConfig(appname + "/eventsrc" , true, false, "data"),
	loconf= new KConfig(appname + ".eventsrc" , false , false );

	QStringList conflist = config->groupList();
	QRegExp rx("^Event/([^/]*)$");
	conflist=conflist.filter( rx );
// 	kdDebug() << k_funcinfo << conflist << endl;

	foreach (QString group , conflist )
	{
		config->setGroup(group);
		rx.indexIn(group);
		QString id=rx.cap(1);

		if(!context_name.isEmpty())
		{
			QStringList contexts = config->readListEntry("Contexts");
			if(!contexts.contains(context_name))
				continue;
			
			id=id+"/"+context_name+"/"+context_value;
		}
		QString name = config->readEntry("Name");
		QString description = config->readEntry("Comment");
		
// 		kdDebug() << k_funcinfo << id <<endl;
		m_elements << new KNotifyEventListItem(this, id, name, description, loconf , config );
	}
}

void KNotifyEventList::save( )
{
	kdDebug() << k_funcinfo << endl;

	foreach( KNotifyEventListItem *it , m_elements )
	{
		kdDebug() << k_funcinfo << "*" <<  endl;
		it->save();
	}
}

void KNotifyEventList::slotSelectionChanged( )
{
	KNotifyEventListItem *it=dynamic_cast<KNotifyEventListItem *>(currentItem());
	if(it)
		emit eventSelected( it->configElement() );
/*	else
		emit eventSelected( 0l );*/
}



KNotifyEventListItem::KNotifyEventListItem( KListView * parent, const QString & eventName, 
				const QString & name, const QString & description , KConfigBase* locconf , KConfigBase *defconf)
	: KListViewItem(parent, name , description) ,
	m_config(eventName , defconf, locconf )
{
	
}


void KNotifyEventListItem::save()
{
	m_config.save();
}


