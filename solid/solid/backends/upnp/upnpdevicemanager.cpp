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

#include "upnpdevicemanager.h"
#include "upnpdevice.h"

#include <QtCore/QtDebug>
#include <QtCore/QList>

#include <HDiscoveryType>
#include <HDeviceInfo>
#include <HDeviceProxy>
#include <HUdn>

namespace Solid
{
namespace Backends
{
namespace UPnP
{
 
const char* uPnPUdiPrefix = "/org/kde/upnp";

typedef QList<Herqq::Upnp::HDevice*> HDeviceList;

UPnPDeviceManager::UPnPDeviceManager(QObject* parent)
  : Solid::Ifaces::DeviceManager(parent),
    mUdiPrefix( QString::fromLatin1(uPnPUdiPrefix) ),    
    mSupportedInterfaces(),
    mControlPoint(new Herqq::Upnp::HControlPoint(this))
{       
  connect(
       mControlPoint,
       SIGNAL(rootDeviceOnline(Herqq::Upnp::HDeviceProxy*)),
       this,
       SLOT(rootDeviceOnline(Herqq::Upnp::HDeviceProxy*)));

   connect(
        mControlPoint,
        SIGNAL(rootDeviceOffline(Herqq::Upnp::HDeviceProxy*)),
        this,
        SLOT(rootDeviceOffline(Herqq::Upnp::HDeviceProxy*)));

   if (!mControlPoint->init())
   {
      qDebug() << "control point init error:" << mControlPoint->errorDescription();
      return;
   }
   
   //mSupportedInterfaces << Solid::DeviceInterface::StorageAccess;
}

UPnPDeviceManager::~UPnPDeviceManager()
{
  //TODO what should I do here? erase all devices? qDeleteAll()?
}

QString UPnPDeviceManager::udiPrefix() const
{
  return mUdiPrefix;
}

QSet< DeviceInterface::Type > UPnPDeviceManager::supportedInterfaces() const
{
  return mSupportedInterfaces;
}

QStringList UPnPDeviceManager::allDevices()
{
  QStringList result;
  
  result << mUdiPrefix;
  
  Herqq::Upnp::HDiscoveryType discoveryType = Herqq::Upnp::HDiscoveryType::createDiscoveryTypeForRootDevices();
  if (mControlPoint->scan(discoveryType))
  {
    Herqq::Upnp::HDeviceProxies list = mControlPoint->rootDevices(); 
    qDebug() << "empty list:" << list.isEmpty();    
    for (int i = 0; i < list.size(); ++i)
    {
      Herqq::Upnp::HDeviceProxy* device = list[i];
      Herqq::Upnp::HDeviceInfo info = device->deviceInfo();
      
      result << ( udiPrefix() + '/' + info.udn().toString() );
      qDebug() << "Found device:" << ( udiPrefix() + '/' + info.udn().toString() );
      //TODO listing only root devices
    }
  } 
  else
  {    
    qDebug() << "scan error:" << mControlPoint->errorDescription();
  }
  
  return result;
}

QStringList UPnPDeviceManager::devicesFromQuery(const QString& parentUdi, DeviceInterface::Type type)
{
  Q_UNUSED(parentUdi)
  Q_UNUSED(type)
  return QStringList(); //FIXME implement it!
}

QObject* UPnPDeviceManager::createDevice(const QString& udi)
{
  QString udnFromUdi = udi.mid(udiPrefix().length());
  Herqq::Upnp::HUdn udn(udnFromUdi);
  if (udn.isValid())
  {
    Herqq::Upnp::HDeviceProxy* device = mControlPoint->device(udn);
    if (device)
    {
      return new Solid::Backends::UPnP::UPnPDevice(device);
    }
  }
  
  return 0;
}

void UPnPDeviceManager::rootDeviceOnline(Herqq::Upnp::HDeviceProxy* device)
{
  QString udn = device->deviceInfo().udn().toString();
  
  emit deviceAdded(udiPrefix() + '/' + udn);
}

void UPnPDeviceManager::rootDeviceOffline(Herqq::Upnp::HDeviceProxy* device)
{
  QString udn = device->deviceInfo().udn().toString();
  
  emit deviceRemoved(udiPrefix() + '/' + udn);
  
  mControlPoint->removeRootDevice(device);
}

}
}
}