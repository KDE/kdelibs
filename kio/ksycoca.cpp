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

#include <kapp.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <assert.h>

// Read-only constructor
KSycoca::KSycoca() : DCOPObject("ksycoca")
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
   m_lstFactories->setAutoDelete( true );
   _self = this;

   // Register app as able to receive DCOP messages
   if (!kapp->dcopClient()->isAttached())
   {
      kapp->dcopClient()->attach();
   }
   if (!kapp->dcopClient()->isRegistered())
   {
      debug("registering as dcopclient...");
      kapp->dcopClient()->registerAs( kapp->name() ); 
   }
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ ) // and not really a dcop object
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
   m_lstFactories->setAutoDelete( true );
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
  debug("KSycoca::~KSycoca()");
   QIODevice *device = 0;
   if (str)
      device = str->device();
   if (device)
      device->close();
      
   delete str;
   delete device;
   delete m_lstFactories;
   _self = 0L;
}

bool KSycoca::process(const QCString &fun, const QByteArray &/*data*/,
                      QCString &replyType, QByteArray &/*replyData*/)
{
  debug(fun);
  if (fun == "databaseChanged()") {
    debug("got a databaseChanged signal !");
    // kded tells us the database file changed
    // Let's just delete everything
    // The next call to any public method will recreate
    // everything that's needed.
    delete this;

    replyType = "void";
    return true;
  } else {
    kdebug(KDEBUG_WARN, 7011, QString("unknown function call to KSycoca::process() : %1").arg(fun));
    return false;
  }
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

void KSycoca::checkVersion()
{
   Q_INT32 aVersion;
   (*str) >> aVersion;
   if ( aVersion != KSYCOCA_VERSION )
   {
      // Do this even if aVersion > KSYCOCA_VERSION (e.g. when downgrading KDE)
      kdebug( KDEBUG_ERROR, 7011, "Outdated database ! Stop kded and restart it !" );
      abort();
   }
}

QDataStream * KSycoca::findFactory(KSycocaFactoryId id)
{
   str->device()->at(0);
   checkVersion();
   Q_INT32 aId;
   Q_INT32 aOffset;
   while(true)
   {
      (*str) >> aId;
      assert( aId > 0 && aId <= 2 ); // to update in case of new factories
      kdebug( KDEBUG_INFO, 7011, QString("KSycoca::findFactory : found factory %1").arg(aId));
      if (aId == 0)
      {
         kdebug(KDEBUG_ERROR, 7011, "Error, KSycocaFactory (id = %d) not found!\n", id);
         break;
      }
      (*str) >> aOffset;
      if (aId == id)
      {
         kdebug( KDEBUG_INFO, 7011, QString("KSycoca::findFactory(%1) offset %2").arg((int)id).arg(aOffset));
         str->device()->at(aOffset);
         return str;
      }
   }
   return 0;
}

QDataStream * KSycoca::findHeader()
{
   str->device()->at(0);
   checkVersion();
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
   // We now point to the header
   return str;
}

KSycoca * KSycoca::_self = 0L;

#include "ksycoca.moc"
