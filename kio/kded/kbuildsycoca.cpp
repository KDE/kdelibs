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

#include <qdir.h>

#include "kbuildsycoca.h"

#include <kservice.h>
#include <kmimetype.h>
#include <kbuildservicetypefactory.h>
#include <kservicefactory.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qtimer.h>

#include <assert.h>
#include <kapp.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>
#include <unistd.h>

KBuildSycoca::KBuildSycoca() : KSycoca( true )
{
  m_pTimer = new QTimer(this);
  connect (m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  m_pDirWatch = new KDirWatch;

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
	   this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
	   this, SLOT(dirDeleted(const QString&)));
}
   
KBuildSycoca::~KBuildSycoca()
{
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;
}

void KBuildSycoca::clear()
{
  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for ( ; factit.current(); ++factit )
  {
    // Clear it
    factit.current()->clear();
  }
  m_lstFactories->clear();
}

void KBuildSycoca::build()
{
  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for ( ; factit.current(); ++factit )
  {
    // Clear it
    factit.current()->clear();
    // For each path the factory deals with
    QStringList::ConstIterator it = factit.current()->pathList()->begin();
    for( ; it != factit.current()->pathList()->end(); ++it )
    {
      readDirectory( *it, factit.current() );
    }
  }
}

void KBuildSycoca::recreate()
{
  QString path = KGlobal::dirs()->saveLocation("config") + "ksycoca.building";
  QFile *database = new QFile(path);
  if (!database->open( IO_ReadWrite ))
  {
    fprintf(stderr, "Error can't open database!\n");
    exit(-1);
  }
  str = new QDataStream(database);

  kdebug(KDEBUG_INFO, 7020, "Recreating ksycoca file");
     
  // It is very important to build the servicetype one first
  // Both a registered in KSycoca, no need to keep the pointers
  (void) new KBuildServiceTypeFactory;
  (void) new KServiceFactory;
  
  build(); // Parse dirs
  save(); // Save database
  clear(); // save memory usage

  // Notify ALL applications that have a ksycoca object, using a broadcast
  QByteArray data;
  kapp->dcopClient()->send( "*", "ksycoca", "databaseChanged()", data );
}

void KBuildSycoca::dirDeleted(const QString& /*path*/)
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 5000, true /* single shot */ );
}

void KBuildSycoca::update(const QString& path)
{
  kdebug(KDEBUG_INFO, 7020, QString("KBuildSycoca::update( %1 ) - starting timer").arg( path ));
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 5000, true /* single shot */ );
}

void KBuildSycoca::readDirectory( const QString& _path, KSycocaFactory * factory )
{
  //kdebug(KDEBUG_INFO, 7020, QString("reading %1").arg(_path));

  QDir d( _path );                               // set QDir ...
  if ( !d.exists() )                            // exists&isdir?
    return;                             // return false
  d.setSorting(QDir::Name);                  // just name

  QString path( _path );
  if ( path.right(1) != "/" )
    path += "/";

  QString file;

  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  if ( !m_pDirWatch->contains( path ) ) // New dir?
    m_pDirWatch->addDir(path);          // add watch on this dir

  // Note: If some directory is gone, dirwatch will delete it from the list.

  //************************************************************************
  //                               Reading
  //************************************************************************

  unsigned int i;                           // counter and string length.
  unsigned int count = d.count();
  for( i = 0; i < count; i++ )                        // check all entries
    {
      if (d[i] == "." || d[i] == ".." || d[i] == "magic")
	continue;                          // discard those ".", "..", "magic"...

      file = path;                           // set full path
      file += d[i];                          // and add the file name.
      struct stat m_statbuff;
      if ( stat( file.ascii(), &m_statbuff ) == -1 )           // get stat...
	continue;                                   // no such, continue.

      if ( S_ISDIR( m_statbuff.st_mode ) )               // isdir?
	{
          readDirectory( file, factory );      // yes, dive into it.
	}
      else                                         // no, not a dir/no recurse...
	{
          if ( file.right(1) != "~" )
          {
            // Can we read the file ?
            if ( access( file.ascii(), R_OK ) != -1 )
            {
              // Create a new entry
              KSycocaEntry* entry = factory->createEntry( file );
              if ( entry && entry->isValid() )
                factory->addEntry( entry );
            }
          }
	}
    }
}

