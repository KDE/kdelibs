/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
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

#include "kservicefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservice.h"

#include <qstring.h>
#include <kmessagebox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstddirs.h>

KServiceFactory::KServiceFactory(bool buildDatabase)
 : KSycocaFactory( buildDatabase, KST_KServiceFactory )
{
   if (buildDatabase)
   {
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "apps" );
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "services" );
   }
}

KServiceFactory::~KServiceFactory()
{
}

/*
KService* KServiceFactory::createEntry( const QString& _file )
{
  Q_UINT32 u;
  _str >> u;

  if ( u != TC_KService )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid Service : %s", _file.ascii() );
    return 0;
  }

  KService *s = new KService( _str );
  if ( !s->isValid() )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid Service : %s", _file.ascii() );
    delete s;
    return 0;
  }

  KServiceEntry* e = new KServiceEntry( _reg, _file, s, this );

  e->load( _str );

  return e;
}

KRegEntry* KServiceFactory::create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg )
{
  KService *service = new KService( _cfg );
  if ( !service->isValid() )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid Service : %s", _file.ascii() );
    delete service;
    return 0;
  }

  KServiceEntry* e = new KServiceEntry( _reg, _file, service, this );

  return e;
}
*/

// Static function!
KService *
KServiceFactory::findServiceByName(const QString &_name)
{
   if (!self)
      self = new KServiceFactory();
   return self->_findServiceByName(_name);   
}

KService *
KServiceFactory::_findServiceByName(const QString &_name)
{
   if (!m_entryDict) return 0; // Error!
   int offset = m_entryDict->find_string( _name );

   if (!offset) return 0; // Not found

   KService *newService = createService(offset);

   // Check whether the dictionary was right.
   if (newService && (newService->name() != _name))
   {
      // No it wasn't...
      delete newService;
      newService = 0; // Not found
   }
   return newService;
}

KService *
KServiceFactory::createService(int offset)
{
   KService *newEntry = 0;
   KSycocaType type; 
   QDataStream *str = KSycoca::findEntry(offset, type);
   switch(type)
   {
     case KST_KService:
        newEntry = new KService(*str);
        break;

     default:
        QString tmp = i18n("KServiceFactory: unexpected object entry in KSycoca database (type = %1)\n");
        KMessageBox::error( 0L, tmp.arg((int)type) );
        break;
   } 
   if (!newEntry->isValid())
   {
      KMessageBox::error( 0L, i18n("KServiceFactory: corrupt object in KSycoca database!\n") );
      delete newEntry;
      newEntry = 0;
   }   
   return newEntry;
}

KServiceFactory *KServiceFactory::self = 0;
