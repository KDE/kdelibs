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
     fprintf(stderr, "Error can't open database!\n");
     exit(-1);
   }
   str = new QDataStream(database);
   m_lstFactories = new KSycocaFactoryList();
   self = this;
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
   self = this;
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

//static
QDataStream *
KSycoca::findEntry(int offset, KSycocaType &type)
{
   if (!self)
      self = new KSycoca();
   return self->_findEntry(offset, type);
}

QDataStream *
KSycoca::_findEntry(int offset, KSycocaType &type)
{
   kdebug( KDEBUG_INFO, 7011, QString("KSycoca::_findEntry(%1,%2)").arg(offset).arg((int)type));
   str->device()->at(offset);
   Q_INT32 aType;
   (*str) >> aType;
   type = (KSycocaType) aType;
   return str;
}

//static
QDataStream *
KSycoca::registerFactory( KSycocaFactoryId id)
{
   if (!self)
      self = new KSycoca();
   return self->_registerFactory( id );
}

QDataStream *
KSycoca::_registerFactory( KSycocaFactoryId id)
{
   str->device()->at(0);
   Q_INT32 aId;
   Q_INT32 aOffset;
   while(true)
   {
      (*str) >> aId;
      kdebug( KDEBUG_INFO, 7011, QString("KSycoca::_registerFactory : found factory %1").arg(aId));
      if (aId == KST_KSycocaFactory)  // ????? shouldn't that be 0 ??? (David)
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

//static
bool KSycoca::isBuilding()
{
  if (!self)
    self = new KSycoca();
  return self->_isBuilding();
}

KSycoca * KSycoca::self = 0;

#include "ksycoca.moc"
