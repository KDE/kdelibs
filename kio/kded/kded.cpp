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
#include "kdedmodule.h"

#include <kbuildservicetypefactory.h>
#include <kbuildservicefactory.h>
#include <kresourcelist.h>
#include <kcrash.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <qfile.h>
#include <qtimer.h>

#include <dcopclient.h>

#include <kuniqueapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>
#include <klibloader.h>
#include <kio/global.h>

static void runBuildSycoca()
{
   QStringList args;
   args.append("--incremental");
   KApplication::kdeinitExecWait( "kbuildsycoca", args );
}

static void runKonfUpdate()
{
   KApplication::kdeinitExecWait( "kconf_update");
}

static void runDontChangeHostname(const QCString &oldName, const QCString &newName)
{
   QStringList args;
   args.append(QFile::decodeName(oldName));
   args.append(QFile::decodeName(newName));
   KApplication::kdeinitExecWait( "kdontchangethehostname", args );
}

class KBuildSycoca : public KSycoca
{
public:
   KBuildSycoca() : KSycoca(true) { }

   static KBuildSycoca* createBuildSycoca() 
   { 
      if (_self)
         delete _self;
      return new KBuildSycoca();
   }

   KSycocaFactoryList* factoryList() { return m_lstFactories; }

   /**
    * return true if building
    */
   virtual bool isBuilding() { return true; }
};


