/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Jakub Stachowski <qbast@go2.pl>
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

#ifndef DNSSDSERVICESACTION_H
#define DNSSDSERVICESACTION_H

#include <QtGui/QAction>
#include <dnssd/dnssd_export.h>
#include <dnssd/remoteservice.h>

namespace DNSSD
{

class ServicesActionPrivate;
class ServiceBrowser;


/**
\class ServicesAction servicesaction.h DNSSD/ServicesAction
ServicesAction class provides simple user interface for service browser. 

It provides submenu containing all services found by ServiceBrowser passed in constructor (updated in real time). 
The class inherits QAction so you can add it to tool button or menu. If currently there are no services, action becomes disabled.

Following example creates tool button that shows list of Zeroconf-enabled HTTP servers on local network. Selecting
one of them will call resolveAndConnect() slot:

\code
QToolButton* button=new QToolButton(this);
ServicesAction* action=new ServicesAction(new ServicesBrowser("_http._tcp"), KIcon("network-workgroup"),
    i18n("Servers"),this);
button->setDefaultAction(action);
button->setPopupMode(QToolButton::InstantPopup);
connect(action,SIGNAL(selected(DNSSD::RemoteService::Ptr)), this, 
    SLOT(resolveAndConnect(DNSSD::RemoteService::Ptr)));
\endcode

\since 4.1
@author Jakub Stachowski
@short Action with menu containing found services
*/
class KDNSSD_EXPORT ServicesAction : public QAction
{
Q_OBJECT

public:
	/**
	Creates action for given service browses and starts browsing for services. The action becomes parent of the
	browser so there is no need to delete it afterwards.
	
	@param browser ServiceBrowser object that should be used for searching for services.
	*/ 
	ServicesAction(ServiceBrowser* browser, QObject* parent=0);
	ServicesAction(ServiceBrowser* browser, const QString& text, QObject* parent=0);
	ServicesAction(ServiceBrowser* browser, const QIcon& icon, const QString& text, QObject* parent=0);
	virtual ~ServicesAction();

Q_SIGNALS:
	/**
	Emitted when user selects an action from menu.
	*/
	void selected(DNSSD::RemoteService::Ptr);

protected:
	/**
	Creates action to represent a service. Default implementation sets action text to service name.
	Reimplement this function if you want to change presentation of services in the menu. Do not 
	use data(), because it will be overwritten. 
	*/
	virtual QAction* makeAction(DNSSD::RemoteService::Ptr srv);

private:
	ServicesActionPrivate* const d;
	friend class ServicesActionPrivate;

};

}

#endif
