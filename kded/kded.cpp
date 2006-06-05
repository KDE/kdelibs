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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <qdir.h>

#include "kded.h"
#include "kdedmodule.h"

#include <kresourcelist.h>
#include <kcrash.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <qfile.h>
#include <qtimer.h>

#include <dbus/qdbus.h>

#include <kuniqueapplication.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kservicetypetrader.h>
#include <ktoolinvocation.h>
#include "klauncher_iface.h"

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#define MODULES_PATH "/modules/"

Kded *Kded::_self = 0;

static bool checkStamps = true;
static bool delayedCheck = false;

extern QDBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage&));

static void runBuildSycoca(QObject *callBackObj=0, const char *callBackSlot=0)
{
   QStringList args;
   args.append("--incremental");
   if(checkStamps)
      args.append("--checkstamps");
   if(delayedCheck)
      args.append("--nocheckfiles");
   else
      checkStamps = false; // useful only during kded startup
   if (callBackObj)
   {
      QVariantList args;
      args << QString("kbuildsycoca") << args;
      KToolInvocation::klauncher()->callWithArgs("kdeinit_exec_wait", callBackObj,
                                                 callBackSlot, args);
   }
   else
   {
      KToolInvocation::kdeinitExecWait( "kbuildsycoca", args );
   }
}

static void runKonfUpdate()
{
   KToolInvocation::kdeinitExecWait( "kconf_update", QStringList(), 0, 0, "0" /*no startup notification*/ );
}

static void runDontChangeHostname(const QByteArray &oldName, const QByteArray &newName)
{
   QStringList args;
   args.append(QFile::decodeName(oldName));
   args.append(QFile::decodeName(newName));
   KToolInvocation::kdeinitExecWait( "kdontchangethehostname", args );
}

Kded::Kded(bool checkUpdates)
  : b_checkUpdates(checkUpdates),
    m_needDelayedCheck(false)
{
  _self = this;

  new KBuildsycocaAdaptor(this);
  new KdedAdaptor(this);

  QDBusConnection &session = QDBus::sessionBus();
  session.registerObject(MODULES_PATH "kbuildsycoca", this);
  session.registerObject(MODULES_PATH "kded", this);

  qDBusAddSpyHook(messageFilter);

  m_pTimer = new QTimer(this);
  m_pTimer->setSingleShot( true );
  connect(m_pTimer, SIGNAL(timeout()), this, SLOT(recreate()));

  m_pDirWatch = 0;

  m_recreateCount = 0;
  m_recreateBusy = false;
}

Kded::~Kded()
{
  _self = 0;
  m_pTimer->stop();
  delete m_pTimer;
  delete m_pDirWatch;

  for (QHash<QByteArray,KDEDModule*>::iterator
           it(m_modules.begin()), itEnd(m_modules.end());
       it != itEnd; ++it)
  {
      KDEDModule* module(it.value());

      // first disconnect otherwise slotKDEDModuleRemoved() is called
      // and changes m_modules while we're iterating over it
      disconnect(module, SIGNAL(moduleDeleted(KDEDModule*)),
                 this, SLOT(slotKDEDModuleRemoved(KDEDModule*)));

      delete module;
  }
}

// on-demand module loading
// this function is called by the D-Bus message processing function before
// calls are delivered to objects
void Kded::messageFilter(const QDBusMessage &message)
{
  if (message.type() != QDBusMessage::MethodCallMessage)
     return;

  QString obj = message.path();
  if (!obj.startsWith(MODULES_PATH))
     return;

  obj = obj.mid(strlen(MODULES_PATH));
  if (obj == "ksycoca")
     return; // Ignore this one.

  if (self()->m_dontLoad.value(obj, 0))
     return;

  KDEDModule *module = self()->loadModule(obj, true);
  Q_UNUSED(module);
}

