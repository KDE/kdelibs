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

#include "kded.h"
#include "kdedadaptor.h"
#include "kdedmodule.h"

#include "kresourcelist.h"
#include <kcrash.h>

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <QtDBus/QtDBus>

#include <kuniqueapplication.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <ktoolinvocation.h>
#include <kde_file.h>
#include "klauncher_iface.h"

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#define KDED_EXENAME "kded4"

#define MODULES_PATH "/modules/"

Kded *Kded::_self = 0;

static bool checkStamps = true;
static bool delayedCheck = false;

extern QDBUS_EXPORT void qDBusAddSpyHook(void (*)(const QDBusMessage&));

static void runBuildSycoca(QObject *callBackObj=0, const char *callBackSlot=0)
{
   const QString exe = KStandardDirs::findExe(KBUILDSYCOCA_EXENAME);
   Q_ASSERT(!exe.isEmpty());
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
      QVariantList argList;
      argList << exe << args << QStringList() << QString();
      KToolInvocation::klauncher()->callWithCallback("kdeinit_exec_wait", argList, callBackObj,
                                                         callBackSlot);
   }
   else
   {
      KToolInvocation::kdeinitExecWait( exe, args );
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

  QDBusConnection session = QDBusConnection::sessionBus();
  session.registerObject("/kbuildsycoca", this);
  session.registerObject("/kded", this);

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

  // This happens when kded goes down and some modules try to clean up.
  if (!self())
     return;

  if (self()->m_dontLoad.value(obj, 0))
     return;

  KDEDModule *module = self()->loadModule(obj, true);
  Q_UNUSED(module);
}

