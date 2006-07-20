// -*- c-basic-offset: 3 -*-
/*  This file is part of the KDE libraries
 *  Copyright (C) 1999-2006 David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kbuildmimetypefactory.h"
#include "ksycoca.h"
#include "kdedesktopmimetype.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <kdesktopfile.h>
#include <qhash.h>

KBuildMimeTypeFactory::KBuildMimeTypeFactory() :
  KMimeTypeFactory()
{
   m_resourceList = new KSycocaResourceList;
   m_resourceList->add( "mime", "*.desktop" );
}

// return all resource types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildMimeTypeFactory::resourceTypes()
{
    return QStringList() << "mime";
}

KBuildMimeTypeFactory::~KBuildMimeTypeFactory()
{
   delete m_resourceList;
}

KMimeType::Ptr KBuildMimeTypeFactory::findMimeTypeByName(const QString &_name)
{
   assert (KSycoca::self()->isBuilding());
   // We're building a database - the mime type must be in memory
   KSycocaEntry::Ptr servType = m_entryDict->value( _name );
   return KMimeType::Ptr::staticCast( servType );
}

KSycocaEntry::List KBuildMimeTypeFactory::allEntries()
{
   assert (KSycoca::self()->isBuilding());
   KSycocaEntry::List lst;
   KSycocaEntryDict::Iterator itmime = m_entryDict->begin();
   const KSycocaEntryDict::Iterator endmime = m_entryDict->end();
   for( ; itmime != endmime ; ++itmime )
      lst.append( *itmime );
   return lst;
}

KSycocaEntry *
KBuildMimeTypeFactory::createEntry(const QString &file, const char *resource)
{
  QString name = file;
  int pos = name.lastIndexOf('/');
  if (pos != -1)
  {
     name = name.mid(pos+1);
  }

  if (name.isEmpty())
     return 0;

  KDesktopFile desktopFile(file, true, resource);

  if ( desktopFile.readEntry( "Hidden", false ) == true )
    return 0;

  const QString type = desktopFile.readEntry( "Type" );
  if ( type != QLatin1String( "MimeType" ) )
  {
     kWarning(7012) << "The mime type config file " << desktopFile.fileName() << " has Type=" << type << " instead of Type=MimeType" << endl;
    return 0;
  }

  const QString mime = desktopFile.readEntry( "MimeType" );

  if ( mime.isEmpty() )
  {
     kWarning(7012) << "The mime type config file " << desktopFile.fileName() << " does not contain a MimeType=... entry" << endl;
    return 0;
  }

  KMimeType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( &desktopFile );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( &desktopFile );
  else
    e = new KMimeType( &desktopFile );

  if (e->isDeleted())
  {
    delete e;
    return 0;
  }

  if ( !(e->isValid()) )
  {
    kWarning(7012) << "Invalid MimeType : " << file << endl;
    delete e;
    return 0;
  }

  return e;
}

void
KBuildMimeTypeFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);
   str << (qint32) m_fastPatternOffset;
   str << (qint32) m_otherPatternOffset;
}

void
KBuildMimeTypeFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   savePatternLists(str);

   int endOfFactoryData = str.device()->pos();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->seek(endOfFactoryData);
}

void
KBuildMimeTypeFactory::savePatternLists(QDataStream &str)
{
   // Store each patterns in one of the 2 string lists (for sorting)
   QStringList fastPatterns;  // for *.a to *.abcd
   QStringList otherPatterns; // for the rest (core.*, *.tar.bz2, *~) ...
   QHash<QString, const KMimeType*> dict; // KMimeType::Ptr not needed here, this is short term

   // For each mimetype in mimetypeFactory
   for(KSycocaEntryDict::Iterator it = m_entryDict->begin();
       it != m_entryDict->end();
       ++it)
   {
      const KSycocaEntry::Ptr& entry = (*it);
      Q_ASSERT( entry->isType( KST_KMimeType ) );

      const KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( entry );
      const QStringList pat = mimeType->patterns();
      QStringList::ConstIterator patit = pat.begin();
      for ( ; patit != pat.end() ; ++patit )
      {
         const QString &pattern = *patit;
         if ( pattern.lastIndexOf('*') == 0
              && pattern.lastIndexOf('.') == 1
              && pattern.length() <= 6 )
            // it starts with "*.", has no other '*' and no other '.', and is max 6 chars
            // => fast patttern
            fastPatterns.append( pattern );
         else if (!pattern.isEmpty()) // some stupid mimetype files have "Patterns=;"
            otherPatterns.append( pattern );
         // Assumption : there is only one mimetype for that pattern
         // It doesn't really make sense otherwise, anyway.
         dict.insert( pattern, mimeType.constData() );
      }
   }
   // Sort the list - the fast one, useless for the other one
   fastPatterns.sort();

   qint32 entrySize = 0;
   qint32 nrOfEntries = 0;

   m_fastPatternOffset = str.device()->pos();

   // Write out fastPatternHeader (Pass #1)
   str.device()->seek(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For each fast pattern
   for ( QStringList::ConstIterator it = fastPatterns.begin(); it != fastPatterns.end() ; ++it )
   {
     int start = str.device()->pos();
     // Justify to 6 chars with spaces, so that the size remains constant
     // in the database file.
     QString paddedPattern = (*it).leftJustified(6).right(4); // remove leading "*."
     //kDebug(7021) << QString("FAST : '%1' '%2'").arg(paddedPattern).arg(dict[(*it)]->name()) << endl;
     str << paddedPattern;
     str << dict[(*it)]->offset();
     entrySize = str.device()->pos() - start;
     nrOfEntries++;
   }

   // store position
   m_otherPatternOffset = str.device()->pos();

   // Write out fastPatternHeader (Pass #2)
   str.device()->seek(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For the other patterns
   str.device()->seek(m_otherPatternOffset);

   for ( QStringList::ConstIterator it = otherPatterns.begin(); it != otherPatterns.end() ; ++it )
   {
     //kDebug(7021) << QString("OTHER : '%1' '%2'").arg(*it).arg(dict[(*it)]->name()) << endl;
     str << (*it);
     str << dict[(*it)]->offset();
   }

   str << QString(""); // end of list marker (has to be a string !)
}

void
KBuildMimeTypeFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
   KMimeType::Ptr mimeType = KMimeType::Ptr::staticCast( newEntry );
   if ( m_entryDict->value( newEntry->name() ) )
   {
     // Already exists -> replace
     KSycocaFactory::removeEntry(newEntry->name());
   }
   KSycocaFactory::addEntry(newEntry);
}