void Kded::initModules()
{
     m_dontLoad.clear();
     KConfig *config = KGlobal::config();
     bool kde_running = !( getenv( "KDE_FULL_SESSION" ) == NULL || getenv( "KDE_FULL_SESSION" )[ 0 ] == '\0' );

     // Preload kded modules.
     KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-KDE-Kded-autoload", QVariant::Bool).toBool();
         config->setGroup(QString("Module-%1").arg(service->desktopEntryName()));
         autoload = config->readEntry("autoload", autoload);
         // see ksmserver's README for description of the phases
         QVariant phasev = service->property("X-KDE-Kded-phase", QVariant::Int );
         int phase = phasev.isValid() ? phasev.toInt() : 2;
         bool prevent_autoload = false;
         switch( phase )
         {
             case 0: // always autoload
                 break;
             case 1: // autoload only in KDE
                 if( !kde_running )
                     prevent_autoload = true;
                 break;
             case 2: // autoload delayed, only in KDE
             default:
                 prevent_autoload = true;
                 break;
         }
         if (autoload && !prevent_autoload)
            loadModule(service, false);

         bool dontLoad = false;
         QVariant p = service->property("X-KDE-Kded-load-on-demand", QVariant::Bool);
         if (p.isValid() && (p.toBool() == false))
            dontLoad = true;
         if (dontLoad)
            noDemandLoad(service->desktopEntryName());

         if (dontLoad && !autoload)
            unloadModule(service->desktopEntryName().toLatin1());
     }
}

void Kded::loadSecondPhase()
{
     kDebug(7020) << "Loading second phase autoload" << endl;
     KConfig *config = KGlobal::config();
     KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-KDE-Kded-autoload", QVariant::Bool).toBool();
         config->setGroup(QString("Module-%1").arg(service->desktopEntryName()));
         autoload = config->readEntry("autoload", autoload);
         QVariant phasev = service->property("X-KDE-Kded-phase", QVariant::Int );
         int phase = phasev.isValid() ? phasev.toInt() : 2;
         if( phase == 2 && autoload )
            loadModule(service, false);
     }
}

void Kded::noDemandLoad(const QString &obj)
{
  m_dontLoad.insert(obj.toLatin1(), this);
}

KDEDModule *Kded::loadModule(const QString &obj, bool onDemand)
{
  KDEDModule *module = m_modules.value(obj, 0);
  if (module)
     return module;
  KService::Ptr s = KService::serviceByDesktopPath("kded/"+obj+".desktop");
  return loadModule(s, onDemand);
}

KDEDModule *Kded::loadModule(const KService::Ptr& s, bool onDemand)
{
  KDEDModule *module = 0;
  if (s && !s->library().isEmpty())
  {
    QString obj = s->desktopEntryName();
    KDEDModule *oldModule = m_modules.value(obj, 0);
    if (oldModule)
       return oldModule;

    if (onDemand)
    {
      QVariant p = s->property("X-KDE-Kded-load-on-demand", QVariant::Bool);
      if (p.isValid() && (p.toBool() == false))
      {
         noDemandLoad(s->desktopEntryName());
         return 0;
      }
    }
    // get the library loader instance

    KLibLoader *loader = KLibLoader::self();

    QVariant v = s->property("X-KDE-FactoryName", QVariant::String);
    QString factory = v.isValid() ? v.toString() : QString();
    if (factory.isEmpty())
    {
       // Stay bugward compatible
       v = s->property("X-KDE-Factory", QVariant::String);
       factory = v.isValid() ? v.toString() : QString();
    }
    if (factory.isEmpty())
      factory = s->library();

    factory = "create_" + factory;
    QString libname = "kded_"+s->library();

    KLibrary *lib = loader->library(QFile::encodeName(libname));
    if (!lib)
    {
      kWarning() << k_funcinfo << "Could not load library. [ "
                 << loader->lastErrorMessage() << " ]" << endl;
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
        KDEDModule* (*func)(const QString &);
        func = (KDEDModule* (*)(const QString &)) create;
        module = func(obj);
        if (module)
        {
          m_modules.insert(obj, module);
          m_libs.insert(obj, lib);
          connect(module, SIGNAL(moduleDeleted(KDEDModule *)), SLOT(slotKDEDModuleRemoved(KDEDModule *)));
          kDebug(7020) << "Successfully loaded module '" << obj << "'\n";
          return module;
        }
      }
      loader->unloadLibrary(QFile::encodeName(libname));
    }
    else
    {
       kWarning() << k_funcinfo << "Could not load library. [ "
                   << loader->lastErrorMessage() << " ]" << endl;
    }
    kDebug(7020) << "Could not load module '" << obj << "'\n";
  }
  return 0;
}

bool Kded::unloadModule(const QString &obj)
{
  KDEDModule *module = m_modules.value(obj, 0);
  if (!module)
     return false;
  kDebug(7020) << "Unloading module '" << obj << "'\n";
  m_modules.remove(obj);
  delete module;
  return true;
}

QStringList Kded::loadedModules()
{
    return m_modules.keys();
}

