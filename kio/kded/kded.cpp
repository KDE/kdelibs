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
#include <kresourcelist.h>

#include <unistd.h>
#include <stdlib.h>

#include <qfile.h>
#include <qtimer.h>

#include <dcopclient.h>

#include <kuniqueapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>
#include <kimageiofactory.h>

Kded::Kded( bool needUpdate)
  : KSycoca( true )
{
  QString path = KGlobal::dirs()->saveLocation("config")+"ksycoca";
  QCString cPath = QFile::encodeName(path);
  struct stat buff;
  if (!needUpdate && (stat( cPath, &buff) == 0))
  {
     m_sycocaDate = buff.st_ctime;
     m_needUpdate = false;
  }
  else
  {
     m_sycocaDate = 0;
     m_needUpdate = true;
  }
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
  KBuildServiceGroupFactory *bsgf = new KBuildServiceGroupFactory;
  (void) new KBuildServiceFactory(stf, bsgf);
  // We don't include KImageIOFactory here because it doesn't add
  // new resourceList entries anyway.
  //(void) KImageIOFactory::self();

  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->first() )
  {
    // For each resource the factory deals with
    for( KSycocaResourceList::ConstIterator it1 = factory->resourceList()->begin();
         it1 != factory->resourceList()->end();
         ++it1 )
    {
      KSycocaResource res = (*it1);
      QStringList dirs = KGlobal::dirs()->resourceDirs( res.resource );
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
   // Using KLauncher here is difficult since we might not have a
   // database

   system("kbuildsycoca");
   build();
}

void Kded::dirDeleted(const QString& /*path*/)
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 2000, true /* single shot */ );
}

void Kded::update(const QString& )
{
  // We could be smarter here, and find out which factory
  // deals with that dir, and update only that...
  // But rebuilding everything is fine for me.
  m_pTimer->start( 2000, true /* single shot */ );
}

bool Kded::process(const QCString &fun, const QByteArray &/*data*/,
			   QCString &replyType, QByteArray &/*replyData*/)
{
  if (fun == "recreate()") {
    kdDebug() << "got a recreate signal!" << endl;
    recreate();
    replyType = "void";
    return true;
  } else
    return false;
    // don't call KSycoca::process - this is for other apps, not k
}


void Kded::readDirectory( const QString& _path )
{
  // kdDebug(7020) << QString("reading %1").arg(_path) << endl;

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
  {
    m_pDirWatch->addDir(path);          // add watch on this dir
    if (!m_needUpdate)
    {
       time_t ctime = m_pDirWatch->ctime(path);
       if (ctime && (ctime > m_sycocaDate))
          m_needUpdate = true;
    }
  }

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

static KCmdLineOptions options[] =
{
  { "check", I18N_NOOP("Check sycoca database only once."), 0 },
  { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
     KAboutData aboutData( "kded", I18N_NOOP("KDE Daemon"),
        "$Id$",
        I18N_NOOP("KDE Daemon - triggers Sycoca database updates when needed."));

     KCmdLineArgs::init(argc, argv, &aboutData);

     KUniqueApplication::addCmdLineOptions();

     KCmdLineArgs::addCmdLineOptions( options );

     // this program is in kdelibs so it uses kdelibs as catalogue
     KLocale::setMainCatalogue("kdelibs");

     // WABA: Make sure not to enable session management.
     putenv("SESSION_MANAGER=");

     KInstance *instance = new KInstance(&aboutData);
     (void) instance->config(); // Enable translations.

     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
     bool check = args->isSet("check");

     if (!check)
     {
        if (!KUniqueApplication::start())
        {
           fprintf(stderr, "KDE Daemon (kded) already running.\n");
           exit(0);
        }
     }

     QString ksycoca_kfsstnd = KSycoca::self()->kfsstnd_prefixes();
     delete KSycoca::self();
     QString current_kfsstnd = KGlobal::dirs()->kfsstnd_prefixes();

     Kded *kded = new Kded(ksycoca_kfsstnd != current_kfsstnd); // Build data base

     kded->build();

     bool needUpdate = kded->needUpdate();
     if (needUpdate)
        kded->recreate();

     if (check)
        return 0;

     KUniqueApplication k( false, false ); // No styles, no GUI

     if (!needUpdate)
     {
        // During startup kdesktop waits for KDED to finish.
        // Send a databaseChanged signal even if the database hasn't
        // changed.
        QByteArray data;
        kapp->dcopClient()->send( "*", "ksycoca", "databaseChanged()", data );
     }

     return k.exec(); // keep running
}

#include "kded.moc"
