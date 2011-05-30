/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef ACCESSMANAGER_P_H
#define ACCESSMANAGER_P_H

#include <dnssd/remoteservice.h>

#include "../remote/accessmanager.h"

class KJob;

namespace DNSSD
{
    class ServiceBrowser;
}

namespace Plasma
{

class AccessManagerPrivate
{
    public:
        AccessManagerPrivate(AccessManager *manager);
        ~AccessManagerPrivate();
        void slotJobFinished(KJob *job);
        void slotAddService(DNSSD::RemoteService::Ptr service);
        void slotRemoveService(DNSSD::RemoteService::Ptr service);

        AccessManager *q;
        DNSSD::ServiceBrowser *browser;
        QMap<QString, RemoteObjectDescription> services;
        QMap<QString, DNSSD::RemoteService::Ptr> zeroconfServices;
};

}

#endif
