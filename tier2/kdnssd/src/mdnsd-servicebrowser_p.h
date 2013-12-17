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

#ifndef MDNSD_SERVICEBROWSER_P_H
#define MDNSD_SERVICEBROWSER_P_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "mdnsd-responder.h"
#include "servicebrowser.h"

namespace KDNSSD
{

class ServiceBrowserPrivate : public Responder
{
Q_OBJECT
public:
	ServiceBrowserPrivate(ServiceBrowser* parent) : Responder(),  m_parent(parent)
	{}
	QList<RemoteService::Ptr> m_services;
	QList<RemoteService::Ptr> m_duringResolve;
	QString m_type;
	QString m_domain;
	QString m_subtype;
	bool m_autoResolve;
	bool m_finished;
	ServiceBrowser* m_parent;
	QTimer timeout;

	// get already found service identical to s or null if not found
	RemoteService::Ptr find(RemoteService::Ptr s, const QList<RemoteService::Ptr>& where) const;
	virtual void customEvent(QEvent* event);
public Q_SLOTS:
	void queryFinished();
	void serviceResolved(bool success);
	void onTimeout();
};

}

#endif