void KBuildSycoca::saveOfferList( KSycocaFactory * serviceFactory,
                                  KSycocaFactory * servicetypeFactory )
{
   if (!serviceFactory || !servicetypeFactory)
   {
     kdebug(KDEBUG_WARN, 7020, "Don't have the two mandatory factories. No servicetype index.");
     return;
   }
   // For each entry in servicetypeFactory
   for(QDictIterator<KSycocaEntry> it ( *(servicetypeFactory->entryDict()) );
       it.current(); 
       ++it)
   {
      // export associated services
      // This means looking for the service type in ALL services
      // This is SLOW. But it used to be done in every app (in KServiceTypeProfile)
      // Doing it here saves a lot of time to the clients
      QString serviceType = it.current()->name();
      for(QDictIterator<KSycocaEntry> itserv ( *(serviceFactory->entryDict()) );
          itserv.current(); 
          ++itserv)
      {
         if ( ((KService *)itserv.current())->hasServiceType( serviceType ) )
         {
            (*str) << (Q_INT32) it.current()->offset();
            (*str) << (Q_INT32) itserv.current()->offset();
            //kdebug(KDEBUG_INFO, 7020, QString("<< %1 %2")
            //       .arg(it.current()->offset(),8,16).arg(itserv.current()->offset(),8,16));
         }
      }
   }
   (*str) << (Q_INT32) 0;               // End of list marker (0)
}

void KBuildSycoca::saveMimeTypePattern( KSycocaFactory * servicetypeFactory, 
                                        Q_INT32 & entrySize,
                                        Q_INT32 & otherIndexOffset )
{
   if (!servicetypeFactory)
   {
     kdebug(KDEBUG_WARN, 7020, "No service type factory. Can't save mimetype patterns index.");
     entrySize = 0;
     otherIndexOffset = 0;
     return;
   }
   // Store each patterns in one of the 2 string lists (for sorting)
   QStringList fastPatterns;  // for *.a to *.abcd
   QStringList otherPatterns; // for the rest (core.*, *.tar.bz2, *~) ...
   QDict<KMimeType> dict;
   // For each mimetype in servicetypeFactory
   for(QDictIterator<KSycocaEntry> it ( *(servicetypeFactory->entryDict()) );
       it.current(); 
       ++it)
   {
      if ( it.current()->isType( KST_KMimeType ) )
      {
        QStringList pat = ( (KMimeType *) it.current())->patterns();
        QStringList::ConstIterator patit = pat.begin();
        for ( ; patit != pat.end() ; ++patit )
        {
           if ( (*patit).findRev('*') == 0 
                && (*patit).findRev('.') == 1 
                && (*patit).length() <= 6 )
              // it starts with "*.", has no other '*' and no other '.', and is max 6 chars
              // => fast patttern
              fastPatterns.append( (*patit) );
           else if (!(*patit).isEmpty()) // some stupid mimetype files have "Patterns=;"
              otherPatterns.append( (*patit) );
           // Assumption : there is only one mimetype for that pattern
           // It doesn't really make sense otherwise, anyway.
           dict.replace( (*patit), (KMimeType *) it.current() );
        }
      }
   }
   // Sort the list - the fast one, useless for the other one
   fastPatterns.sort();
   
   entrySize = 0;

   // For each fast pattern
   QStringList::ConstIterator it = fastPatterns.begin();
   for ( ; it != fastPatterns.end() ; ++it )
   {
     int start = str->device()->at();
     // Justify to 6 chars with spaces, so that the size remains constant
     // in the database file.
     QString paddedPattern = (*it).leftJustify(6).right(4); // remove leading "*."
     //kdebug(KDEBUG_INFO, 7020, "%s", QString("FAST : '%1' '%2'").arg(paddedPattern).arg(dict[(*it)]->name()).latin1());
     (*str) << paddedPattern;
     (*str) << dict[(*it)]->offset();
     // Check size remains constant
     assert( !entrySize || ( entrySize == str->device()->at() - start ) );
     entrySize = str->device()->at() - start;
   }
   // For the other patterns
   otherIndexOffset = str->device()->at();
   it = otherPatterns.begin();
   for ( ; it != otherPatterns.end() ; ++it )
   {
     //kdebug(KDEBUG_INFO, 7020, "%s", QString("OTHER : '%1' '%2'").arg(*it).arg(dict[(*it)]->name()).latin1());
     (*str) << (*it);
     (*str) << dict[(*it)]->offset();
   }
   
   (*str) << QString(""); // end of list marker (has to be a string !)
}

