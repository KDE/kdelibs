/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"

#include <qdatastream.h>
#include <qfile.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <assert.h>

// Read-only constructor
KSycoca::KSycoca()
{
   QString path = KGlobal::dirs()->saveLocation("config") + "ksycoca";
   QFile *database = new QFile(path);
   if (!database->open( IO_ReadOnly ))
   {
     // No database file
     // TODO launch kded here, using KProcess, and upon completion
     // retry again (but not more than once)
     fprintf(stderr, "Error can't open database! Run kded !\n");
     exit(-1);
   }
   str = new QDataStream(database);
   m_lstFactories = new KSycocaFactoryList();
   _self = this;
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ )
{
   QString path = KGlobal::dirs()->saveLocation("config") + "ksycoca";
   QFile *database = new QFile(path);
   if (!database->open( IO_ReadWrite ))
   {
      fprintf(stderr, "Error can't open database!\n");
      exit(-1);
   }
   str = new QDataStream(database);
   m_lstFactories = new KSycocaFactoryList();
   _self = this;
}

KSycoca * KSycoca::self()
{
  if (!_self)
    _self = new KSycoca();
  return _self;
}

KSycoca::~KSycoca()
{
   QIODevice *device = 0;
   if (str)
      device = str->device();
   if (device)
      device->close();
      
   delete str;
   delete device;
   delete m_lstFactories;
}

QDataStream * KSycoca::findEntry(int offset, KSycocaType &type)
{
   //kdebug( KDEBUG_INFO, 7011, QString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16));
   str->device()->at(offset);
   Q_INT32 aType;
   (*str) >> aType;
   type = (KSycocaType) aType;
   //kdebug( KDEBUG_INFO, 7011, QString("KSycoca::found type %1").arg(aType) );
   return str;
}

QDataStream * KSycoca::registerFactory(KSycocaFactoryId id)
{
   str->device()->at(0);
   Q_INT32 aId;
   Q_INT32 aOffset;
   while(true)
   {
      (*str) >> aId;
      kdebug( KDEBUG_INFO, 7011, QString("KSycoca::_registerFactory : found factory %1").arg(aId));
      if (aId == 0)
      {
fprintf(stderr, "KSycoca: Error, KSycocaFactory (id = %d) not found!\n", id);
         break;
      }
      (*str) >> aOffset;
      if (aId == id)
      {
         kdebug( KDEBUG_INFO, 7011, QString("KSycoca::_registerFactory(%1) offset %2").arg((int)id).arg(aOffset));
         str->device()->at(aOffset);
         return str;
      }
   }
   return 0;
}

QDataStream * KSycoca::findOfferList()
{
   str->device()->at(0);
   Q_INT32 aId;
   Q_INT32 aOffset;
   // skip factories offsets
   while(true)
   {
      (*str) >> aId;
      if ( aId )
        (*str) >> aOffset;
      else
        break; // just read 0
   }
   (*str) >> aOffset; // Service type index offset
   kdebug(KDEBUG_INFO, 7011, QString("servicetypeIndexOffset : %1").
          arg(aOffset,8,16));
   str->device()->at( aOffset );
   return str;
}

KSycoca * KSycoca::_self = 0L;

#include "ksycoca.moc"
