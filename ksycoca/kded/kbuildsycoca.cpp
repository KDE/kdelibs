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
#include "ksycocatype.h"
#include "ksycocaentry.h"
#include "ksycocafactory.h"

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
  kdebug(KDEBUG_INFO, 7011, QString("reading %1").arg(_path));

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
              if ( entry )
                factory->addEntry( entry );
            }
          }
	}
    }
}

void 
KBuildSycoca::save()
{
   // Write header (#pass 1)
   str->device()->at(0);

   KSycocaFactory * servicetypeFactory = 0L;
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      if ( aId == KST_KServiceTypeFactory )
         servicetypeFactory = factory;
      aOffset = factory->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (Q_INT32) 0; // No more factories.
   (*str) << (Q_INT32) 0; // Servicetype index offset

   // Write factory data....
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      factory->save(*str);
   }

   // Write servicetype index
   Q_INT32 servicetypeIndexOffset = str->device()->at();

   // .. TODO
   // for each entry in servicetypeFactory
   // export associated services
   
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
   (*str) << servicetypeIndexOffset;

   // Jump to end of database
   str->device()->at(endOfData);
}

#include "kbuildsycoca.moc"
