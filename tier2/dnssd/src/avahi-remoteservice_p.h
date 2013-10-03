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

#ifndef AVAHI_REMOTESERVICE_P_H
#define AVAHI_REMOTESERVICE_P_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include "servicebase_p.h"
#include "remoteservice.h"
#include "avahi_serviceresolver_interface.h"


#define K_D RemoteServicePrivate* d=static_cast<RemoteServicePrivate*>(this->d)

namespace DNSSD
{

class RemoteServicePrivate : public QObject, public ServiceBasePrivate
{
Q_OBJECT
public:
	RemoteServicePrivate(RemoteService* parent, const QString& name, const QString& type, const QString& domain) : QObject(), 
	ServiceBasePrivate(name, type, domain, QString(), 0), m_resolved(false), m_running(false), m_resolver(0), m_parent(parent)
	{}
        ~RemoteServicePrivate() {  if (m_resolver) m_resolver->Free(); delete m_resolver; }
	bool m_resolved;
	bool m_running;
	org::freedesktop::Avahi::ServiceResolver* m_resolver;
	RemoteService* m_parent;
	void stop();

private Q_SLOTS:
        void gotFound(int, int, const QString &name, const QString &type, const QString &domain, const QString &host, int aprotocol, const QString &address, ushort port, const QList<QByteArray> &txt, uint flags);
	void gotError();
};

}

#endif
