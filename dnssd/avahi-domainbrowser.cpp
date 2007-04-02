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


#include <QtCore/QSet>
#include "avahi_server_interface.h"
#include "domainbrowser.h"
#include "avahi_domainbrowser_interface.h"
#include "avahi-domainbrowser_p.h"

#define UNSPEC -1

typedef enum {
    AVAHI_DOMAIN_BROWSER_BROWSE,            /**< Browse for a list of available browsing domains */
    AVAHI_DOMAIN_BROWSER_BROWSE_DEFAULT,    /**< Browse for the default browsing domain */
    AVAHI_DOMAIN_BROWSER_REGISTER,          /**< Browse for a list of available registering domains */
    AVAHI_DOMAIN_BROWSER_REGISTER_DEFAULT,  /**< Browse for the default registering domain */
    AVAHI_DOMAIN_BROWSER_BROWSE_LEGACY,     /**< Legacy browse domain - see DNS-SD spec for more information */
    AVAHI_DOMAIN_BROWSER_MAX
} AvahiDomainBrowserType;

Q_DECLARE_METATYPE(QList<QByteArray>);

namespace DNSSD
{

DomainBrowser::DomainBrowser(DomainType type, QObject *parent) : QObject(parent), d(new DomainBrowserPrivate(type,this))
{}

DomainBrowser::~DomainBrowser()
{
    delete d;
}

void DomainBrowser::startBrowse()
{
	if (d->m_started) return;
	d->m_started=true;
	org::freedesktop::Avahi::Server s("org.freedesktop.Avahi","/",QDBusConnection::systemBus());
	QDBusReply<QDBusObjectPath> rep=s.DomainBrowserNew(-1, -1, "", (d->m_type==Browsing) ? 
	    AVAHI_DOMAIN_BROWSER_BROWSE : AVAHI_DOMAIN_BROWSER_REGISTER,0);
	
	if (!rep.isValid()) return;
	org::freedesktop::Avahi::DomainBrowser *b=new org::freedesktop::Avahi::DomainBrowser("org.freedesktop.Avahi",rep.value().path(),
	    QDBusConnection::systemBus());
	connect(b,SIGNAL(ItemNew(int,int,const QString&,uint)),d, SLOT(gotNewDomain(int,int,const QString&, uint)));
	connect(b,SIGNAL(ItemRemove(int,int,const QString&,uint)),d, SLOT(gotRemoveDomain(int,int,const QString&, uint)));
	d->m_browser=b;
}

void DomainBrowserPrivate::gotNewDomain(int,int,const QString& domain,uint)
{
	if (m_domains.contains(domain)) return;
	emit m_parent->domainAdded(domain);
}

void DomainBrowserPrivate::gotRemoveDomain(int,int,const QString& domain,uint)
{
	if (!m_domains.contains(domain)) return;
	m_domains.remove(domain);
	emit m_parent->domainRemoved(domain);
}


QStringList DomainBrowser::domains() const
{
	return d->m_domains.values();
}

bool DomainBrowser::isRunning() const
{
	return d->m_started;
}


}
#include "domainbrowser.moc"
#include "avahi-domainbrowser_p.moc"
