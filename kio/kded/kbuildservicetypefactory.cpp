/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
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

#include "kbuildservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <kdesktopfile.h>

template class QDict<KMimeType>;

KBuildServiceTypeFactory::KBuildServiceTypeFactory() :
  KServiceTypeFactory()
{
   // Read servicetypes first, since they might be needed to read mimetype properties
   m_resourceList = new KSycocaResourceList;
   m_resourceList->add("servicetypes", "*.desktop");
   m_resourceList->add("servicetypes", "*.kdelnk");
   m_resourceList->add( "mime", "*.desktop" );
   m_resourceList->add( "mime", "*.kdelnk" );
}

KBuildServiceTypeFactory::~KBuildServiceTypeFactory()
{
   delete m_resourceList;
}

KServiceType * KBuildServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   assert (KSycoca::self()->isBuilding());
   // We're building a database - the service type must be in memory
   KSycocaEntry * servType = (*m_entryDict)[ _name ];
   return (KServiceType *) servType;
}


KSycocaEntry *
KBuildServiceTypeFactory::createEntry(const QString &file, const char *resource)
{
  QString name = file;
  int pos = name.findRev('/');
  if (pos != -1)
  {
     name = name.mid(pos+1);
  }

  if (name.isEmpty())
     return 0;

  KDesktopFile desktopFile(file, true, resource);

  // TODO check Type field first
  QString mime = desktopFile.readEntry( "MimeType" );
  QString service = desktopFile.readEntry( "X-KDE-ServiceType" );

  if ( mime.isEmpty() && service.isEmpty() )
  {
    QString tmp = QString("The service/mime type config file\n%1\n"
		  "does not contain a ServiceType=...\nor MimeType=... entry").arg( file );
    kdWarning(7012) << tmp << endl;
    return 0;
  }

  KServiceType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( &desktopFile );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( &desktopFile );
  else if ( mime == "application/x-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( &desktopFile );
  else if ( !mime.isEmpty() )
    e = new KMimeType( &desktopFile );
  else
    e = new KServiceType( &desktopFile );

  if ( !(e->isValid()) )
  {
    kdWarning(7012) << "Invalid ServiceType : " << file << endl;
    delete e;
    return 0;
  }

  return e;
}

void
KBuildServiceTypeFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);
   str << (Q_INT32) m_fastPatternOffset;
   str << (Q_INT32) m_otherPatternOffset;
}

void
KBuildServiceTypeFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   savePatternLists(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void
KBuildServiceTypeFactory::savePatternLists(QDataStream &str)
{
   // Store each patterns in one of the 2 string lists (for sorting)
   QStringList fastPatterns;  // for *.a to *.abcd
   QStringList otherPatterns; // for the rest (core.*, *.tar.bz2, *~) ...
   QDict<KMimeType> dict;

   // For each mimetype in servicetypeFactory
   for(QDictIterator<KSycocaEntry> it ( *m_entryDict );
       it.current();
       ++it)
   {
      if ( it.current()->isType( KST_KMimeType ) )
      {
        KMimeType *mimeType = (KMimeType *) it.current();
        QStringList pat = mimeType->patterns();
        QStringList::ConstIterator patit = pat.begin();
        for ( ; patit != pat.end() ; ++patit )
        {
           const QString &pattern = *patit;
           if ( pattern.findRev('*') == 0
                && pattern.findRev('.') == 1
                && pattern.length() <= 6 )
              // it starts with "*.", has no other '*' and no other '.', and is max 6 chars
              // => fast patttern
              fastPatterns.append( pattern );
           else if (!pattern.isEmpty()) // some stupid mimetype files have "Patterns=;"
              otherPatterns.append( pattern );
           // Assumption : there is only one mimetype for that pattern
           // It doesn't really make sense otherwise, anyway.
           dict.replace( pattern, mimeType );
        }
      }
   }
   // Sort the list - the fast one, useless for the other one
   fastPatterns.sort();

   Q_INT32 entrySize = 0;
   Q_INT32 nrOfEntries = 0;

   m_fastPatternOffset = str.device()->at();

   // Write out fastPatternHeader (Pass #1)
   str.device()->at(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For each fast pattern
   QStringList::ConstIterator it = fastPatterns.begin();
   for ( ; it != fastPatterns.end() ; ++it )
   {
     int start = str.device()->at();
     // Justify to 6 chars with spaces, so that the size remains constant
     // in the database file.
     QString paddedPattern = (*it).leftJustify(6).right(4); // remove leading "*."
     //kdDebug(7020) << QString("FAST : '%1' '%2'").arg(paddedPattern).arg(dict[(*it)]->name()) << endl;
     str << paddedPattern;
     str << dict[(*it)]->offset();
     // Check size remains constant
     assert( !entrySize || ( entrySize == str.device()->at() - start ) );
     entrySize = str.device()->at() - start;
     nrOfEntries++;
   }

   // store position
   m_otherPatternOffset = str.device()->at();

   // Write out fastPatternHeader (Pass #2)
   str.device()->at(m_fastPatternOffset);
   str << nrOfEntries;
   str << entrySize;

   // For the other patterns
   str.device()->at(m_otherPatternOffset);

   it = otherPatterns.begin();
   for ( ; it != otherPatterns.end() ; ++it )
   {
     //kdDebug(7020) << QString("OTHER : '%1' '%2'").arg(*it).arg(dict[(*it)]->name()) << endl;
     str << (*it);
     str << dict[(*it)]->offset();
   }

   str << QString(""); // end of list marker (has to be a string !)
}
