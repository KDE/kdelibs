/*
 *  This file is part of the KDB libraries
 *  Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */     

#ifndef PGSQL_FACTORY_H
#define PGSQL_FACTORY_H "$Id$"

#include <pluginimpl.h>
#include <klibloader.h>
#include <kinstance.h>


class Factory : public KLibFactory  
{
  
  Q_OBJECT

public: 
  Factory(QObject* parent = 0, const char* name = 0);
	
  ~Factory();
  /**  */
  virtual QObject * create( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

 private:
  static KInstance * s_instance;
};

#endif
