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
#include <factory.h>
#include <kdebug.h>

#include <factory.moc>

KInstance * Factory::s_instance = 0L;

Factory::Factory(QObject* parent, const char* name)
    : KLibFactory(parent, name)
{
    kdDebug(20012) << "Factory::Factory" << endl;
    s_instance = new KInstance("ksql_mysql");
}

Factory::~Factory()
{
    kdDebug(20012) << "Factory::~Factory" << endl;
}

/**  */
QObject * Factory::create(QObject* parent , const char* , const char*,  const QStringList &)
{
    kdDebug(20012) << "Factory::Create" << " parent=" << parent <<  endl;
    PluginImpl * p = new PluginImpl(parent);
    emit objectCreated(p);
    return p;
}

extern "C"
{
    void* init_libkdb_mysql()
    {
        return new Factory();
    }
};


