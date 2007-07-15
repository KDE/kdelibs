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

#ifndef MDNSD_SERVICETYPEBROWSER_P_H
#define MDNSD_SERVICETYPEBROWSER_P_H

#include <QtCore/QStringList>

#include "servicebrowser.h"
#include "servicetypebrowser.h"

namespace DNSSD
{

class ServiceTypeBrowserPrivate : public QObject
{
Q_OBJECT
public:
	ServiceTypeBrowserPrivate(ServiceTypeBrowser* parent):  m_parent(parent) {}
	ServiceTypeBrowser* m_parent;
	ServiceBrowser* m_browser;
	QStringList m_servicetypes;
public Q_SLOTS:
    void newService(DNSSD::RemoteService::Ptr);
    void removeService(DNSSD::RemoteService::Ptr);
};

}
#endif
