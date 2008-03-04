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
\class DomainBrowser domainbrowser.h DNSSD/DomainBrowser

@short Class used to provide  list of additional domains for browsing or publishing.
@author Jakub Stachowski
*/
class KDNSSD_EXPORT DomainBrowser : public QObject
{
	Q_OBJECT
public:
	enum DomainType { Browsing, Publishing };
	/**
	Standard constructor. It takes all parameters from global configuration.
	All changes in configuration are applied immediately.
	@param parent Parent object.
	 */
	explicit DomainBrowser(DomainType type, QObject *parent=0);

	~DomainBrowser();

	/**
	Current list of domains to browse.
	 */
	QStringList domains() const;

	/**
	Starts browsing. To stop destroy this object. domainAdded signals may be emitted before this function returns.
	 */
	void startBrowse() ;

	/**
	Returns true when browse has already started
	 */
	bool isRunning() const;

Q_SIGNALS:
	/**
	Emitted when domain has been removed from browsing list
	 */
	void domainRemoved(const QString&);
	/**
	New domain has been discovered. Also emitted for domain specified in constructor
	and in global configuration
	 */
	void domainAdded(const QString&);

private:
	friend class DomainBrowserPrivate;
	DomainBrowserPrivate* const d;
};

}

#endif