Kded::Kded(bool checkUpdates, int pollInterval, int NFSPollInterval)
  : DCOPObject("kbuildsycoca"), DCOPObjectProxy(), 
    b_checkUpdates(checkUpdates),
    m_PollInterval(pollInterval), 
    m_NFSPollInterval(NFSPollInterval)
{
  QString path = KGlobal::dirs()->saveLocation("tmp")+"ksycoca";
  QCString cPath = QFile::encodeName(path);
  m_pTimer = new QTimer(this);
  connect(m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  QTimer::singleShot(100, this, SLOT(installCrashHandler()));

  m_pDirWatch = 0;
  m_pDirWatchNfs = 0;
//  connect(kapp->dcopClient(), SIGNAL(applicationRemoved(const QCString&)),
//          this, SLOT(slotApplicationRemoved(const QCString&)));
//  kapp->dcopClient()->setNotifications(true);
}

Kded::~Kded()
{
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;
  delete m_pDirWatchNfs;
}

bool Kded::process(const QCString &obj, const QCString &fun, 
                   const QByteArray &data,
                   QCString &replyType, QByteArray &replyData)
{
  if (obj == "ksycoca") return false; // Ignore this one.

  KService::Ptr s = KService::serviceByDesktopPath("kded/"+obj+".desktop");
  if (s && !s->library().isEmpty())
  {
    // get the library loader instance
 
    KLibLoader *loader = KLibLoader::self();

    QVariant v = s->property("X-KDE-Factory");
    QString factory = v.isValid() ? v.toString() : QString::null;
    if (factory.isEmpty())
      factory = s->library();

    factory = "create_" + factory;
    QString libname = "libkded_"+s->library(); 

    KLibrary *lib = loader->library(QFile::encodeName(libname));
    if (lib)
    {
      // get the create_ function
      void *create = lib->symbol(QFile::encodeName(factory));

      if (create)
      {
        // create the module
        KDEDModule* (*func)(const QCString &); 
        func = (KDEDModule* (*)(const QCString &)) create;
        KDEDModule *module = func(obj);
        if (module)
        {
          m_modules.append(module);
          connect(module, SIGNAL(destroyed()), SLOT(slotKDEDModuleRemoved()));
          return module->process(fun, data, replyType, replyData);
        }
      }
    }
  }
  return false;
}

QCStringList Kded::functions()
{
    QCStringList res = DCOPObject::functions();
    res += "ASYNC recreate()";
    return res;
}

void Kded::slotKDEDModuleRemoved()
{
  KDEDModule *module = (KDEDModule *) sender();
  m_modules.removeRef(module);
}

void Kded::slotApplicationRemoved(const QCString &appId)
{
  for(KDEDModule *module = m_modules.first();
      module; module = m_modules.next())
  {
     module->removeAll(appId);
  }
}

void Kded::build()
{
  if (!b_checkUpdates) return;

  KBuildSycoca* kbs = KBuildSycoca::createBuildSycoca();

  delete m_pDirWatch;
  delete m_pDirWatchNfs;
  m_pDirWatch = new KDirWatch(m_PollInterval);
  m_pDirWatchNfs = new KDirWatch(m_NFSPollInterval);

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
           this, SLOT(dirDeleted(const QString&)));
  QObject::connect( m_pDirWatchNfs, SIGNAL(dirty(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatchNfs, SIGNAL(deleted(const QString&)),
           this, SLOT(dirDeleted(const QString&)));

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  KBuildServiceGroupFactory *bsgf = new KBuildServiceGroupFactory;
  (void) new KBuildServiceFactory(stf, bsgf);
  // We don't include KImageIOFactory here because it doesn't add
  // new resourceList entries anyway.
  //(void) KImageIOFactory::self();
  // Same for KBuildProtocolInfoFactory

  KSycocaFactoryList *factoryList = kbs->factoryList();

  // For each factory
  QListIterator<KSycocaFactory> factit ( *factoryList );
  for (KSycocaFactory *factory = factoryList->first();
       factory;
       factory = factoryList->first() )
  {
    // For each resource the factory deals with
    for( KSycocaResourceList::ConstIterator it1 = factory->resourceList()->begin();
         it1 != factory->resourceList()->end();
         ++it1 )
    {
      KSycocaResource res = (*it1);
      QStringList dirs = KGlobal::dirs()->resourceDirs( res.resource.ascii() );
      // For each resource the factory deals with
      for( QStringList::ConstIterator it2 = dirs.begin();
           it2 != dirs.end();
           ++it2 )
      {
         if (KIO::probably_slow_mounted(*it2))
         {   
            readDirectory( *it2, m_pDirWatchNfs );
         } else {
            readDirectory( *it2, m_pDirWatch );
         }
      }
    }
    factoryList->removeRef(factory);
  }
  delete kbs;
}

void Kded::crashHandler(int)
{
   DCOPClient::emergencyClose();
   system("kded");
}

void Kded::installCrashHandler()
{
   KCrash::setEmergencySaveFunction(crashHandler);
}

void Kded::recreate()
{
   // Using KLauncher here is difficult since we might not have a
   // database

   build(); // Update tree first, to be sure to miss nothing.

   runBuildSycoca();

   while( !m_requests.isEmpty())
   {
      QCString replyType = "void";
      QByteArray replyData;
      kapp->dcopClient()->endTransaction(m_requests.first(), replyType, replyData);
      m_requests.remove(m_requests.begin());
   }
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

bool Kded::process(const QCString &fun, const QByteArray &data,
                           QCString &replyType, QByteArray &replyData)
{
  if (fun == "recreate()") {
    if (m_requests.isEmpty())
    {
       m_pTimer->start(0, true /* single shot */ );
    }
    m_requests.append(kapp->dcopClient()->beginTransaction());
    replyType = "void";
    return true;
  } else {
    return DCOPObject::process(fun, data, replyType, replyData);
  }
}


void Kded::readDirectory( const QString& _path, KDirWatch *dirWatch )
{
  QString path( _path );
  if ( path.right(1) != "/" )
    path += "/";

  if ( dirWatch->contains( path ) ) // Already seen this one?
     return;

  QDir d( _path, QString::null, QDir::Unsorted, QDir::Readable | QDir::Executable | QDir::Dirs | QDir::Hidden );
  // set QDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  dirWatch->addDir(path);          // add watch on this dir

  if ( !d.exists() )                            // exists&isdir?
  {
    kdDebug(7020) << QString("Does not exist! (%1)").arg(_path) << endl;
    return;                             // return false
  }

  // Note: If some directory is gone, dirwatch will delete it from the list.

  //************************************************************************
  //                               Reading
  //************************************************************************
  QString file;
  unsigned int i;                           // counter and string length.
  unsigned int count = d.count();
  for( i = 0; i < count; i++ )                        // check all entries
  {
     if (d[i] == "." || d[i] == ".." || d[i] == "magic")
       continue;                          // discard those ".", "..", "magic"...

     file = path;                           // set full path
     file += d[i];                          // and add the file name.

     readDirectory( file, dirWatch );      // yes, dive into it.
  }
}

static void sighandler(int /*sig*/)
{
    kapp->quit();
}

KUpdateD::KUpdateD(int pollInterval, int NFSPollInterval) :
    m_PollInterval(pollInterval), 
    m_NFSPollInterval(NFSPollInterval)
{
    m_pDirWatch = new KDirWatch(m_PollInterval);
    m_pDirWatchNfs = new KDirWatch(m_NFSPollInterval);
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(runKonfUpdate()));
    QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(slotNewUpdateFile()));
    QObject::connect( m_pDirWatchNfs, SIGNAL(dirty(const QString&)),
           this, SLOT(slotNewUpdateFile()));

    QStringList dirs = KGlobal::dirs()->findDirs("data", "kconf_update");
    for( QStringList::ConstIterator it = dirs.begin();
         it != dirs.end();
         ++it )
    {
       QString path = *it;
       if (path[path.length()-1] != '/')
          path += "/";
       KDirWatch *dirWatch = KIO::probably_slow_mounted(path) ? m_pDirWatchNfs : m_pDirWatch;

       if (!dirWatch->contains(path))
          dirWatch->addDir(path);
    }
}

KUpdateD::~KUpdateD()
{
    delete m_pDirWatch;
    delete m_pDirWatchNfs;
    delete m_pTimer;
}

void KUpdateD::runKonfUpdate()
{
    ::runKonfUpdate();    
}

void KUpdateD::slotNewUpdateFile()
{
    m_pTimer->start( 500, true /* single shot */ );
}

KHostnameD::KHostnameD(int pollInterval)
{
    m_Timer.start(pollInterval, false /* repetitive */ );
    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(checkHostname()));
    checkHostname();
}

