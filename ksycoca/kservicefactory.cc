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
 : m_pathList(0), m_entryList(0)
{
   if (!buildDatabase)
   {
      QDataStream *str = KSycoca::registerFactory( factoryId() );

      // Read position of index tables....
      Q_INT32 entryDictOffset;
      (*str) >> entryDictOffset;

      // Init index tables
      m_entryDict = new KSycocaDict(str, entryDictOffset);   
   }
   else
   {
      // Build new database!
      m_pathList = new QStringList();
      m_entryList = new KServiceList();
      m_entryList->setAutoDelete(true);
      m_entryDict = new KSycocaDict();

      (*m_pathList) += KGlobal::dirs()->resourceDirs( "apps" );
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "services" );
   }
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

