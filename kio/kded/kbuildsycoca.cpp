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
#include <ksycocatype.h>
#include <ksycocaentry.h>
#include <ksycocafactory.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qtimer.h>

#include <assert.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>

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

void KBuildSycoca::addFactory( KSycocaFactory *factory)
{
   assert(m_lstFactories);
   m_lstFactories->append(factory);
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
  // TODO : set a flag to prevent re-entrance here (if update() gets called while building)
  debug("KBuildSycoca::recreate()");
  build();
  save();
  // TODO notify applications using DCOP !
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
  debug(QString("KBuildSycoca::update( %1 ) - starting timer").arg( path ));
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 5000, true /* single shot */ );
}

void KBuildSycoca::readDirectory( const QString& _path, KSycocaFactory * factory )
{
  //kdebug(KDEBUG_INFO, 7011, QString("reading %1").arg(_path));

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
     kdebug(KDEBUG_WARN, 7011, "Don't have the two mandatory factories. No servicetype index.");
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
            //kdebug(KDEBUG_INFO, 7011, QString("<< %1 %2")
            //       .arg(it.current()->offset(),8,16).arg(itserv.current()->offset(),8,16));
         }
      }
   }
   (*str) << (Q_INT32) 0;               // End of list marker (0)
}


/**
 * Saves the mimetype patterns index
 * @return the index of the start of the index
 */
void KBuildSycoca::saveMimeTypePattern( KSycocaFactory * servicetypeFactory )
{
   if (!servicetypeFactory)
   {
     kdebug(KDEBUG_WARN, 7011, "No service type factory. Can't save mimetype patterns index.");
     return;
   }
   // Store each pattern in a string list(for sorting)
   QStringList patterns;
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
          if (!(*patit).isEmpty()) // some stupid mimetype files have "Patterns=;"
          {
            patterns.append( (*patit) );
            dict.insert( (*patit), (KMimeType *) it.current() );
          }
      }
   }
   // Sort the list
   patterns.sort();
   // Now for each pattern
   QStringList::ConstIterator it = patterns.begin();
   for ( ; it != patterns.end() ; ++it )
   {
     debug("SORTED : '%s' '%s'",(*it).ascii(), dict[(*it)]->name().ascii());
   }
   
   (*str) << (Q_INT32) 0;               // End of list marker (0)
}

void KBuildSycoca::save()
{
   // Write header (#pass 1)
   str->device()->at(0);

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
   saveMimeTypePattern( servicetypeFactory );
   
   int endOfData = str->device()->at();

   // Write header (#pass 2)
   str->device()->at(0);

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
   kdebug(KDEBUG_INFO, 7011, QString("offerListOffset : %1").
          arg(offerListOffset,8,16));
   (*str) << offerListOffset;
   kdebug(KDEBUG_INFO, 7011, QString("mimeTypesPatternsOffset : %1").
          arg(mimeTypesPatternsOffset,8,16));
   (*str) << mimeTypesPatternsOffset;;

   kdebug(KDEBUG_INFO, 7011, QString("endOfData : %1").
          arg(endOfData,8,16));

   // Jump to end of database
   str->device()->at(endOfData);
}

#include "kbuildsycoca.moc"
