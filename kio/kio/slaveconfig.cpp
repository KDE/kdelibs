// -*- c++ -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <assert.h>

#include <qdict.h>

#include <kconfig.h>
#include <kstaticdeleter.h>
#include <kprotocolinfo.h>
#include <kprotocolmanager.h>

#include "slaveconfig.h"

using namespace KIO;

namespace KIO {

class SlaveConfigProtocol
{
public:
  SlaveConfigProtocol() { host.setAutoDelete(true); }
  ~SlaveConfigProtocol() 
  {
     delete configFile;
  }

public:
  MetaData global;
  QDict<MetaData> host;
  KConfig *configFile;
};

static void readConfig(KConfig *config, const QString & group, MetaData *metaData)
{
   *metaData += config->entryMap(group);
}

class SlaveConfigPrivate
{
  public:
     void readGlobalConfig();
     SlaveConfigProtocol *readProtocolConfig(const QString &_protocol);
     SlaveConfigProtocol *findProtocolConfig(const QString &_protocol);
     void readConfigProtocolHost(const QString &_protocol, SlaveConfigProtocol *scp, const QString &host);
  public:
     MetaData global;
     QDict<SlaveConfigProtocol> protocol;
};

void SlaveConfigPrivate::readGlobalConfig()
{
   global.clear();
   // Read stuff...
   KConfig *config = KProtocolManager::config();
   readConfig(KGlobal::config(), "Socks", &global); // Socks settings.
   readConfig(config, "<default>", &global);
}

SlaveConfigProtocol* SlaveConfigPrivate::readProtocolConfig(const QString &_protocol)
{
   SlaveConfigProtocol *scp = protocol.find(_protocol);
   if (!scp)
   {
      QString filename = KProtocolInfo::config(_protocol);
      scp = new SlaveConfigProtocol;
      scp->configFile = new KConfig(filename, true, false);
      protocol.insert(_protocol, scp);
   }
   // Read global stuff...
   readConfig(scp->configFile, "<default>", &(scp->global));
   return scp;
}

SlaveConfigProtocol* SlaveConfigPrivate::findProtocolConfig(const QString &_protocol)
{
   SlaveConfigProtocol *scp = protocol.find(_protocol);
   if (!scp)
      scp = readProtocolConfig(_protocol);
   return scp;
}

void SlaveConfigPrivate::readConfigProtocolHost(const QString &, SlaveConfigProtocol *scp, const QString &host)
{
   MetaData *metaData = new MetaData;
   scp->host.replace(host, metaData);

   // Read stuff
   // Break host into domains
   QString domain = host;

   if (!domain.contains('.'))
   {
      // Host without domain.
      if (scp->configFile->hasGroup("<local>"))
         readConfig(scp->configFile, "<local>", metaData);
   }

   int pos = 0;
   do
   {
      pos = host.findRev('.', pos-1);

      if (pos < 0)
        domain = host;
      else
        domain = host.mid(pos+1);

      if (scp->configFile->hasGroup(domain))
         readConfig(scp->configFile, domain.lower(), metaData);
   }
   while (pos > 0);
}


SlaveConfig *SlaveConfig::_self = 0;
KStaticDeleter<SlaveConfig> slaveconfigsd;

SlaveConfig *SlaveConfig::self()
{
   if (!_self)
      _self = slaveconfigsd.setObject(new SlaveConfig);
   return _self;
}

SlaveConfig::SlaveConfig()
{
  d = new SlaveConfigPrivate;
  d->protocol.setAutoDelete(true);
  d->readGlobalConfig();
}

SlaveConfig::~SlaveConfig()
{
   delete d; d = 0;
   _self = 0; 
}

void SlaveConfig::setConfigData(const QString &protocol, 
                                const QString &host, 
                                const QString &key, 
                                const QString &value )
{
   MetaData config;
   config.insert(key, value);
   setConfigData(protocol, host, config);
}

void SlaveConfig::setConfigData(const QString &protocol, const QString &host, const MetaData &config )
{
   if (protocol.isEmpty())
      d->global += config;
   else {
      SlaveConfigProtocol *scp = d->findProtocolConfig(protocol);
      if (host.isEmpty())
      {
         scp->global += config;
      }
      else
      {
         MetaData *hostConfig = scp->host.find(host);
         if (!hostConfig)
         {
            d->readConfigProtocolHost(protocol, scp, host);
            hostConfig = scp->host.find(host);
            assert(hostConfig);
         }
         *hostConfig += config;
      }
   }
}

MetaData SlaveConfig::configData(const QString &protocol, const QString &host)
{
   MetaData config = d->global;
   SlaveConfigProtocol *scp = d->findProtocolConfig(protocol);
   config += scp->global;
   if (host.isEmpty())
      return config;
   MetaData *hostConfig = scp->host.find(host);
   if (!hostConfig)
   {
      d->readConfigProtocolHost(protocol, scp, host);
      emit configNeeded(protocol, host);
      hostConfig = scp->host.find(host);
      assert(hostConfig);
   }
   config += *hostConfig;
   return config;
}

QString SlaveConfig::configData(const QString &protocol, const QString &host, const QString &key)
{
   return configData(protocol, host)[key];
}

void SlaveConfig::reset()
{
   d->protocol.clear();
   d->readGlobalConfig();
}

}

#include "slaveconfig.moc"
