/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart at kde.org>

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


#include <QTreeWidget>

class KNotifyConfigElement;
class KNotifyEventListItem;
class KConfig;

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
	void updateCurrentItem();
        QSize sizeHint() const;
private:
	KConfig *config;
	QList<KNotifyEventListItem*> m_elements;
	
	class KNotifyEventListDelegate;
	
private Q_SLOTS:
	void slotSelectionChanged(QTreeWidgetItem *current , QTreeWidgetItem *previous);
	
Q_SIGNALS:
	void eventSelected(KNotifyConfigElement *);

};

class KNotifyEventListItem : public QTreeWidgetItem
{
	public:
		KNotifyEventListItem(QTreeWidget *parent , const QString & eventName , const QString & name ,
                         const QString & description , KConfig* confir);
		~KNotifyEventListItem();
		void save();
		
		KNotifyConfigElement *configElement() { return &m_config; }
		
		void update();
		
	private:
		KNotifyConfigElement m_config;
		
};

#endif
