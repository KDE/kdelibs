/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#include "upnpstorageaccess.h"

namespace Solid
{
namespace Backends
{
namespace UPnP
{
  
UPnPStorageAccess::UPnPStorageAccess(UPnPDevice* device) : 
   UPnPDeviceInterface(device)
{
}

UPnPStorageAccess::~UPnPStorageAccess()
{
}

bool UPnPStorageAccess::isAccessible() const
{
  return mDevice->isValid();
}

QString UPnPStorageAccess::filePath() const
{
  return mDevice->location();
}

//TODO: since the device is on the network, we cannot mount/unmount it from here.
// could these two methods (and the signals) remain pure virtual?

bool UPnPStorageAccess::setup()
{
  return this->isAccessible(); 
}

bool UPnPStorageAccess::teardown()
{
  return true;
}

}
}
}