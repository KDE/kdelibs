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

#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kdatastream.h>
#include <kio/global.h>
#include <kservicetype.h>

Kded *Kded::_self = 0;

static bool checkStamps = true;

static void runBuildSycoca()
{
   QStringList args;
   args.append("--incremental");
   if(checkStamps)
      args.append("--checkstamps");
   KApplication::kdeinitExecWait( "kbuildsycoca", args );
   checkStamps = false; // useful only during kded startup
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


Kded::Kded(bool checkUpdates)
  : DCOPObject("kbuildsycoca"), DCOPObjectProxy(), 
    b_checkUpdates(checkUpdates)
{
  _self = this;
  QCString cPath;
  QCString ksycoca_env = getenv("KDESYCOCA");
  if (ksycoca_env.isEmpty())
     cPath = QFile::encodeName(KGlobal::dirs()->saveLocation("tmp")+"ksycoca");
  else
     cPath = ksycoca_env;
  m_pTimer = new QTimer(this);
  connect(m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  QTimer::singleShot(100, this, SLOT(installCrashHandler()));

  m_pDirWatch = 0;
  
  m_windowIdList.setAutoDelete(true);
}

Kded::~Kded()
{
  _self = 0;
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;

  m_modules.setAutoDelete(true);
}

bool Kded::process(const QCString &obj, const QCString &fun, 
                   const QByteArray &data,
                   QCString &replyType, QByteArray &replyData)
{
  if (obj == "ksycoca") return false; // Ignore this one.

  if (m_dontLoad[obj])
     return false;

  KDEDModule *module = loadModule(obj, true);
  if (!module)
     return false; 

  module->setCallingDcopClient(kapp->dcopClient());
  return module->process(fun, data, replyType, replyData);
}

void Kded::noDemandLoad(const QString &obj)
{
  m_dontLoad.insert(obj.latin1(), this);
}

KDEDModule *Kded::loadModule(const QCString &obj, bool onDemand)
{
  KDEDModule *module = m_modules.find(obj);
  if (module)
     return module;
  KService::Ptr s = KService::serviceByDesktopPath("kded/"+obj+".desktop");
  return loadModule(s, onDemand);
}

KDEDModule *Kded::loadModule(const KService *s, bool onDemand)
{
  KDEDModule *module = 0;
  if (s && !s->library().isEmpty())
  {
    if (onDemand)
    {
      QVariant p = s->property("X-KDE-Kded-load-on-demand");
      if (p.isValid() && (p.toBool() == false))
      {
         noDemandLoad(s->desktopEntryName());
         return 0;
      }
    }
    QCString obj = s->desktopEntryName().latin1();
    // get the library loader instance
 
    KLibLoader *loader = KLibLoader::self();

    QVariant v = s->property("X-KDE-Factory");
    QString factory = v.isValid() ? v.toString() : QString::null;
    if (factory.isEmpty())
      factory = s->library();

    factory = "create_" + factory;
    QString libname = "kded_"+s->library(); 

    KLibrary *lib = loader->library(QFile::encodeName(libname));
    if (!lib)
    {
      libname.prepend("lib");
      lib = loader->library(QFile::encodeName(libname));
    }
    if (lib)
    {
      // get the create_ function
      void *create = lib->symbol(QFile::encodeName(factory));

      if (create)
      {
        // create the module
        KDEDModule* (*func)(const QCString &); 
        func = (KDEDModule* (*)(const QCString &)) create;
        module = func(obj);
        if (module)
        {
          m_modules.insert(obj, module);
          m_libs.insert(obj, lib);
          connect(module, SIGNAL(moduleDeleted(KDEDModule *)), SLOT(slotKDEDModuleRemoved(KDEDModule *)));
          kdDebug(7020) << "Successfully loaded module '" << obj << "'\n";
          return module;
        }
      }
      loader->unloadLibrary(QFile::encodeName(libname));
    }
    kdDebug(7020) << "Could not load module '" << obj << "'\n";
  }
  return 0;
}

bool Kded::unloadModule(const QCString &obj)
{
  KDEDModule *module = m_modules.take(obj);
  if (!module)
     return false;
  kdDebug(7020) << "Unloading module '" << obj << "'\n";
  delete module;
  return true;
}

// DCOP
QCStringList Kded::loadedModules()
{
	QCStringList modules;
	QAsciiDictIterator<KDEDModule> it( m_modules );
	for ( ; it.current(); ++it)
		modules.append( it.currentKey() );

	return modules;
}

QCStringList Kded::functions()
{
    QCStringList res = DCOPObject::functions();
    res += "ASYNC recreate()";
    return res;
}

void Kded::slotKDEDModuleRemoved(KDEDModule *module)
{
  m_modules.remove(module->objId());
  KLibrary *lib = m_libs.take(module->objId());
  if (lib)
     lib->unload();
}

void Kded::slotApplicationRemoved(const QCString &appId)
{
  for(QAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
     it.current()->removeAll(appId);
  }

  QValueList<long> *windowIds = m_windowIdList.find(appId);
  if (windowIds)
  {
     for( QValueList<long>::ConstIterator it = windowIds->begin();
          it != windowIds->end(); ++it)
     {
        long windowId = *it;
        m_globalWindowIdList.remove(windowId);
        for(QAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
        {
            emit it.current()->windowUnregistered(windowId);
        }
     }
     m_windowIdList.remove(appId);
  }
}

void Kded::build()
{
  if (!b_checkUpdates) return;

  KBuildSycoca* kbs = KBuildSycoca::createBuildSycoca();

  delete m_pDirWatch;
  m_pDirWatch = new KDirWatch;

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(created(const QString&)),
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
  // Same for KBuildProtocolInfoFactory

  KSycocaFactoryList *factoryList = kbs->factoryList();

  // For each factory
  QPtrListIterator<KSycocaFactory> factit ( *factoryList );
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
	readDirectory( *it2 );
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
qWarning("Last DCOP call before KDED crash was from application '%s'\n"
         "to object '%s', function '%s'.", 
         DCOPClient::postMortemSender(), 
         DCOPClient::postMortemObject(),
         DCOPClient::postMortemFunction());
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


void Kded::readDirectory( const QString& _path )
{
  QString path( _path );
  if ( path.right(1) != "/" )
    path += "/";

  if ( m_pDirWatch->contains( path ) ) // Already seen this one?
     return;

  QDir d( _path, QString::null, QDir::Unsorted, QDir::Readable | QDir::Executable | QDir::Dirs | QDir::Hidden );
  // set QDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  m_pDirWatch->addDir(path);          // add watch on this dir

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

     readDirectory( file );      // yes, dive into it.
  }
}

bool Kded::isWindowRegistered(long windowId)
{
  return m_globalWindowIdList.find(windowId) != 0;

}

// DCOP
void Kded::registerWindowId(long windowId)
{
  m_globalWindowIdList.replace(windowId, &windowId);
  QCString sender = callingDcopClient()->senderId();
  QValueList<long> *windowIds = m_windowIdList.find(sender);
  if (!windowIds)
  {
    windowIds = new QValueList<long>;
    m_windowIdList.insert(sender, windowIds);
  }
  windowIds->append(windowId);


  for(QAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
     emit it.current()->windowRegistered(windowId);
  }
}

// DCOP
void Kded::unregisterWindowId(long windowId)
{
  m_globalWindowIdList.remove(windowId);
  QCString sender = callingDcopClient()->senderId();
  QValueList<long> *windowIds = m_windowIdList.find(sender);
  if (windowIds)
  {
     windowIds->remove(windowId);
     if (windowIds->isEmpty())
        m_windowIdList.remove(sender);
  }

  for(QAsciiDictIterator<KDEDModule> it(m_modules); it.current(); ++it)
  {
    emit it.current()->windowUnregistered(windowId);
  }
}


static void sighandler(int /*sig*/)
{
    if (kapp)
       kapp->quit();
}

KUpdateD::KUpdateD()
{
    m_pDirWatch = new KDirWatch;
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(runKonfUpdate()));
    QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(slotNewUpdateFile()));

    QStringList dirs = KGlobal::dirs()->findDirs("data", "kconf_update");
    for( QStringList::ConstIterator it = dirs.begin();
         it != dirs.end();
         ++it )
    {
       QString path = *it;
       if (path[path.length()-1] != '/')
          path += "/";

       if (!m_pDirWatch->contains(path))
          m_pDirWatch->addDir(path);
    }
}

KUpdateD::~KUpdateD()
{
    delete m_pDirWatch;
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

  QCStringList functions()
    {
       QCStringList res = KUniqueApplication::functions();
       res += "bool loadModule(QCString)";
       res += "bool unloadModule(QCString)";
       res += "void registerWindowId(long int)";
       res += "void unregisterWindowId(long int)";
       res += "QCStringList loadedModules()";
       return res;
    }

  bool process(const QCString &fun, const QByteArray &data,
               QCString &replyType, QByteArray &replyData)
  {
    if (fun == "loadModule(QCString)") {
      QCString module;
      QDataStream arg( data, IO_ReadOnly );
      arg >> module;
      bool result = (Kded::self()->loadModule(module, false) != 0);
      replyType = "bool";
      QDataStream _replyStream( replyData, IO_WriteOnly );
      _replyStream << result;
      return true;
    } 
    else if (fun == "unloadModule(QCString)") {
      QCString module;
      QDataStream arg( data, IO_ReadOnly );
      arg >> module;
      bool result = Kded::self()->unloadModule(module);
      replyType = "bool";
      QDataStream _replyStream( replyData, IO_WriteOnly );
      _replyStream << result;
      return true;
    }
    else if (fun == "registerWindowId(long int)") {
      long windowId;
      QDataStream arg( data, IO_ReadOnly );
      arg >> windowId;
      Kded::self()->setCallingDcopClient(callingDcopClient());
      Kded::self()->registerWindowId(windowId);
      return true;
    }
     else if (fun == "unregisterWindowId(long int)") {
      long windowId;
      QDataStream arg( data, IO_ReadOnly );
      arg >> windowId;
      Kded::self()->setCallingDcopClient(callingDcopClient());
      Kded::self()->unregisterWindowId(windowId);
      return true;
    }
    else if (fun == "loadedModules()") {
      replyType = "QCStringList";
      QDataStream _replyStream(replyData, IO_WriteOnly);
      _replyStream << Kded::self()->loadedModules();
      return true;
    }
    return KUniqueApplication::process(fun, data, replyType, replyData);
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
     
     // Parse command line before checking DCOP
     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

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

     if (args->isSet("check"))
     {
        config->setGroup("General");
        checkStamps = config->readBoolEntry("CheckFileStamps", true);
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
     int HostnamePollInterval = config->readNumEntry("HostnamePollInterval", 5000);
     bool bCheckSycoca = config->readBoolEntry("CheckSycoca", true);
     bool bCheckUpdates = config->readBoolEntry("CheckUpdates", true);
     bool bCheckHostname = config->readBoolEntry("CheckHostname", true);
     checkStamps = config->readBoolEntry("CheckFileStamps", true);

     Kded *kded = new Kded(bCheckSycoca); // Build data base

     kded->recreate();

     signal(SIGTERM, sighandler);
     signal(SIGHUP, sighandler);
     KDEDApplication k;

     if (bCheckUpdates)
        (void) new KUpdateD; // Watch for updates

     runKonfUpdate(); // Run it once.

     if (bCheckHostname)
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes

     DCOPClient *client = kapp->dcopClient();
     QObject::connect(client, SIGNAL(applicationRemoved(const QCString&)),
             kded, SLOT(slotApplicationRemoved(const QCString&)));
     client->setNotifications(true);
     client->setDaemonMode( true );

     // Preload kded modules.
     KService::List kdedModules = KServiceType::offers("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
	     if (!service->property( "X-KDE-Kded-nostart").toBool())
         {
            bool autoload = service->property("X-KDE-Kded-autoload").toBool();
            config->setGroup(QString("Module-%1").arg(service->desktopEntryName()));
            autoload = config->readBoolEntry("autoload", autoload);
            if (autoload)
               kded->loadModule(service, false);

            bool dontLoad = false;
            QVariant p = service->property("X-KDE-Kded-load-on-demand");
            if (p.isValid() && (p.toBool() == false))
               dontLoad = true;
            if (dontLoad)
               kded->noDemandLoad(service->desktopEntryName());
         }
     }

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, kbuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)
     QByteArray data;
     client->send( "*", "ksycoca", "notifyDatabaseChanged()", data );
     client->send( "ksplash", "", "upAndRunning(QString)",  QString("kded"));

     int result = k.exec(); // keep running
       
     delete kded;
     delete instance; // Deletes config as well
       
     return result;
}

#include "kded.moc"
