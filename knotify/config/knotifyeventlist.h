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


#ifndef KNOTIFYEVENTLIST_H
#define KNOTIFYEVENTLIST_H

#include "knotifyconfigelement.h"
#include <kconfig.h>

#include <QString>
#include <QTreeWidget>

class KNotifyConfigElement;
class KNotifyEventListItem;
class KConfigBase;

/**
	@author Olivier Goffart <ogoffart at kde.org>
*/
class KNotifyEventList : public QTreeWidget
{
	Q_OBJECT
public:
    KNotifyEventList(QWidget *parent);
    ~KNotifyEventList();
	void fill( const QString & appname , const QString & context_name=QString() , 
			   const QString & context_value=QString());
	void save();
	
private:
	KConfig *config;
	KConfig *loconf;
	QList<KNotifyEventListItem*> m_elements;
	
private Q_SLOTS:
	void slotSelectionChanged();
	
Q_SIGNALS:
	void eventSelected(KNotifyConfigElement *);

};

class KNotifyEventListItem : public QTreeWidgetItem
{
	public:
		KNotifyEventListItem(QTreeWidget *parent , const QString & eventName , const QString & name ,
                         const QString & description , KConfigBase* locconf , KConfigBase *defconf);
		~KNotifyEventListItem();
		void save();
		
		KNotifyConfigElement *configElement() { return &m_config; }
		
	private:
		KNotifyConfigElement m_config;
		
};

#endif
