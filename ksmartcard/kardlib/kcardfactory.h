/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat < >
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef _KCARDFACTORY_H
#define _KCARDFACTORY_H


#include <kservice.h>
#include <qmap.h>

class KCardReader;
class KCardImplementation;
class QStringList;


// FIXME: Change this to use the strings instead.  Makes it easier to extend
// later.


//typedef  enum {KCardGSMType, KCardProcessorType} KCardType;


class KCardFactory {

 public:

  KCardImplementation * getCard ( const QString & slot,
				  const QString & cardATR);
  KCardImplementation * getCard ( const QString & slot );
  static KCardFactory *self();
  QStringList & getImplentationList() const ;
 
 protected:

  virtual ~KCardFactory();
 private:

  KCardFactory();
  
  int loadModules();
  void *loadModule(KService::Ptr svc);

  static KCardFactory *_self;
 
  
 typedef QMap< QString, QMap< QString, QMap< QString, KService::Ptr > > > _modulesMap;
  
 _modulesMap _modules;
 
 static QStringList _implementationList ;
 
 
 class KCardFactoryPrivate;
 KCardFactoryPrivate *d;

};

#endif