void Kded::slotKDEDModuleRemoved(KDEDModule *module)
{
  m_modules.remove(module->moduleName());
  KLibrary *lib = m_libs.take(module->moduleName());
  if (lib)
     lib->unload();
}

void Kded::slotApplicationRemoved(const QString &name, const QString &oldOwner,
                                  const QString &)
{
#if 0 // see kdedmodule.cpp (KDED_OBJECTS)
  foreach( KDEDModule* module, m_modules )
  {
     module->removeAll(appId);
  }
#endif
  if (oldOwner.isEmpty() || !QDBusUtil::isValidUniqueConnectionName(name))
     return;

  const QList<qlonglong> windowIds = m_windowIdList.value(name);
  for( QList<qlonglong>::ConstIterator it = windowIds.begin();
       it != windowIds.end(); ++it)
  {
      qlonglong windowId = *it;
      m_globalWindowIdList.remove(windowId);
      foreach( KDEDModule* module, m_modules )
      {
          emit module->windowUnregistered(windowId);
      }
  }
  m_windowIdList.remove(name);
}

void Kded::updateDirWatch()
{
  if (!b_checkUpdates) return;

  delete m_pDirWatch;
  m_pDirWatch = new KDirWatch;

  QObject::connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(created(const QString&)),
           this, SLOT(update(const QString&)));
  QObject::connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
           this, SLOT(dirDeleted(const QString&)));

  // For each resource
  for( QStringList::ConstIterator it = m_allResourceDirs.begin();
       it != m_allResourceDirs.end();
       ++it )
  {
     readDirectory( *it );
  }
}

void Kded::updateResourceList()
{
  delete KSycoca::self();

  if (!b_checkUpdates) return;

  if (delayedCheck) return;

  const QStringList dirs = KSycoca::self()->allResourceDirs();
  // For each resource
  for( QStringList::ConstIterator it = dirs.begin();
       it != dirs.end();
       ++it )
  {
     if (!m_allResourceDirs.contains(*it))
     {
        m_allResourceDirs.append(*it);
        readDirectory(*it);
     }
  }
}

void Kded::crashHandler(int)
{
   if (_self) // Don't restart if we were closing down
      system("kded");
}

void Kded::installCrashHandler()
{
   KCrash::setEmergencySaveFunction(crashHandler);
}

void Kded::recreate()
{
   recreate(false);
}

void Kded::runDelayedCheck()
{
   if( m_needDelayedCheck )
      recreate(false);
   m_needDelayedCheck = false;
}

void Kded::recreate(bool initial)
{
   m_recreateBusy = true;
   // Using KLauncher here is difficult since we might not have a
   // database

   if (!initial)
   {
      updateDirWatch(); // Update tree first, to be sure to miss nothing.
      runBuildSycoca(this, SLOT(recreateDone()));
   }
   else
   {
      if(!delayedCheck)
         updateDirWatch(); // this would search all the directories
      runBuildSycoca();
      recreateDone();
      if(delayedCheck)
      {
         // do a proper ksycoca check after a delay
         QTimer::singleShot( 60000, this, SLOT(runDelayedCheck()));
         m_needDelayedCheck = true;
         delayedCheck = false;
      }
      else
         m_needDelayedCheck = false;
      QTimer::singleShot(100, this, SLOT(installCrashHandler()));
   }
}

void Kded::recreateDone()
{
   updateResourceList();

   for(; m_recreateCount; m_recreateCount--)
   {
      QDBusMessage reply = m_recreateRequests.takeFirst();
      reply.connection().send(reply);
   }
   m_recreateBusy = false;

   // Did a new request come in while building?
   if (!m_recreateRequests.isEmpty())
   {
      m_pTimer->start(2000);
      m_recreateCount = m_recreateRequests.count();
   }
}

void Kded::dirDeleted(const QString& path)
{
  update(path);
}

void Kded::update(const QString& )
{
  if (!m_recreateBusy)
  {
    m_pTimer->start( 2000 );
  }
}

void Kded::recreate(const QDBusMessage &msg)
{
   if (!m_recreateBusy)
   {
      if (m_recreateRequests.isEmpty())
      {
         m_pTimer->start(0);
         m_recreateCount = 0;
      }
      m_recreateCount++;
   }
   m_recreateRequests.append(QDBusMessage::methodReply(msg));
   return;
}


