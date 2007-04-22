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

#include "mdnsd-servicetypebrowser_p.h"
#include "servicetypebrowser.h"
#include "servicebrowser.h"

namespace DNSSD
{

ServiceTypeBrowser::ServiceTypeBrowser(const QString& domain,QObject *parent) : QObject(parent),d(new ServiceTypeBrowserPrivate(this))
{
    d->m_browser=new ServiceBrowser("_services._dns-sd._udp",false, domain);
    connect(d->m_browser,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)), d, SLOT(newService(DNSSD::RemoteService::Ptr)));
    connect(d->m_browser,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr)), d, SLOT(removeService(DNSSD::RemoteService::Ptr)));
    connect(d->m_browser,SIGNAL(finished()), this, SIGNAL(finished()));
}

ServiceTypeBrowser::~ServiceTypeBrowser()
{
	delete d;
}

QStringList ServiceTypeBrowser::serviceTypes() const
{
	return d->m_servicetypes;
}


void ServiceTypeBrowser::startBrowse()
{
    d->m_browser->startBrowse();
}

//@todo: remove on Monday
bool ServiceTypeBrowser::isRunning() const
{
	return false;
}

void ServiceTypeBrowserPrivate::newService(DNSSD::RemoteService::Ptr srv)
{
    QString type=srv->serviceName()+'.'+srv->type();
    m_servicetypes+=type;
    emit m_parent->serviceTypeAdded(type);
}    

void ServiceTypeBrowserPrivate::removeService(DNSSD::RemoteService::Ptr srv)
{
    QString type=srv->serviceName()+'.'+srv->type();
    m_servicetypes.removeAll(type);    
    emit m_parent->serviceTypeRemoved(type);
}


}



#include "servicetypebrowser.moc"
#include "mdnsd-servicetypebrowser_p.moc"
