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


#include "avahi-domainbrowser_p.h"
#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QStandardPaths>
#include <avahi-common/defs.h>
#include "avahi_server_interface.h"
#include "domainbrowser.h"
#include "avahi_domainbrowser_interface.h"



namespace KDNSSD
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
	connect(b,SIGNAL(ItemNew(int,int,QString,uint)),d, SLOT(gotNewDomain(int,int,QString,uint)));
	connect(b,SIGNAL(ItemRemove(int,int,QString,uint)),d, SLOT(gotRemoveDomain(int,int,QString,uint)));
	d->m_browser=b;
	if (d->m_type==Browsing) {
    	    QString domains_evar=qgetenv("AVAHI_BROWSE_DOMAINS");
	    if (!domains_evar.isEmpty()) {
		QStringList edomains=domains_evar.split(':');
		Q_FOREACH(const QString &s, edomains) d->gotNewDomain(-1,-1,s,0);
	    }
	    //FIXME: watch this file and restart browser if it changes
	    QString confDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	    QFile domains_cfg(confDir + "/avahi/browse-domains");
	    if (domains_cfg.open(QIODevice::ReadOnly | QIODevice::Text))
		while (!domains_cfg.atEnd())
                    d->gotNewDomain(-1,-1,QString::fromUtf8(domains_cfg.readLine().data()).trimmed(),0);

	}

}

void DomainBrowserPrivate::gotNewDomain(int,int,const QString& domain,uint)
{
	QString decoded=DNSToDomain(domain);
	if (m_domains.contains(decoded)) return;
	m_domains+=decoded;
	emit m_parent->domainAdded(decoded);
}

void DomainBrowserPrivate::gotRemoveDomain(int,int,const QString& domain,uint)
{
	QString decoded=DNSToDomain(domain);
	if (!m_domains.contains(decoded)) return;
	emit m_parent->domainRemoved(decoded);
	m_domains.remove(decoded);
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
#include "moc_domainbrowser.cpp"
#include "moc_avahi-domainbrowser_p.cpp"
