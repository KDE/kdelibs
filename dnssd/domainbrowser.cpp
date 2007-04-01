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

#include <QtCore/QStringList>
#include "domainbrowser.h"
#include "domainbrowser_p.h"
#include "settings.h"
#include "remoteservice.h"
#include "query.h"
#include <QHash>
#include <QtDBus/QtDBus>

namespace DNSSD
{

DomainBrowser::DomainBrowser(QObject *parent) : QObject(parent),d(new DomainBrowserPrivate(this))
{
	d->m_running = false;
	d->m_domains = Configuration::domainList();
	if (Configuration::browseLocal()) d->m_domains+="local.";

        // Those same names have to be used in the kcontrol module too.
        const QString dbusPath = "/libdnssd";
        const QString dbusInterface = "org.kde.DNSSD.DomainBrowser";
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.connect( QString(), dbusPath, dbusInterface, "domainListChanged", this, SLOT(domainListChanged()) );
}

DomainBrowser::~DomainBrowser()
{
	delete d;
}


void DomainBrowser::startBrowse()
{
	if (d->m_running) return;
	d->m_running=true;
	QStringList::const_iterator itEnd = d->m_domains.end();
	for (QStringList::const_iterator it=d->m_domains.begin(); it!=itEnd; ++it ) emit domainAdded(*it);
}

void DomainBrowserPrivate::gotNewDomain(DNSSD::RemoteService::Ptr srv)
{
	QString domain = srv->serviceName()+'.'+srv->domain();
	if (m_domains.contains(domain)) return;
	m_domains.append(domain);
	emit m_parent->domainAdded(domain);
}

void DomainBrowserPrivate::gotRemoveDomain(DNSSD::RemoteService::Ptr srv)
{
	QString domain = srv->serviceName()+'.'+srv->domain();
	m_domains.removeAll(domain);
	emit m_parent->domainRemoved(domain);
}

void DomainBrowserPrivate::domainListChanged()
{
	bool was_running = m_running;
	m_running = false;
	if (was_running) {
		QStringList::const_iterator itEnd = m_domains.end();
		for (QStringList::const_iterator it=m_domains.begin(); it!=itEnd; ++it )
			emit m_parent->domainRemoved(*it);
	}
	m_domains.clear();
	// now reread configuration and add domains
	Configuration::self()->readConfig();
	m_domains = Configuration::domainList();
	if (Configuration::browseLocal()) m_domains+="local.";
	// this will emit domainAdded() for every domain if necessary
	if (was_running) m_parent->startBrowse();
}

QStringList DomainBrowser::domains() const
{
	return d->m_domains;
}

bool DomainBrowser::isRunning() const
{
	return d->m_running;
}

}
#include "domainbrowser.moc"
#include "domainbrowser_p.moc"
