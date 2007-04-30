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

#include "mdnsd-domainbrowser_p.h"
#include "domainbrowser.h"
#include "settings.h"
#include "remoteservice.h"
#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtDBus/QtDBus>

namespace DNSSD
{

void domain_callback(DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode, const char *replyDomain, void *context);

DomainBrowser::DomainBrowser(DomainType type, QObject *parent) : QObject(parent),d(new DomainBrowserPrivate(type,this))
{
	d->m_domains = Configuration::domainList();

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
	QStringList::const_iterator itEnd = d->m_domains.end();
	for (QStringList::const_iterator it=d->m_domains.begin(); it!=itEnd; ++it ) emit domainAdded(*it);
	if (d->isRunning()) return;
	DNSServiceRef ref;
	if (DNSServiceEnumerateDomains(&ref,(d->m_type==Browsing) ? kDNSServiceFlagsBrowseDomains:kDNSServiceFlagsBrowseDomains,
	0, domain_callback,reinterpret_cast<void*>(d)) == kDNSServiceErr_NoError) d->setRef(ref);
}

void DomainBrowserPrivate::customEvent(QEvent* event)
{
	if (event->type()==QEvent::User+SD_ERROR) stop();
	if (event->type()==QEvent::User+SD_ADDREMOVE) {
		AddRemoveEvent *aev = static_cast<AddRemoveEvent*>(event);
		if (aev->m_op==AddRemoveEvent::Add) {
		    //FIXME: check if domain name is not name+domain (there was some mdnsd weirdness)
	    	    if (m_domains.contains(aev->m_domain)) return;
		    m_domains.append(aev->m_domain);
		    emit m_parent->domainAdded(aev->m_domain);
		}
		else {
			m_domains.removeAll(aev->m_domain);
			emit m_parent->domainRemoved(aev->m_domain);
		}
	}
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
	// this will emit domainAdded() for every domain if necessary
	if (was_running) m_parent->startBrowse();
}

QStringList DomainBrowser::domains() const
{
	return d->m_domains;
}

bool DomainBrowser::isRunning() const
{
	return d->isRunning();
}

void domain_callback(DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode, const char *replyDomain, void *context)
{
	QObject *obj = reinterpret_cast<QObject*>(context);
	if (errorCode != kDNSServiceErr_NoError) {
		ErrorEvent err;
		QCoreApplication::sendEvent(obj, &err);
	} else {
		// domain browser is supposed to return only _additional_ domains
		if (flags&kDNSServiceFlagsDefault) return;
		AddRemoveEvent arev((flags & kDNSServiceFlagsAdd) ? AddRemoveEvent::Add :
			AddRemoveEvent::Remove, QString(), QString(),
			DNSToDomain(replyDomain), !(flags & kDNSServiceFlagsMoreComing));
		QCoreApplication::sendEvent(obj, &arev);
	}
}

}



#include "domainbrowser.moc"
#include "mdnsd-domainbrowser_p.moc"