void Kded::initModules()
{
     m_dontLoad.clear();
     KSharedConfig::Ptr config = KGlobal::config();
     bool kde_running = !qgetenv( "KDE_FULL_SESSION" ).isEmpty();
    // not the same user like the one running the session (most likely we're run via sudo or something)
    const QByteArray sessionUID = qgetenv( "KDE_SESSION_UID" );
    if( !sessionUID.isEmpty() && uid_t( sessionUID.toInt() ) != getuid())
        kde_running = false;
     // Preload kded modules.
     const KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.begin(); it != kdedModules.end(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-KDE-Kded-autoload", QVariant::Bool).toBool();
         KConfigGroup cg(config, QString("Module-%1").arg(service->desktopEntryName()));
         autoload = cg.readEntry("autoload", autoload);
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
     kDebug(7020) << "Loading second phase autoload";
     KSharedConfig::Ptr config = KGlobal::config();
     KService::List kdedModules = KServiceTypeTrader::self()->query("KDEDModule");
     for(KService::List::ConstIterator it = kdedModules.constBegin(); it != kdedModules.constEnd(); ++it)
     {
         KService::Ptr service = *it;
         bool autoload = service->property("X-KDE-Kded-autoload", QVariant::Bool).toBool();
         KConfigGroup cg(config, QString("Module-%1").arg(service->desktopEntryName()));
         autoload = cg.readEntry("autoload", autoload);
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

        KDEDModule *module = 0;
        QString libname = "kded_"+s->library();
        KPluginLoader loader(libname);

        KPluginFactory *factory = loader.factory();
        if (!factory) {
            // kde3 compat
            QString factoryName = s->property("X-KDE-FactoryName", QVariant::String).toString();
            if (factoryName.isEmpty())
                factoryName = s->library();
            factoryName = "create_" + factoryName;
            KLibrary* lib = KLibLoader::self()->library(libname);
            KDEDModule* (*create)();
            if (lib) {
                create = (KDEDModule* (*)())lib->resolveFunction(QFile::encodeName(factoryName));
                if (create)
                    module = create();
            }
            if (!module) {
                kWarning() << "Could not load library" << libname << ". ["
                           << loader.errorString() << "]";
            }
        } else {
            // create the module
            module = factory->create<KDEDModule>(this);
        }
        if (module) {
            module->setModuleName(obj);
            m_modules.insert(obj, module);
            //m_libs.insert(obj, lib);
            connect(module, SIGNAL(moduleDeleted(KDEDModule *)), SLOT(slotKDEDModuleRemoved(KDEDModule *)));
            kDebug(7020) << "Successfully loaded module" << obj;
            return module;
        } else {
            kDebug(7020) << "Could not load module" << obj;
            //loader.unload();
        }
    }
    return 0;
}

bool Kded::unloadModule(const QString &obj)
{
  KDEDModule *module = m_modules.value(obj, 0);
  if (!module)
     return false;
  kDebug(7020) << "Unloading module" << obj;
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
  //KLibrary *lib = m_libs.take(module->moduleName());
  //if (lib)
  //   lib->unload();
}

void Kded::slotApplicationRemoved(const QString &name, const QString &oldOwner,
                                  const QString &newOwner)
{
#if 0 // see kdedmodule.cpp (KDED_OBJECTS)
  foreach( KDEDModule* module, m_modules )
  {
     module->removeAll(appId);
  }
#endif
  if (oldOwner.isEmpty() || !newOwner.isEmpty())
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
  for( QStringList::ConstIterator it = m_allResourceDirs.constBegin();
       it != m_allResourceDirs.constEnd();
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
   }
}

void Kded::recreateDone()
{
   updateResourceList();

   for(; m_recreateCount; m_recreateCount--)
   {
      QDBusMessage msg = m_recreateRequests.takeFirst();
      QDBusConnection::sessionBus().send(msg.createReply());
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
    m_pTimer->start( 10000 );
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
   msg.setDelayedReply(true);
   m_recreateRequests.append(msg);
   return;
}


void Kded::readDirectory( const QString& _path )
{
  QString path( _path );
  if ( !path.endsWith( '/' ) )
    path += '/';

  if ( m_pDirWatch->contains( path ) ) // Already seen this one?
     return;

  m_pDirWatch->addDir(path,KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);          // add watch on this dir
  return; // KDirWatch now claims to also support recursive watching
#if 0
  QDir d( _path, QString(), QDir::Unsorted, QDir::Readable | QDir::Executable | QDir::Dirs | QDir::Hidden );
  // set QDir ...


  //************************************************************************
  //                           Setting dirs
  //************************************************************************

  if ( !d.exists() )                            // exists&isdir?
  {
    kDebug(7020) << "Does not exist:" << _path;
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
#endif
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
    if (qApp)
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

    const QStringList dirs = KGlobal::dirs()->findDirs("data", "kconf_update");
    for( QStringList::ConstIterator it = dirs.begin();
         it != dirs.end();
         ++it )
    {
       QString path = *it;
       if (path[path.length()-1] != '/')
          path += '/';

       if (!m_pDirWatch->contains(path))
          m_pDirWatch->addDir(path,KDirWatch::WatchFiles|KDirWatch::WatchSubDirs);
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


#if 0
// Thiago: I have no idea what the following class is here for
// David: the commit log was:
//       Disable DCOP-Qt bridge but make sure that "dcopquit kded" continues to work.
//       (see the setQtBridgeEnabled below)
class KDEDQtDCOPObject : public DCOPObject
{
public:
  KDEDQtDCOPObject() : DCOPObject("qt/kded") { }

  virtual bool process(const DCOPCString &fun, const QByteArray &data,
                       DCOPCString& replyType, QByteArray &replyData)
    {
      if (qApp && (fun == "quit()"))
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
     KAboutData aboutData( "kded" /*don't change this one to kded4! dbus registration should be org.kde.kded etc.*/,
        "kdelibs4", ki18n("KDE Daemon"),
        "$Id$",
        ki18n("KDE Daemon - triggers Sycoca database updates when needed"));

     KCmdLineOptions options;
     options.add("check", ki18n("Check Sycoca database only once"));

     KCmdLineArgs::init(argc, argv, &aboutData);

     KUniqueApplication::addCmdLineOptions();

     KCmdLineArgs::addCmdLineOptions( options );

     // WABA: Make sure not to enable session management.
     putenv(strdup("SESSION_MANAGER="));

     // Parse command line before checking DCOP
     KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

     KComponentData componentData(&aboutData);
     KSharedConfig::Ptr config = componentData.config(); // Enable translations.

     KConfigGroup cg(config, "General");
     if (args->isSet("check"))
     {
        // KUniqueApplication not wanted here.
        KApplication app;
        checkStamps = cg.readEntry("CheckFileStamps", true);
        runBuildSycoca();
        runKonfUpdate();
        return 0;
     }

     if (!KUniqueApplication::start())
     {
        fprintf(stderr, "KDE Daemon (kded) already running.\n");
        return 0;
     }

     // Thiago: reenable if such a thing exists in QtDBus in the future
     //KUniqueApplication::dcopClient()->setQtBridgeEnabled(false);

     int HostnamePollInterval = cg.readEntry("HostnamePollInterval", 5000);
     bool bCheckSycoca = cg.readEntry("CheckSycoca", true);
     bool bCheckUpdates = cg.readEntry("CheckUpdates", true);
     bool bCheckHostname = cg.readEntry("CheckHostname", true);
     checkStamps = cg.readEntry("CheckFileStamps", true);
     delayedCheck = cg.readEntry("DelayedCheck", false);

     Kded *kded = new Kded(bCheckSycoca); // Build data base

     KDE_signal(SIGTERM, sighandler);
     KDE_signal(SIGHUP, sighandler);
     KDEDApplication k;
     k.setQuitOnLastWindowClosed(false);

     KCrash::setFlags(KCrash::AutoRestart);

     // Not sure why kded is created before KDEDApplication
     // but if it has to be, then it needs to be moved to the main thread
     // before it can use timers (DF)
     kded->moveToThread( k.thread() );

     kded->recreate(true); // initial

     if (bCheckUpdates)
        (void) new KUpdateD; // Watch for updates

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

     runKonfUpdate(); // Run it once.

#ifdef Q_WS_X11
     e.xclient.type = ClientMessage;
     e.xclient.message_type = XInternAtom( QX11Info::display(), "_KDE_SPLASH_PROGRESS", False );
     e.xclient.display = QX11Info::display();
     e.xclient.window = QX11Info::appRootWindow();
     e.xclient.format = 8;
     strcpy( e.xclient.data.b, "confupdate" );
     XSendEvent( QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &e );
#endif

     if (bCheckHostname)
        (void) new KHostnameD(HostnamePollInterval); // Watch for hostname changes

     QObject::connect(QDBusConnection::sessionBus().interface(),
                      SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                      kded, SLOT(slotApplicationRemoved(QString,QString,QString)));

     // During startup kdesktop waits for KDED to finish.
     // Send a notifyDatabaseChanged signal even if the database hasn't
     // changed.
     // If the database changed, kbuildsycoca's signal didn't go anywhere
     // anyway, because it was too early, so let's send this signal
     // unconditionnally (David)

     QDBusMessage msg = QDBusMessage::createSignal("/kbuildsycoca", "org.kde.KSycoca", "notifyDatabaseChanged" );
     msg << QStringList();
     QDBusConnection::sessionBus().send(msg);
     int result = k.exec(); // keep running

     delete kded;

     return result;
}

#include "kded.moc"
