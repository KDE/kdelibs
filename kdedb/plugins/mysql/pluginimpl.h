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
#ifndef __PLUGINIMPL_H
#define __PLUGINIMPL_H

#include <kdb/plugin.h>

class KDB::Connector;

class PluginImpl : public KDB::Plugin
{
    Q_OBJECT

public:

    PluginImpl(QObject *parent);
    ~PluginImpl();

    /**
     * returns the information about the plugin
     */
    virtual KDB::Plugin::PluginInfo info();

    virtual bool provides(KDB::capability cap);

    virtual KDB::Capability *createObject(KDB::capability cap);

protected:

    virtual KDB::Connector *createConnector();


};


#endif





