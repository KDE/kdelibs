/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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

#include "kded.h"

#include <kbuildservicetypefactory.h>
#include <kbuildservicefactory.h>

#include <unistd.h>
#include <stdlib.h>

#include <qfile.h>
#include <qtimer.h>

#include <kuniqueapp.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>


Kded::Kded() 
  : KSycoca( true )
{
  m_pTimer = new QTimer(this);
  connect (m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  m_pDirWatch = 0;
}
   
Kded::~Kded()
{
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;
}

void Kded::build()
{
  delete m_pDirWatch;
  m_pDirWatch = new KDirWatch;

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
	   this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
	   this, SLOT(dirDeleted(const QString&)));

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  (void) new KBuildServiceFactory(stf);

  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for (KSycocaFactory *factory = m_lstFactories->first(); 
       factory;
       factory = m_lstFactories->first() )
  {
    // For each resource the factory deals with
    for( QStringList::ConstIterator it1 = factory->resourceList()->begin();
         it1 != factory->resourceList()->end(); 
         ++it1 )
    {
      QStringList dirs = KGlobal::dirs()->resourceDirs( *it1 );
      // For each resource the factory deals with
      for( QStringList::ConstIterator it2 = dirs.begin();
           it2 != dirs.end(); 
           ++it2 )
      {
         readDirectory( *it2 );
      }
    }
    m_lstFactories->removeRef(factory);
  }
}

void Kded::recreate()
{
  system("kbuildsycoca"); // Use KLauncher
  build();
}

void Kded::dirDeleted(const QString& /*path*/)
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 1200, true /* single shot */ );
}

void Kded::update(const QString& path)
{
  kdebug(KDEBUG_INFO, 7020, QString("Kded::update( %1 ) - starting timer").arg( path ));
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 1200, true /* single shot */ );
}

void Kded::readDirectory( const QString& _path )
{
  // kdebug(KDEBUG_INFO, 7020, QString("reading %1").arg(_path));

  QDir d( _path, QString::null, QDir::Unsorted, QDir::AccessMask | QDir::Dirs );         
  // set QDir ...
  if ( !d.exists() )                            // exists&isdir?
    return;                             // return false

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

     readDirectory( file );      // yes, dive into it.
  }
}

int main(int argc, char *argv[])
{
     printf("MAIN called.\n");
     if (!KUniqueApplication::start(argc, argv, "kded"))
     {
        printf("KDED already running!\n");
        exit(0);
     }
     KUniqueApplication k(argc,argv, "kded", false, false );

     Kded *kded= new Kded; // Build data base

     kded->build();
     kded->recreate();
     return k.exec(); // keep running
}

#include "kded.moc"
