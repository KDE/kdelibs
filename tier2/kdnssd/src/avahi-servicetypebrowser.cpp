/* This file is part of the KDE project
 *
 * Copyright (C) 2004,2007 Jakub Stachowski <qbast@go2.pl>
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


#include "avahi-servicetypebrowser_p.h"
#include <QtCore/QSet>
#include "avahi_server_interface.h"
#include "servicetypebrowser.h"
#include "avahi_servicetypebrowser_interface.h"

#define UNSPEC -1
namespace KDNSSD
{

ServiceTypeBrowser::ServiceTypeBrowser(const QString& domain, QObject *parent) : QObject(parent), d(new ServiceTypeBrowserPrivate(this))
{
	d->m_domain=domain;
	d->m_timer.setSingleShot(true);
}

ServiceTypeBrowser::~ServiceTypeBrowser()
{
    delete d;
}

void ServiceTypeBrowser::startBrowse()
{
	if (d->m_started) return;
	d->m_started=true;
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	QDBusReply<QDBusObjectPath> rep=s.ServiceTypeBrowserNew(-1, -1, d->m_domain, 0);

	if (!rep.isValid()) return;
	org::freedesktop::Avahi::ServiceTypeBrowser *b=new org::freedesktop::Avahi::ServiceTypeBrowser("org.freedesktop.Avahi",rep.value().path(),
	    QDBusConnection::systemBus());
	connect(b,SIGNAL(ItemNew(int,int,QString,QString,uint)),d, SLOT(gotNewServiceType(int,int,QString,QString,uint)));
	connect(b,SIGNAL(ItemRemove(int,int,QString,QString,uint)),d, SLOT(gotRemoveServiceType(int,int,QString,QString,uint)));
	connect(b,SIGNAL(AllForNow()),d,SLOT(finished()));
	connect(&d->m_timer,SIGNAL(timeout()), d, SLOT(finished()));
	d->m_browser=b;
	d->m_timer.start(domainIsLocal(d->m_domain) ? TIMEOUT_LAST_SERVICE : TIMEOUT_START_WAN);
}

void ServiceTypeBrowserPrivate::finished()
{
    m_timer.stop();
    emit m_parent->finished();
}

void ServiceTypeBrowserPrivate::gotNewServiceType(int,int,const QString& type,const QString&,uint)
{
	m_timer.start(TIMEOUT_LAST_SERVICE);
	m_servicetypes+=type;
	emit m_parent->serviceTypeAdded(type);
}



void ServiceTypeBrowserPrivate::gotRemoveServiceType(int,int,const QString& type,const QString&,uint)
{
	m_timer.start(TIMEOUT_LAST_SERVICE);
	m_servicetypes.removeAll(type);
	emit m_parent->serviceTypeRemoved(type);
}


QStringList ServiceTypeBrowser::serviceTypes() const
{
	return d->m_servicetypes;
}

#ifndef KDE_NO_DEPRECATED
bool ServiceTypeBrowser::isRunning() const
{
	return d->m_started;
}
#endif


}
#include "moc_servicetypebrowser.cpp"
#include "moc_avahi-servicetypebrowser_p.cpp"