void Kded::readDirectory( const QString& _path )
{
  QString path( _path );
  if ( path.right(1) != "/" )
    path += '/';

  if ( m_pDirWatch->contains( path ) ) // Already seen this one?
     return;

  QDir d( _path, QString(), QDir::Unsorted, QDir::Readable | QDir::Executable | QDir::Dirs | QDir::Hidden );
  // set QDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  m_pDirWatch->addDir(path);          // add watch on this dir

  if ( !d.exists() )                            // exists&isdir?
  {
    kDebug(7020) << QString("Does not exist! (%1)").arg(_path) << endl;
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

/*
bool Kded::isWindowRegistered(long windowId) const
{
  return m_globalWindowIdList.contains(windowId);

}
*/

void Kded::registerWindowId(qlonglong windowId, const QString &sender)
{
  m_globalWindowIdList.insert(windowId);
  QList<qlonglong> windowIds = m_windowIdList.value(sender);
  windowIds.append(windowId);
  m_windowIdList.insert(sender, windowIds);

  foreach( KDEDModule* module, m_modules )
  {
     emit module->windowRegistered(windowId);
  }
}

void Kded::unregisterWindowId(qlonglong windowId, const QString &sender)
{
  m_globalWindowIdList.remove(windowId);
  QList<qlonglong> windowIds = m_windowIdList.value(sender);
  if (!windowIds.isEmpty())
  {
     windowIds.removeAll(windowId);
     if (windowIds.isEmpty())
        m_windowIdList.remove(sender);
     else
        m_windowIdList.insert(sender, windowIds);
  }

  foreach( KDEDModule* module, m_modules )
  {
    emit module->windowUnregistered(windowId);
  }
}


static void sighandler(int /*sig*/)
{
    if (kapp)
       qApp->quit();
}

KUpdateD::KUpdateD()
{
    m_pDirWatch = new KDirWatch;
    m_pTimer = new QTimer;
    m_pTimer->setSingleShot( true );
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
          path += '/';

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
    m_pTimer->start( 500 );
}

KHostnameD::KHostnameD(int pollInterval)
{
    m_Timer.start(pollInterval); // repetitive timer (not single-shot)
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
    buf[sizeof(buf)-1] = '\0';

    if (m_hostname.isEmpty())
    {
       m_hostname = buf;
       return;
    }

    if (m_hostname == buf)
       return;

    QByteArray newHostname = buf;

    runDontChangeHostname(m_hostname, newHostname);
    m_hostname = newHostname;
}


static KCmdLineOptions options[] =
{
  { "check", I18N_NOOP("Check Sycoca database only once"), 0 },
  KCmdLineLastOption
};

#ifdef __GNUC__
# warning "I have no idea what the following class is here for"
#elif 0
class KDEDQtDCOPObject : public DCOPObject
{
public:
  KDEDQtDCOPObject() : DCOPObject("qt/kded") { }

  virtual bool process(const DCOPCString &fun, const QByteArray &data,
                       DCOPCString& replyType, QByteArray &replyData)
    {
      if ( kapp && (fun == "quit()") )
      {
        qApp->quit();
        replyType = "void";
        return true;
      }
      return DCOPObject::process(fun, data, replyType, replyData);
    }

  DCOPCStringList functions()
    {
       DCOPCStringList res = DCOPObject::functions();
       res += "void quit()";
       return res;
    }
};
#endif

KBuildsycocaAdaptor::KBuildsycocaAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
}

void KBuildsycocaAdaptor::recreate(const QDBusMessage &msg)
{
   Kded::self()->recreate(msg);
}

KdedAdaptor::KdedAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
}

bool KdedAdaptor::loadModule(const QString &module)
{
   return Kded::self()->loadModule(module, false) != 0;
}

bool KdedAdaptor::unloadModule(const QString &module)
{
   return Kded::self()->unloadModule(module);
}

void KdedAdaptor::registerWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->registerWindowId(windowId, msg.sender());
}

void KdedAdaptor::unregisterWindowId(qlonglong windowId, const QDBusMessage &msg)
{
   Kded::self()->unregisterWindowId(windowId, msg.sender());
}

QStringList KdedAdaptor::loadedModules()
{
   return Kded::self()->loadedModules();
}

void KdedAdaptor::reconfigure()
{
   KGlobal::config()->reparseConfiguration();
   Kded::self()->initModules();
}

void KdedAdaptor::loadSecondPhase()
{
   Kded::self()->loadSecondPhase();
}

