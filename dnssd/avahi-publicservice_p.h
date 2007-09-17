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

#ifndef AVAHI_PUBLICSERVICE_P_H
#define AVAHI_PUBLICSERVICE_P_H

#include <QtCore/QStringList>
#include "servicebase_p.h"
#include <avahi-common/defs.h>
#include "publicservice.h"
#include "avahi_server_interface.h"
#include "avahi_entrygroup_interface.h"

#define K_D PublicServicePrivate* d=static_cast<PublicServicePrivate*>(this->d)

namespace DNSSD
{

class PublicServicePrivate : public QObject, public ServiceBasePrivate
{
Q_OBJECT
public:
	PublicServicePrivate(PublicService* parent, const QString& name, const QString& type, const QString& domain, unsigned int port) : 
	    QObject(), ServiceBasePrivate(name, type, domain, QString(), port), m_published(false), m_running(false), m_group(0), 
	     m_server(0), m_collision(false), m_parent(parent)
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
