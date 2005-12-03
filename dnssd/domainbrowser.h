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

#include <qobject.h>
#include <q3dict.h>
#include <dnssd/remoteservice.h>

// KIPC message ID used by kcm module to signal change in browsing domains list

#define KIPCDomainsChanged 2014

class QStringList;
namespace DNSSD
{
class DomainBrowserPrivate;

/**
@short Class used to provide current list of domains for browsing.
@author Jakub Stachowski
*/
class KDNSSD_EXPORT DomainBrowser : public QObject
{
	Q_OBJECT
public:
	/**
	Standard constructor. It takes all parameters from global configuration.
	All changes in configuration are applied immediately.
	@param parent Parent object.
	 */
	DomainBrowser(QObject *parent=0);
	
	/**
	Constructor that creates browser for domain list. This does not use global
	configuration at all.
	@param domains List of domains 
	@param recursive TRUE - specified domains will be recursively browsed to
	discover more domains. This means that every domain (specified in constructor,
	or found by query) will be queried for list of more browsing domains. 
	@param parent Parent object.
	This process is recursive.
	 */
	DomainBrowser(const QStringList& domains, bool recursive=false, QObject *parent=0);

	~DomainBrowser();
	
	/**
	Current list of domains to browse.
	 */
	const QStringList& domains() const;
	
	/**
	Starts browsing. To stop destroy this object.
	 */
	void startBrowse() ;
	
	/**
	Returns true when browse has already started
	 */
	bool isRunning() const;

signals:
	/**
	Emitted when domain has been removed from browsing list
	 */
	void domainRemoved(const QString&);
	/**
	New domain has been discovered. Also emitted for domain specified in constructor
	and in global configuration
	 */
	void domainAdded(const QString&);

protected:
	virtual void virtual_hook(int,void*);
private:
	DomainBrowserPrivate *d;
private slots:
	void gotNewDomain(DNSSD::RemoteService::Ptr);
	void gotRemoveDomain(DNSSD::RemoteService::Ptr);
	void domainListChanged(int,int);
};

}

#endif
