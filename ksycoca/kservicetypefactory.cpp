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

#include <kglobal.h>
#include <kstddirs.h>

// DEBUG
#include <stdio.h>
#include <kdebug.h>

KServiceTypeFactory::KServiceTypeFactory(bool buildDatabase)
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
      m_entryList = new KServiceTypeList();
      m_entryList->setAutoDelete(true);
      m_entryDict = new KSycocaDict();

      // Read servicetypes first, since they might be needed to read mimetype properties
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "servicetypes" );
      (*m_pathList) += KGlobal::dirs()->resourceDirs( "mime" );
   }
}

KServiceType *
KServiceTypeFactory::createEntry(const QString &file)
{
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return 0;

  KSimpleConfig cfg( file, true);
  cfg.setDesktopGroup();

  QString service = cfg.readEntry( "X-KDE-ServiceType" );
  QString mime = cfg.readEntry( "MimeType" );

/* disable this check because on some system kded tries to read the .desktop file
   before it is really complete. Commenting this out might lead to a corrupt
   kregistry though :-(((                             (Simon probably)
   => what about a delay after the dir watch signal ? (David)

  if ( mime.isEmpty() && service.isEmpty() )
  {
    QString tmp = i18n( "The service/mime type config file\n%1\n"
			"does not contain a ServiceType=...\nor MimeType=... entry").arg( _file );
    KMessageBox::error( 0L, tmp);
    return 0;
  }
*/
  
  KServiceType* e;
#if 0
  if ( mime == "inode/directory" )
    e = new KFolderType( _cfg );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( _cfg );
  else if ( mime == "application/x-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( _cfg );
  else if ( !mime.isEmpty() )
    e = new KMimeType( _cfg );
  else
#endif
    e = new KServiceType( cfg );

  if ( !e->isValid() )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid ServiceType : %s", file.ascii() );
    delete e;
    return 0;
  }

  return e;
}


KServiceTypeFactory::~KServiceTypeFactory()
{
   delete m_pathList;
   delete m_entryList;
   delete m_entryDict;
}

void
KServiceTypeFactory::save(QDataStream &str)
{
   if (!m_entryList) return; // Error! Function should only be called when
                             // building database

   if (!m_entryDict) return; // Error!

   int startOfFactoryData = str.device()->at();
   Q_INT32 entryDictOffset = 0;

   // Write header (pass #1)
   str << entryDictOffset;
   
   // Write all entries.
   for(KServiceType *entry = m_entryList->first(); 
       entry; 
       entry=m_entryList->next())
   {
      entry->save(str);
   }

   // Write indices...
   entryDictOffset = str.device()->at();      

   m_entryDict->save(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #2)
   str.device()->at(startOfFactoryData);
   str << entryDictOffset;

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void 
KServiceTypeFactory::add(KServiceType *newEntry)
{
   if (!m_entryList) return; // Error! Function should only be called when
                             // building database

   if (!m_entryDict) return; // Error!

   m_entryList->append(newEntry);
   m_entryDict->add(newEntry->name(), newEntry);
}


// Static function!
KServiceType *
KServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   if (!self)
      self = new KServiceTypeFactory();
   return self->_findServiceTypeByName(_name);   
}

KServiceType *
KServiceTypeFactory::_findServiceTypeByName(const QString &_name)
{
   if (!m_entryDict) return 0; // Error!
   int offset = m_entryDict->find_string( _name );

   if (!offset) return 0; // Not found

   KServiceType *newServiceType = createServiceType(offset);

   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
      // No it wasn't...
      delete newServiceType;
      newServiceType = 0; // Not found
   }
   return newServiceType;
}

KServiceType *
KServiceTypeFactory::createServiceType(int offset)
{
   KServiceType *newEntry = 0;
   KSycocaType type; 
   QDataStream *str = KSycoca::findEntry(offset, type);
   switch(type)
   {
     case KST_KServiceType:
        newEntry = new KServiceType(*str);
        break;

     default:
fprintf(stderr, "KServiceTypeFactory: unexpected object entry in KSycoca database (type = %d)\n", type);
        break;
   } 
   if (!newEntry->isValid())
   {
fprintf(stderr, "KServiceTypeFactory: corrupt object in KSycoca database!\n");
      delete newEntry;
      newEntry = 0;
   }   
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::self = 0;