KHostnameD::~KHostnameD()
{
    // Empty
}

void KHostnameD::checkHostname()
{
    char buf[1024+1];
    if (gethostname(buf, 1024) != 0)
       return;

    if (m_hostname.isEmpty())
    {
       m_hostname = buf;
       return;
    }

    if (m_hostname == buf)
       return;

    QCString newHostname = buf;

    runDontChangeHostname(m_hostname, newHostname);
    m_hostname = newHostname;    
}


static KCmdLineOptions options[] =
{
  { "check", I18N_NOOP("Check Sycoca database only once."), 0 },
  { 0, 0, 0 }
};

class KDEDApplication : public KUniqueApplication
{
public:
  KDEDApplication() : KUniqueApplication( ) 
    { startup = true; }

  int newInstance()
    {
       if (startup)
          startup = false;
       else
          runBuildSycoca();
       return 0;
    }
  bool startup;
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
     putenv(strdup("SESSION_MANAGER="));

     // Check DCOP communication.
     {
        DCOPClient testDCOP;
        QCString dcopName = testDCOP.registerAs("kded", false);
        if (dcopName.isEmpty())
        {
           kdFatal() << "DCOP communication problem!" << endl;
           return 1;
        }
     }

     KInstance *instance = new KInstance(&aboutData);
     KConfig *config = instance->config(); // Enable translations.

     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

     if (args->isSet("check"))
     {
        runBuildSycoca();
        runKonfUpdate();
        exit(0);
     }

     if (!KUniqueApplication::start())
     {
        fprintf(stderr, "KDE Daemon (kded) already running.\n");
        exit(0);
     }

     config->setGroup("General");
     int PollInterval = config->readNumEntry("PollInterval", 500);
     int NFSPollInterval = config->readNumEntry("NFSPollInterval", 5000);
     int HostnamePollInterval = config->readNumEntry("HostnamePollInterval", 5000);
     bool bCheckSycoca = config->readBoolEntry("CheckSycoca", true);
     bool bCheckUpdates = config->readBoolEntry("CheckUpdates", true);
     bool bCheckHostname = config->readBoolEntry("CheckHostname", true);

     Kded *kded = new Kded(bCheckUpdates, PollInterval, NFSPollInterval); // Build data base

     kded->recreate();

     signal(SIGTERM, sighandler);
     KDEDApplication k;

     if (bCheckUpdates)
        (void) new KUpdateD(PollInterval, NFSPollInterval); // Watch for updates

     runKonfUpdate(); // Run it once.

     if (bCheckHostname)
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes

     DCOPClient *client = kapp->dcopClient();
     QObject::connect(client, SIGNAL(applicationRemoved(const QCString&)),
             kded, SLOT(slotApplicationRemoved(const QCString&)));
     client->setNotifications(true);
     client->setDaemonMode( true );

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, kbuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)
     QByteArray data;
     client->send( "*", "ksycoca", "notifyDatabaseChanged()", data );
     client->send( "ksplash", "", "upAndRunning(QString)",  QString("kded"));

     return k.exec(); // keep running
}

#include "kded.moc"