void KdedAdaptor::quit()
{
   QCoreApplication::instance()->quit();
}

class KDEDApplication : public KUniqueApplication
{
public:
  KDEDApplication() : KUniqueApplication( )
    {
       startup = true;
    }

  int newInstance()
    {
       if (startup) {
          startup = false;
          Kded::self()->initModules();
       } else
          runBuildSycoca();

       return 0;
    }

  bool startup;
};

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
     KAboutData aboutData( "kded", I18N_NOOP("KDE Daemon"),
        "$Id$",
        I18N_NOOP("KDE Daemon - triggers Sycoca database updates when needed"));

     KCmdLineArgs::init(argc, argv, &aboutData);

     KUniqueApplication::addCmdLineOptions();

     KCmdLineArgs::addCmdLineOptions( options );

     // this program is in kdelibs so it uses kdelibs as catalog
     KLocale::setMainCatalog("kdelibs");

     // WABA: Make sure not to enable session management.
     putenv(strdup("SESSION_MANAGER="));

     // Parse command line before checking DCOP
     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

     // Check DCOP communication.
     {
        QDBusBusService *bus = QDBus::sessionBus().busService();
        if (!QDBus::sessionBus().isConnected() || !bus->isValid() ||
            bus->requestName("org.kde.kded", QDBusBusService::DoNotQueueName) !=
            QDBusBusService::PrimaryOwnerReply)
        {
           kFatal() << "D-Bus communication problem!" << endl;
           return 1;
        }
     }

     KInstance *instance = new KInstance(&aboutData);
     KConfig *config = instance->config(); // Enable translations.

     if (args->isSet("check"))
     {
        config->setGroup("General");
        checkStamps = config->readEntry("CheckFileStamps", true);
        runBuildSycoca();
        runKonfUpdate();
        exit(0);
     }

     if (!KUniqueApplication::start())
     {
        fprintf(stderr, "KDE Daemon (kded) already running.\n");
        exit(0);
     }

     // Thiago: reenable if such a thing exists in QtDBus in the future
     //KUniqueApplication::dcopClient()->setQtBridgeEnabled(false);

     config->setGroup("General");
     int HostnamePollInterval = config->readEntry("HostnamePollInterval", 5000);
     bool bCheckSycoca = config->readEntry("CheckSycoca", true);
     bool bCheckUpdates = config->readEntry("CheckUpdates", true);
     bool bCheckHostname = config->readEntry("CheckHostname", true);
     checkStamps = config->readEntry("CheckFileStamps", true);
     delayedCheck = config->readEntry("DelayedCheck", false);

     Kded *kded = new Kded(bCheckSycoca); // Build data base

     signal(SIGTERM, sighandler);
     signal(SIGHUP, sighandler);
     KDEDApplication k;
     k.setQuitOnLastWindowClosed(false);

     // Not sure why kded is created before KDEDApplication
     // but if it has to be, then it needsto be moved to the main thread
     // before it can use timers (DF)
     kded->moveToThread( k.thread() );

     kded->recreate(true); // initial

     if (bCheckUpdates)
        (void) new KUpdateD; // Watch for updates

     runKonfUpdate(); // Run it once.

     if (bCheckHostname)
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes

     QObject::connect(QDBus::sessionBus().busService(),
                      SIGNAL(nameOwnerChanged(QString,QString,QString)),
                      kded, SLOT(slotApplicationRemoved(QString,QString,QString)));

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, kbuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)

     // FIXME: rename the signal
     QDBusMessage msg = QDBusMessage::signal(MODULES_PATH "kbuildsycoca", "org.kde.ksycoca",
                                             "notifyDatabaseChanged");
     QDBus::sessionBus().send(msg);

     QDBusInterfacePtr("org.kde.ksplash", "/")->
        call(QDBusInterface::NoWaitForReply, "upAndRunning", QString("kded"));
#ifdef Q_WS_X11
     XEvent e;
     e.xclient.type = ClientMessage;
     e.xclient.message_type = XInternAtom( QX11Info::display(), "_KDE_SPLASH_PROGRESS", False );
     e.xclient.display = QX11Info::display();
     e.xclient.window = QX11Info::appRootWindow();
     e.xclient.format = 8;
     strcpy( e.xclient.data.b, "kded" );
     XSendEvent( QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e );
#endif
     int result = k.exec(); // keep running

     delete kded;
     delete instance; // Deletes config as well

     return result;
}

#include "kded.moc"
