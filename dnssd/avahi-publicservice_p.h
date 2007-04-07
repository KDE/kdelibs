/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DNSSDPUBLICSERVICE_P_H
#define DNSSDPUBLICSERVICE_P_H

#include <QtCore/QObject>

namespace DNSSD
{

#define AVAHI_ENTRY_GROUP_UNCOMMITED 0
#define AVAHI_ENTRY_GROUP_REGISTERING 1
#define AVAHI_ENTRY_GROUP_ESTABLISHED 2
#define AVAHI_ENTRY_GROUP_COLLISION 3
#define AVAHI_ENTRY_GROUP_FAILURE 4

#define AVAHI_CLIENT_S_REGISTERING 1
#define AVAHI_CLIENT_S_RUNNING 2
#define AVAHI_CLIENT_S_COLLISION 3
#define AVAHI_CLIENT_FAILURE 100

class PublicServicePrivate : public QObject
{
Q_OBJECT
public:
	PublicServicePrivate(PublicService* parent) : QObject(), m_published(false), m_running(false), m_group(0), m_server(0), m_collision(false),
	     m_parent(parent)
	{}
        ~PublicServicePrivate() {  
	    if (m_group) m_group->Free(); 
	    delete m_group;
	    delete m_server;
	}

	bool m_published;
	bool m_running;
	org::freedesktop::Avahi::EntryGroup* m_group;
	org::freedesktop::Avahi::Server* m_server;
	bool m_collision;
	QStringList m_subtypes;
	PublicService* m_parent;
	
	void commit()
	{
	    if (!m_collision) m_group->Commit();
	}    
	void stop();
	bool fillEntryGroup();
	void tryApply();

public Q_SLOTS:
	void serverStateChanged(int,const QString&);
	void groupStateChanged(int,const QString&);
};

}

#endif