void KBuildSycoca::save()
{
   // Write header (#pass 1)
   str->device()->at(0);

   (*str) << (Q_INT32) KSYCOCA_VERSION;
   KSycocaFactory * servicetypeFactory = 0L;
   KSycocaFactory * serviceFactory = 0L;
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      if ( aId == KST_KServiceTypeFactory )
         servicetypeFactory = factory;
      else if ( aId == KST_KServiceFactory )
         serviceFactory = factory;
      aOffset = factory->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (Q_INT32) 0; // No more factories.
   (*str) << (Q_INT32) 0; // Offer list offset
   (*str) << (Q_INT32) 0; // Mimetype patterns index offset
   (*str) << (Q_INT32) 0; // 'Other' patterns index offset
   (*str) << (Q_INT32) 0; // Entry size in the mimetype-patterns index ("fast" part)

   // Write factory data....
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      factory->save(*str);
   }

   Q_INT32 offerListOffset = str->device()->at();
   saveOfferList( serviceFactory, servicetypeFactory );

   Q_INT32 mimeTypesPatternsOffset = str->device()->at();
   Q_INT32 entrySize;
   Q_INT32 otherIndexOffset;
   saveMimeTypePattern( servicetypeFactory, entrySize, otherIndexOffset );
   
   int endOfData = str->device()->at();

   // Write header (#pass 2)
   str->device()->at(0);

   (*str) << (Q_INT32) KSYCOCA_VERSION;
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      aOffset = factory->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (Q_INT32) 0; // No more factories.
   kdebug(KDEBUG_INFO, 7020, QString("offerListOffset : %1").
          arg(offerListOffset,8,16));
   (*str) << offerListOffset;
   kdebug(KDEBUG_INFO, 7020, QString("mimeTypesPatternsOffset : %1").
          arg(mimeTypesPatternsOffset,8,16));
   (*str) << mimeTypesPatternsOffset;;
   kdebug(KDEBUG_INFO, 7020, QString("otherIndexOffset : %1").
          arg(otherIndexOffset,8,16));
   (*str) << otherIndexOffset;;
   (*str) << entrySize;

   kdebug(KDEBUG_INFO, 7020, QString("endOfData : %1").
          arg(endOfData,8,16));

   // Jump to end of database
   str->device()->at(endOfData);

   // Close database file
   assert( str );
   QIODevice *device = str->device();
   assert( device );
   device->close();
   delete str;
   str = 0L;

   // Make public the database we just built
   QString from = KGlobal::dirs()->saveLocation("config") + "ksycoca.building";
   QString to = KGlobal::dirs()->saveLocation("config") + "ksycoca";
   if ( unlink( to.ascii() ) != 0 )
     kdebug(KDEBUG_ERROR, 7020, "Can't unlink %s", to.ascii());
   if ( rename( from.ascii(), to.ascii() ) != 0 )
     kdebug(KDEBUG_ERROR, 7020, "Can't rename %s to %s", from.ascii(), to.ascii());
}

#include "kbuildsycoca.moc"
