/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "winstorageaccess.h"


using namespace Solid::Backends::Win;

WinStorageAccess::WinStorageAccess(WinDevice *device) :
    WinInterface(device)
{
}

WinStorageAccess::~WinStorageAccess()
{
}

bool WinStorageAccess::isAccessible() const
{
    return true;
}

QString WinStorageAccess::filePath() const
{
    return QString("Not implemented");
}

bool WinStorageAccess::isIgnored() const
{
    return false;
}

bool WinStorageAccess::setup()
{
    return true;
}

bool WinStorageAccess::teardown()
{
    return true;
}

void WinStorageAccess::accessibilityChanged(bool accessible, const QString &udi)
{
}

void WinStorageAccess::setupDone(Solid::ErrorType error, QVariant resultData, const QString &udi)
{
}

void WinStorageAccess::teardownDone(Solid::ErrorType error, QVariant resultData, const QString &udi)
{
}

void WinStorageAccess::setupRequested(const QString &udi)
{
}

void WinStorageAccess::teardownRequested(const QString &udi)
{
}

#include "winstorageaccess.moc"
