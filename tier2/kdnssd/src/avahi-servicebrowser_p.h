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

#ifndef AVAHI_SERVICEBROWSER_P_H
#define AVAHI_SERVICEBROWSER_P_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include "servicebrowser.h"
#include "avahi_servicebrowser_interface.h"

namespace KDNSSD
{

class ServiceBrowserPrivate : public QObject
{
Q_OBJECT
public:
	ServiceBrowserPrivate(ServiceBrowser* parent) : QObject(), m_running(false), m_browser(0), m_parent(parent)
	{}
        ~ServiceBrowserPrivate() {  if (m_browser) m_browser->Free(); delete m_browser;}
	QList<RemoteService::Ptr> m_services;
	QList<RemoteService::Ptr> m_duringResolve;
	QString m_type;
	QString m_domain;
	QString m_subtype;
	bool m_autoResolve;
	bool m_running;
	bool m_finished;
	bool m_browserFinished;
	QTimer m_timer;
	org::freedesktop::Avahi::ServiceBrowser* m_browser;
	ServiceBrowser* m_parent;
	
	// get already found service identical to s or null if not found
	RemoteService::Ptr find(RemoteService::Ptr s, const QList<RemoteService::Ptr>& where) const;

private Q_SLOTS:
	void browserFinished();
	void queryFinished();
	void serviceResolved(bool success);
	void gotNewService(int,int,const QString&,const QString&,const QString&, uint);
	void gotRemoveService(int,int,const QString&,const QString&,const QString&, uint);
};

}

#endif
