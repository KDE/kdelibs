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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "knotifyeventlist.h"

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>


KNotifyEventList::KNotifyEventList(QWidget *parent)
 : QTreeWidget(parent)  , config(0l) , loconf(0l)
{
  QStringList headerLabels;
  headerLabels << i18n( "Title" ) << i18n( "Description" ) << "" << "" << "" << "" << "" ; //<< i18n( "State" );
  setHeaderLabels( headerLabels );
  
  resizeColumnToContents(2);
  resizeColumnToContents(3);
  resizeColumnToContents(4);
  resizeColumnToContents(5);
  resizeColumnToContents(6);

  connect(this, SIGNAL(currentItemChanged( QTreeWidgetItem * , QTreeWidgetItem *  )) , this , SLOT(slotSelectionChanged( QTreeWidgetItem * , QTreeWidgetItem *)));
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
	config= new KConfig(appname +"/" + appname + ".notifyrc" , true, false, "data"),
	loconf= new KConfig(appname + ".notifyrc" , false , false );

	QStringList conflist = config->groupList();
	QRegExp rx("^Event/([^/]*)$");
	conflist=conflist.filter( rx );

	foreach (QString group , conflist )
	{
		config->setGroup(group);
		rx.indexIn(group);
		QString id=rx.cap(1);

		if(!context_name.isEmpty())
		{
			QStringList contexts = config->readEntry("Contexts", QStringList());
			if(!contexts.contains(context_name))
				continue;
			
			id=id+"/"+context_name+"/"+context_value;
		}
		QString name = config->readEntry("Name");
		QString description = config->readEntry("Comment");
		
		m_elements << new KNotifyEventListItem(this, id, name, description, loconf , config );
	}
}

void KNotifyEventList::save( )
{
	foreach( KNotifyEventListItem *it , m_elements )
	{
		it->save();
	}
}

void KNotifyEventList::slotSelectionChanged(  QTreeWidgetItem *current , QTreeWidgetItem *previous)
{
	KNotifyEventListItem *it=dynamic_cast<KNotifyEventListItem *>(currentItem());
	if(it)
		emit eventSelected( it->configElement() );
	else
		emit eventSelected( 0l );
	
	it=dynamic_cast<KNotifyEventListItem *>(previous);
	if(it)
		it->update();
}



KNotifyEventListItem::KNotifyEventListItem( QTreeWidget * parent, const QString & eventName, 
				const QString & name, const QString & description , KConfigBase* locconf , KConfigBase *defconf)
	: QTreeWidgetItem(parent) ,
	m_config(eventName , defconf, locconf )
{
  setText( 0, name );
  setText( 1, description );
  /*setSizeHint ( 2 , QSize(22,22) );
  setSizeHint ( 3 , QSize(22,22) );
  setSizeHint ( 4 , QSize(22,22) );
  setSizeHint ( 5 , QSize(22,22) );
  setSizeHint ( 6 , QSize(22,22) );*/
  update();
}

KNotifyEventListItem::~KNotifyEventListItem()
{
}

void KNotifyEventListItem::save()
{
	m_config.save();
}

void KNotifyEventListItem::update()
{
	QString prstring=m_config.readEntry( "Action" );
	QStringList actions=prstring.split ("|");
	
	QPixmap pexec = SmallIcon("exec");
	QPixmap pstderr = SmallIcon("terminal");
	QPixmap pmessage = SmallIcon("info");
	QPixmap plogfile = SmallIcon("log");
	QPixmap psound = SmallIcon("sound");
	QPixmap ptaskbar = SmallIcon("kicker");

	setIcon(2 , actions.contains("Sound") ? QIcon(psound) : QIcon() );
	setIcon(3 , actions.contains("Popup") ? QIcon(pmessage) : QIcon() );
	setIcon(4 , actions.contains("Execute") ? QIcon(pexec) : QIcon() );
	setIcon(5 , actions.contains("Taskbar") ? QIcon(ptaskbar) : QIcon() );
	setIcon(6 , actions.contains("Logfile") ? QIcon(plogfile) : QIcon() );
}



#include "knotifyeventlist.moc"
