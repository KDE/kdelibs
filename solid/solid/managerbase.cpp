/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "managerbase_p.h"

#include <stdlib.h>

#include "backends/fakehw/fakemanager.h"

#if defined (Q_OS_MAC)
#include "backends/iokit/iokitmanager.h"
#elif defined (Q_OS_UNIX)
#include "backends/hal/halmanager.h"
#elif defined (_MSC_VER) // TODO: mingw
#include "backends/wmi/wmimanager.h"
#endif

Solid::ManagerBasePrivate::ManagerBasePrivate()
    : m_backend(0)
{
}

Solid::ManagerBasePrivate::~ManagerBasePrivate()
{
    delete m_backend;
}

void Solid::ManagerBasePrivate::loadBackend()
{
    QString solidFakeXml(QString::fromLocal8Bit(qgetenv("SOLID_FAKEHW")));

    if (!solidFakeXml.isEmpty()) {
        m_backend = new Solid::Backends::Fake::FakeManager(0, solidFakeXml);
    } else {
#        if defined (Q_OS_MAC)
            m_backend = new Solid::Backends::IOKit::IOKitManager(0);
#        elif defined (Q_OS_UNIX)
            m_backend = new Solid::Backends::Hal::HalManager(0);
#        elif defined (_MSC_VER) // TODO: mingw
            m_backend = new Solid::Backends::Wmi::WmiManager(0);
#        endif
    }
}

QString Solid::ManagerBasePrivate::errorText() const
{
    return m_errorText;
}

QObject *Solid::ManagerBasePrivate::managerBackend() const
{
    return m_backend;
}


