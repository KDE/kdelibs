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

#include "kservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservicetype.h"
#include "kmimetype.h"

#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <assert.h>

KServiceTypeFactory::KServiceTypeFactory()
 : KSycocaFactory( KST_KServiceTypeFactory )
{
   if (KSycoca::self()->isBuilding())
   {
      // Read servicetypes first, since they might be needed to read mimetype properties
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "servicetypes" );
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "mime" );
   }
   _self = this;
}

KSycocaEntry *
KServiceTypeFactory::createEntry(const QString &file)
{
  //debug("KServiceTypeFactory::createEntry(%s)",file.ascii());
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return 0;

  KSimpleConfig cfg( file, true);
  cfg.setDesktopGroup();

  QString mime = cfg.readEntry( "MimeType" );
  QString service = cfg.readEntry( "X-KDE-ServiceType" );

  if ( mime.isEmpty() && service.isEmpty() )
  {
    QString tmp = i18n( "The service/mime type config file\n%1\n"
			"does not contain a ServiceType=...\nor MimeType=... entry").arg( file );
    KMessageBoxWrapper::error( 0L, tmp);
    return 0;
  }
  
  KServiceType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( cfg );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( cfg );
  else if ( mime == "application/x-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( cfg );
  else if ( !mime.isEmpty() )
    e = new KMimeType( cfg );
  else
    e = new KServiceType( cfg );

  if ( !(e->isValid()) )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid ServiceType : %s", file.ascii() );
    delete e;
    return 0;
  }

  return e;
}

KServiceTypeFactory::~KServiceTypeFactory()
{
}

KServiceTypeFactory * KServiceTypeFactory::self()
{
  if (!_self)
    _self = new KServiceTypeFactory();
  return _self;
}

KServiceType * KServiceTypeFactory::findServiceTypeRef(const QString &_name)
{
   assert (KSycoca::self()->isBuilding());
   // We're building a database - the service type must be in memory
   KSycocaEntry * servType = (*m_entryDict)[ _name ];
   return (KServiceType *) servType;
}

KServiceType * KServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   if (!m_sycocaDict) return 0L; // Error!
   assert (!KSycoca::self()->isBuilding());
   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found
   KServiceType * newServiceType = createServiceType(offset);
   
   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
     // No it wasn't...
     delete newServiceType;
     newServiceType = 0; // Not found
   }
   return newServiceType;
}

KMimeType::List KServiceTypeFactory::allMimeTypes()
{
   kdebug(KDEBUG_INFO, 7011, "KServiceTypeFactory::allMimeTypes()");
   KMimeType::List list;
   // Assume we're NOT building a database
   // Get stream to factory start
   QDataStream *str = KSycoca::self()->registerFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;

   int offset = str->device()->at();
   KServiceType *newServiceType;
   while ( offset < sycocaDictOffset )
   {
      newServiceType = createServiceType(offset);
      // We don't want service types, but we have to build them
      // anyway, to skip their info
      if (newServiceType && newServiceType->isType( KST_KMimeType ))
      {
         KMimeType * mimeType = (KMimeType *) newServiceType;
         list.append( KMimeType::Ptr( mimeType ) );
      }

      offset = str->device()->at();
   }
   return list;
}

KServiceType::List KServiceTypeFactory::allServiceTypes()
{
   kdebug(KDEBUG_INFO, 7011, "KServiceTypeFactory::allServiceTypes()");
   KServiceType::List list;
   // Assume we're NOT building a database
   // Get stream to factory start
   QDataStream *str = KSycoca::self()->registerFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;

   int offset = str->device()->at();
   KServiceType *newServiceType;
   while ( offset < sycocaDictOffset )
   {
      newServiceType = createServiceType(offset);
      if (newServiceType)
         list.append( KServiceType::Ptr( newServiceType ) );

      offset = str->device()->at();
   }
   return list;
}

bool KServiceTypeFactory::checkMimeTypes()
{
   QDataStream *str = KSycoca::self()->registerFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;
   // There are mimetypes/servicetypes if the dict offset is
   // not right now in the file
   return (str->device()->at() < sycocaDictOffset);
}

KServiceType * KServiceTypeFactory::createServiceType(int offset)
{
   KServiceType *newEntry = 0;
   KSycocaType type; 
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch(type)
   {
     case KST_KServiceType:
        newEntry = new KServiceType(*str, offset);
        break;
     case KST_KMimeType:
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderType:
        newEntry = new KFolderType(*str, offset);
        break;
     case KST_KDEDesktopMimeType:
        newEntry = new KDEDesktopMimeType(*str, offset);
        break;
     case KST_KExecMimeType:
        newEntry = new KExecMimeType(*str, offset);
        break;

     default:
        kdebug( KDEBUG_ERROR, 7011, QString("KServiceTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) );
        break;
   } 
   if (!newEntry->isValid())
   {
      kdebug( KDEBUG_ERROR, 7011, "KServiceTypeFactory: corrupt object in KSycoca database!\n");
      delete newEntry;
      newEntry = 0;
   }   
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::_self = 0;

#include "kservicetypefactory.moc"
