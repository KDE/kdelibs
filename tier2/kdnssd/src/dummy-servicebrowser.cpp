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
#include "servicebrowser.h"
#include <QtCore/QHash>

namespace KDNSSD
{

ServiceBrowser::ServiceBrowser(const QString&,bool,const QString&, const QString&) : d(0)
{}

bool ServiceBrowser::isAutoResolving() const
{
    return false;
}


ServiceBrowser::State ServiceBrowser::isAvailable()
{
	return Unsupported;
}
ServiceBrowser::~ ServiceBrowser()
{
}

void ServiceBrowser::startBrowse()
{
    emit finished();
}

QList<RemoteService::Ptr> ServiceBrowser::services() const
{
	return QList<RemoteService::Ptr>();
}

void ServiceBrowser::virtual_hook(int, void*)
{}

QHostAddress ServiceBrowser::resolveHostName(const QString &hostname)
{
	Q_UNUSED(hostname);
	return QHostAddress();
}

QString ServiceBrowser::getLocalHostName()
{
	return QString();
}

}

#include "moc_servicebrowser.cpp"
