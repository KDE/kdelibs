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

#ifndef DNSSDREMOTESERVICE_P_H
#define DNSSDREMOTESERVICE_P_H

#include <QtCore/QObject>

namespace DNSSD
{

class RemoteServicePrivate : public QObject
{
Q_OBJECT
public:
	RemoteServicePrivate(RemoteService* parent) : QObject(), m_resolved(false), m_running(false), m_resolver(0), m_parent(parent)
	{}
        ~RemoteServicePrivate() {  if (m_resolver) m_resolver->Free(); }
	bool m_resolved;
	bool m_running;
	org::freedesktop::Avahi::ServiceResolver* m_resolver;
	RemoteService* m_parent;
	void stop();

private Q_SLOTS:
        void gotFound(int, int, const QString &name, const QString &type, const QString &domain, const QString &host, int aprotocol, const QString &address, ushort port, const QByteArray &txt, uint flags);
	void gotError();
};

}

#endif
