/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#include "pluginimpl.h"
#include "connectorimpl.h"

#include <kdebug.h>
#include <iostream.h>

#include "pluginimpl.moc"

PluginImpl::PluginImpl(QObject *parent)
    : Plugin(parent, "MySQL")
{
    kdDebug(20012) << "PluginImpl::PluginImpl" << endl;
}

PluginImpl::~PluginImpl()
{
    kdDebug(20012) << "PluginImpl::~PluginImpl" << endl;
}

KDB::Plugin::PluginInfo
PluginImpl::info()
{
    kdDebug(20012) << "KDB::Plugin::PluginInfoPluginImpl::info" << endl;
    KDB::Plugin::PluginInfo info;
    info.name = name();
    info.description = "A MySQL plugin for KDE-DB";
    info.version = "2.0pre";
    info.author = "Alessandro Praduroux";
    info.e_mail = "pradu@thekompany.com";
    info.copyright = "LGPL";

    return info;
}

KDB::Connector *
PluginImpl::createConnector()
{
    kdDebug(20012) << "PluginImpl::createConnector" << endl;
    return new ConnectorImpl;
}

bool
PluginImpl::provides(KDB::capability /*cap*/)
{
    kdDebug(20012) << "PluginImpl::provides" << endl;
    return false;
}

KDB::Capability *
PluginImpl::createObject(KDB::capability /* cap */)
{
    kdDebug(20012) << "PluginImpl::createObject" << endl;
    pushError( new KDB::UnsupportedCapability("this plugin does nothing!"));
    return 0L;
}




