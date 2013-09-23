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

#ifndef DNSSDDOMAINBROWSER_H
#define DNSSDDOMAINBROWSER_H

#include <QtCore/QObject>
#include <dnssd/remoteservice.h>

class QStringList;
namespace DNSSD
{
class DomainBrowserPrivate;

/**
 * @class DomainBrowser domainbrowser.h DNSSD/DomainBrowser
 * @short Browses recommended domains for browsing or publishing to.
 *
 * Usage of this class is very simple.  If you are interested in
 * browsing for services, simple do
 * @code
 * DNSSD::DomainBrowser *browser =
 *     new DNSSD::DomainBrowser(DNSSD::DomainBrowser::Browsing, this);
 * connect(browser, SIGNAL(domainAdded(QString)),
 *         this, SLOT(browsingDomainAdded(QString));
 * connect(browser, SIGNAL(domainRemoved(QString)),
 *         this, SLOT(browsingDomainRemove(QString));
 * browser->startBrowse();
 * @endcode
 *
 * If you are interested in domains where you can register services,
 * usage is identical except that you should pass
 * <tt>DNSSD::DomainBrowser::Registering</tt> to the constructor.
 *
 * @author Jakub Stachowski
 */
class KDNSSD_EXPORT DomainBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	 * A type of domain recommendation
	 */
	enum DomainType
	{
		/** Domains recommended for browsing for services on (using ServiceBrowser) */
		Browsing,
		/** Domains recommended for publishing to (using PublicService) */
		Publishing
	};
	/**
	 * Standard constructor
	 *
	 * The global DNS-SD configuration (for example, the global Avahi
	 * configuration for the Avahi backend) will be used.
	 *
	 * @param type   the type of domain to search for
	 * @param parent parent object (see QObject documentation)
	 *
	 * @see startBrowse() and ServiceBrowser::isAvailable()
	 */
	explicit DomainBrowser(DomainType type, QObject* parent = 0);

	~DomainBrowser();

	/**
	 * The current known list of domains of the requested DomainType
	 *
	 * @return a list of currently known domain names
	 */
	QStringList domains() const;

	/**
	 * Starts browsing
	 *
	 * Only the first call to this function will have any effect.
	 *
	 * Browsing stops when the DomainBrowser object is destroyed.
	 *
	 * @warning The domainAdded() signal may be emitted before this
	 *          function returns.
	 *
	 * @see domainAdded() and domainRemoved()
	 */
	void startBrowse();

	/**
	 * Whether the browsing has been started
	 *
	 * @return @c true if startBrowse() has been called, @c false otherwise
	 */
	bool isRunning() const;

Q_SIGNALS:
	/**
	 * A domain has disappeared from the browsed list
	 *
	 * Emitted when domain has been removed from browsing list
	 * or the publishing list (depending on which list was
	 * requested in the constructor).
	 *
	 * @param domain the name of the domain
	 *
	 * @see domainAdded()
	 */
	void domainRemoved(const QString& domain);

	/**
	 * A new domain has been discovered
	 *
	 * If the requested DomainType is Browsing, this will
	 * also be emitted for the domains specified in the
	 * global configuration.
	 *
	 * @param domain the name of the domain
	 *
	 * @see domainRemoved()
	 */
	void domainAdded(const QString& domain);

private:
	friend class DomainBrowserPrivate;
	DomainBrowserPrivate* const d;
};

}

#endif
