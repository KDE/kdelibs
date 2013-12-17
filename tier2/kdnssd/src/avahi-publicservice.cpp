/* This file is part of the KDE project
 *
 * Copyright (C) 2004, 2005 Jakub Stachowski <qbast@go2.pl>
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

#include "avahi-publicservice_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

#include "publicservice.h"

#include <config-kdnssd.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "servicebrowser.h"
#include "avahi_server_interface.h"
#include "avahi_entrygroup_interface.h"

namespace KDNSSD
{

PublicService::PublicService(const QString& name, const QString& type, unsigned int port,
			      const QString& domain, const QStringList& subtypes)
  		: QObject(), ServiceBase(new PublicServicePrivate(this, name, type, domain, port))
{
	K_D;
	if (domain.isNull()) d->m_domain="local.";
	d->m_subtypes=subtypes;
}


PublicService::~PublicService()
{
    stop();
}

void PublicServicePrivate::tryApply()
{
    if (fillEntryGroup()) commit();
    else {
	m_parent->stop();
	emit m_parent->published(false);
    }
}

void PublicService::setServiceName(const QString& serviceName)
{
	K_D;
	d->m_serviceName = serviceName;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
	} 
}

void PublicService::setDomain(const QString& domain)
{
	K_D;
	d->m_domain = domain;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
	} 
}


void PublicService::setType(const QString& type)
{
	K_D;
	d->m_type = type;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
	} 
}

void PublicService::setSubTypes(const QStringList& subtypes)
{
	K_D;
	d->m_subtypes = subtypes;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
	} 
}

QStringList PublicService::subtypes() const
{
	K_D;
	return d->m_subtypes;
}

void PublicService::setPort(unsigned short port)
{
	K_D;
	d->m_port = port;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
    	} 
}

void PublicService::setTextData(const QMap<QString,QByteArray>& textData)
{
	K_D;
	d->m_textData = textData;
	if (d->m_running) {
	    d->m_group->Reset();
	    d->tryApply();
	} 
}

bool PublicService::isPublished() const
{
	K_D;
	return d->m_published;
}

bool PublicService::publish()
{
	K_D;
	publishAsync();
	while (d->m_running && !d->m_published) QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	return d->m_published;
}

void PublicService::stop()
{
	K_D;
        if (d->m_group) d->m_group->Reset();
        d->m_running = false;
	d->m_published = false;
}
bool PublicServicePrivate::fillEntryGroup()
{
    registerTypes();
    if (!m_group) {
	QDBusReply<QDBusObjectPath> rep=m_server->EntryGroupNew();
	if (!rep.isValid()) return false;
	m_group=new org::freedesktop::Avahi::EntryGroup("org.freedesktop.Avahi",rep.value().path(), QDBusConnection::systemBus());
        connect(m_group,SIGNAL(StateChanged(int,QString)), this, SLOT(groupStateChanged(int,QString)));
    }
    if (m_serviceName.isNull()) {
	QDBusReply<QString> rep=m_server->GetHostName();
	if (!rep.isValid()) return false;
	m_serviceName=rep.value();
    }
	
    QList<QByteArray> txt;
    QMap<QString,QByteArray>::ConstIterator itEnd = m_textData.constEnd();
    for (QMap<QString,QByteArray>::ConstIterator it = m_textData.constBegin(); it!=itEnd ; ++it) 
    	if (it.value().isNull()) txt.append(it.key().toLatin1());
	else txt.append(it.key().toLatin1()+'='+it.value());
	
	
    for (;;) {
        QDBusReply<void> ret = m_group->AddService(-1,-1, 0, m_serviceName, m_type , domainToDNS(m_domain) ,
    	m_hostName, m_port,txt);
        if (ret.isValid()) break;
        
        // serious error, bail out
        if (ret.error().name()!=QLatin1String("org.freedesktop.Avahi.CollisionError")) return false;
        
        // name collision, try another
        QDBusReply<QString> rep=m_server->GetAlternativeServiceName(m_serviceName);
        if (rep.isValid()) m_serviceName = rep.value();
        else return false;
    }
    
    Q_FOREACH(const QString &subtype, m_subtypes) 
	m_group->AddServiceSubtype(-1,-1, 0, m_serviceName, m_type, domainToDNS(m_domain) , subtype);
    return true;
}

void PublicServicePrivate::serverStateChanged(int s,const QString&)
{
    if (!m_running) return;
    switch (s) {
	case AVAHI_SERVER_INVALID:
	    m_parent->stop();
	    emit m_parent->published(false);
	    break;
	case AVAHI_SERVER_REGISTERING:
	case AVAHI_SERVER_COLLISION:
	    if (m_group) m_group->Reset();
	    m_collision=true;
	    break;
	case AVAHI_SERVER_RUNNING:
	    if (m_collision) {
		m_collision=false;
		tryApply();
	    }
    }
}				    

void PublicService::publishAsync()
{
	K_D;
	if (d->m_running) stop();
	
	if (!d->m_server) {
    	    d->m_server = new org::freedesktop::Avahi::Server("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	    connect(d->m_server,SIGNAL(StateChanged(int,QString)),d,SLOT(serverStateChanged(int,QString)));
	}
	
	int state=AVAHI_SERVER_INVALID;
	QDBusReply<int> rep=d->m_server->GetState();
	
	if (rep.isValid()) state=rep.value();
	d->m_running=true; 
	d->m_collision=true; // make it look like server is getting out of collision to force registering
	d->serverStateChanged(state, QString());
}


void PublicServicePrivate::groupStateChanged(int s,  const QString& reason)
{
    switch (s) {
    case AVAHI_ENTRY_GROUP_COLLISION: {
	    QDBusReply<QString> rep=m_server->GetAlternativeServiceName(m_serviceName);
	    if (rep.isValid())  m_parent->setServiceName(rep.value());
	    else serverStateChanged(AVAHI_SERVER_INVALID, reason);
	    break;
	    }
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
	    m_published=true;
	    emit m_parent->published(true);
	    break;
    case AVAHI_ENTRY_GROUP_FAILURE:
	    serverStateChanged(AVAHI_SERVER_INVALID, reason);
    default: 
	break;
    }
}

void PublicService::virtual_hook(int, void*)
{
}


}

#include "moc_publicservice.cpp"
#include "moc_avahi-publicservice_p.cpp"
