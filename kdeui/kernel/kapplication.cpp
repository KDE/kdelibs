/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
        */

#include "kapplication.h"

#include <config.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QSessionManager>
#include <QtGui/QStyleFactory>
#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtDBus/QtDBus>
#include <QtCore/QMetaType>

#include "kauthorized.h"
#include "kaboutdata.h"
#include "kcheckaccelerators.h"
#include "kcrash.h"
#include "kconfig.h"
#include "kcmdlineargs.h"
#include "kclipboard.h"
#include "kglobalsettings.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kicon.h"
#include "klocale.h"
#include "ksessionmanager.h"
#include "kstandarddirs.h"
#include "kstandardshortcut.h"
#include "ktoolinvocation.h"
#include "kgesturemap.h"
#include "kurl.h"
#include "kmessage.h"
#include "kmessageboxmessagehandler.h"

#if defined Q_WS_X11
#include <QtGui/qx11info_x11.h>
#include <kstartupinfo.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#ifndef Q_WS_WIN
#include "kwindowsystem.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // srand(), rand()
#include <unistd.h>
#if defined Q_WS_X11
//#ifndef Q_WS_QWS //FIXME(embedded): NetWM should talk to QWS...
#include <netwm.h>
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>

#include <QX11Info>
#endif

#ifdef Q_WS_MACX
// ick
#undef Status
#include <Carbon/Carbon.h>
#include <QImage>
#include <ksystemtrayicon.h>
#include <kkernel_mac.h>
#endif

#ifdef Q_OS_UNIX
#include <signal.h>
#endif

#include <QtGui/QActionEvent>
#include <kcomponentdata.h>

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;

#ifdef Q_WS_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom kde_xdnd_drop;
static QByteArray* startup_id_tmp;
#endif

template class QList<KSessionManager*>;

#ifdef Q_WS_X11
extern "C" {
static int kde_xio_errhandler( Display * dpy )
{
  return kapp->xioErrhandler( dpy );
}

static int kde_x_errhandler( Display *dpy, XErrorEvent *err )
{
  return kapp->xErrhandler( dpy, err );
}

}
#endif

#ifdef Q_WS_WIN
void KApplication_init_windows();
#endif

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
  KApplicationPrivate(KApplication* q, const QByteArray &cName)
      : q(q)
      , componentData(cName)
      , startup_id("0")
      , app_started_timer(0)
      , session_save(false)
#ifdef Q_WS_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
  }

  KApplicationPrivate(KApplication* q, const KComponentData &cData)
      : q(q)
      , componentData(cData)
      , startup_id("0")
      , app_started_timer(0)
      , session_save(false)
#ifdef Q_WS_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
  }

  KApplicationPrivate(KApplication *q)
      : q(q)
      , componentData(KCmdLineArgs::aboutData())
      , startup_id( "0" )
      , app_started_timer( 0 )
      , session_save( false )
#ifdef Q_WS_X11
      , oldIceIOErrorHandler( 0 )
      , oldXErrorHandler( 0 )
      , oldXIOErrorHandler( 0 )
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
  }

  ~KApplicationPrivate()
  {
  }

#ifndef KDE3_SUPPORT
  KConfig *config() { return KGlobal::config().data(); }
#endif

  void _k_x11FilterDestroyed();
  void _k_checkAppStartedSlot();
  void _k_slot_KToolInvocation_hook(QStringList&, QByteArray&);

  QString sessionConfigName() const;
  void init(bool GUIenabled=true);
  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)
  static void preqapplicationhack();
  static void preread_app_startup_id();
  void read_app_startup_id();

  KApplication *q;
  KComponentData componentData;
  QByteArray startup_id;
  QTimer* app_started_timer;
  bool session_save;

#ifdef Q_WS_X11
  IceIOErrorHandler oldIceIOErrorHandler;
  int (*oldXErrorHandler)(Display*,XErrorEvent*);
  int (*oldXIOErrorHandler)(Display*);
#endif

  QString sessionKey;
  QString pSessionConfigFile;

  KConfig* pSessionConfig; //instance specific application config object
  bool bSessionManagement;
};


static QList< QWeakPointer< QWidget > > *x11Filter = 0;

/**
   * Installs a handler for the SIGPIPE signal. It is thrown when you write to
   * a pipe or socket that has been closed.
   * The handler is installed automatically in the constructor, but you may
   * need it if your application or component does not have a KApplication
   * instance.
   */
static void installSigpipeHandler()
{
#ifdef Q_OS_UNIX
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    sigaction( SIGPIPE, &act, 0 );
#endif
}

void KApplication::installX11EventFilter( QWidget* filter )
{
    if ( !filter )
        return;
    if (!x11Filter)
        x11Filter = new QList< QWeakPointer< QWidget > >;
    connect ( filter, SIGNAL( destroyed() ), this, SLOT( _k_x11FilterDestroyed() ) );
    x11Filter->append( filter );
}

void KApplicationPrivate::_k_x11FilterDestroyed()
{
    q->removeX11EventFilter( static_cast< const QWidget* >(q->sender()));
}

void KApplication::removeX11EventFilter( const QWidget* filter )
{
    if ( !x11Filter || !filter )
        return;
    // removeAll doesn't work, creating QWeakPointer to something that's about to be deleted aborts
    // x11Filter->removeAll( const_cast< QWidget* >( filter ));
    QMutableListIterator< QWeakPointer< QWidget > > it( *x11Filter );
    while( it.hasNext()) {
        QWidget* w = it.next().data();
        if( w == filter || w == NULL )
            it.remove();
    }
    if ( x11Filter->isEmpty() ) {
        delete x11Filter;
        x11Filter = 0;
    }
}

bool KApplication::notify(QObject *receiver, QEvent *event)
{
    QEvent::Type t = event->type();
    if( t == QEvent::Show && receiver->isWidgetType())
    {
        QWidget* w = static_cast< QWidget* >( receiver );
#if defined Q_WS_X11
        if( w->isTopLevel() && !startupId().isEmpty()) // TODO better done using window group leader?
            KStartupInfo::setWindowStartupId( w->winId(), startupId());
#endif
        if( w->isTopLevel() && !( w->windowFlags() & Qt::X11BypassWindowManagerHint ) && w->windowType() != Qt::Popup && !event->spontaneous())
        {
            if( d->app_started_timer == NULL )
            {
                d->app_started_timer = new QTimer( this );
                connect( d->app_started_timer, SIGNAL( timeout()), SLOT( _k_checkAppStartedSlot()));
            }
            if( !d->app_started_timer->isActive()) {
                d->app_started_timer->setSingleShot( true );
                d->app_started_timer->start( 0 );
            }
        }
    }
    return QApplication::notify(receiver, event);
}

void KApplicationPrivate::_k_checkAppStartedSlot()
{
#if defined Q_WS_X11
    KStartupInfo::handleAutoAppStartedSending();
#endif
}

/*
  Auxiliary function to calculate a a session config name used for the
  instance specific config object.
  Syntax:  "session/<appname>_<sessionId>"
 */
QString KApplicationPrivate::sessionConfigName() const
{
#ifdef QT_NO_SESSIONMANAGER
#error QT_NO_SESSIONMANAGER was set, this will not compile. Reconfigure Qt with Session management support.
#endif
    QString sessKey = q->sessionKey();
    if ( sessKey.isEmpty() && !sessionKey.isEmpty() )
        sessKey = sessionKey;
    return QString(QLatin1String("session/%1_%2_%3")).arg(q->applicationName()).arg(q->sessionId()).arg(sessKey);
}

#ifdef Q_WS_X11
static SmcConn mySmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif

KApplication::KApplication(bool GUIenabled)
    : QApplication((KApplicationPrivate::preqapplicationhack(),KCmdLineArgs::qtArgc()), KCmdLineArgs::qtArgv(), GUIenabled),
    d(new KApplicationPrivate(this))
{
    d->read_app_startup_id();
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init(GUIenabled);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap)
    : QApplication((KApplicationPrivate::preqapplicationhack(),dpy), KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), visual, colormap),
    d(new KApplicationPrivate(this))
{
    d->read_app_startup_id();
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init();
}

KApplication::KApplication(Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap, const KComponentData &cData)
    : QApplication((KApplicationPrivate::preqapplicationhack(),dpy), KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), visual, colormap),
    d (new KApplicationPrivate(this, cData))
{
    d->read_app_startup_id();
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init();
}
#endif

KApplication::KApplication(bool GUIenabled, const KComponentData &cData)
    : QApplication((KApplicationPrivate::preqapplicationhack(),KCmdLineArgs::qtArgc()), KCmdLineArgs::qtArgv(), GUIenabled),
    d (new KApplicationPrivate(this, cData))
{
    d->read_app_startup_id();
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init(GUIenabled);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName,
        bool GUIenabled)
    : QApplication((KApplicationPrivate::preqapplicationhack(),display)),
    d(new KApplicationPrivate(this, rAppName))
{
    Q_UNUSED(GUIenabled);
    d->read_app_startup_id();
    setApplicationName(QLatin1String(rAppName));
    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    d->init();
}
#endif

// this function is called in KApplication ctors while evaluating arguments to QApplication ctor,
// i.e. before QApplication ctor is called
void KApplicationPrivate::preqapplicationhack()
{
    preread_app_startup_id();
}

int KApplication::xioErrhandler( Display* dpy )
{
    if(kapp)
    {
#ifdef Q_WS_X11
        d->oldXIOErrorHandler( dpy );
#else
        Q_UNUSED(dpy);
#endif
    }
    exit( 1 );
    return 0;
}

int KApplication::xErrhandler( Display* dpy, void* err_ )
{ // no idea how to make forward decl. for XErrorEvent
#ifdef Q_WS_X11
    XErrorEvent* err = static_cast< XErrorEvent* >( err_ );
    if(kapp)
    {
        // add KDE specific stuff here
        d->oldXErrorHandler( dpy, err );
    }
    const QByteArray fatalXError = qgetenv("KDE_FATAL_X_ERROR");
    if (!fatalXError.isEmpty()) {
        abort();
    }
#endif
    return 0;
}

void KApplication::iceIOErrorHandler( _IceConn *conn )
{
    emit aboutToQuit();

#ifdef Q_WS_X11
    if ( d->oldIceIOErrorHandler != NULL )
      (*d->oldIceIOErrorHandler)( conn );
#endif
    exit( 1 );
}

void KApplicationPrivate::init(bool GUIenabled)
{
  if ((getuid() != geteuid()) ||
      (getgid() != getegid()))
  {
     fprintf(stderr, "The KDE libraries are not designed to run with suid privileges.\n");
     ::exit(127);
  }

#ifdef Q_WS_MAC
  mac_initialize_dbus();
#endif

  KApplication::KApp = q;

  // make sure the clipboard is created before setting the window icon (bug 209263)
  if(GUIenabled)
    (void) QApplication::clipboard();

  parseCommandLine();

  if(GUIenabled)
    (void) KClipboardSynchronizer::self();

  extern KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface;
  kde_kdebug_enable_dbus_interface = true;

  QApplication::setDesktopSettingsAware( false );

#ifdef Q_WS_X11
  // create all required atoms in _one_ roundtrip to the X server
  if ( q->type() == KApplication::GuiClient ) {
      const int max = 20;
      Atom* atoms[max];
      char* names[max];
      Atom atoms_return[max];
      int n = 0;

      atoms[n] = &atom_DesktopWindow;
      names[n++] = (char *) "KDE_DESKTOP_WINDOW";

      atoms[n] = &atom_NetSupported;
      names[n++] = (char *) "_NET_SUPPORTED";

      atoms[n] = &kde_xdnd_drop;
      names[n++] = (char *) "XdndDrop";

      XInternAtoms( QX11Info::display(), names, n, false, atoms_return );

      for (int i = 0; i < n; i++ )
        *atoms[i] = atoms_return[i];
  }
#endif


  // sanity checking, to make sure we've connected
  extern void qDBusBindToApplication();
  qDBusBindToApplication();
  QDBusConnectionInterface *bus = 0;
  if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
      kFatal(101) << "Session bus not found" << endl;
      ::exit(125);
  }

  extern bool s_kuniqueapplication_startCalled;
  if ( bus && !s_kuniqueapplication_startCalled ) // don't register again if KUniqueApplication did so already
  {
      QStringList parts = q->organizationDomain().split(QLatin1Char('.'), QString::SkipEmptyParts);
      QString reversedDomain;
      if (parts.isEmpty())
          reversedDomain = QLatin1String("local.");
      else
          foreach (const QString& s, parts)
          {
              reversedDomain.prepend(QLatin1Char('.'));
              reversedDomain.prepend(s);
          }
      const QString pidSuffix = QString::number( getpid() ).prepend( QLatin1String("-") );
      const QString serviceName = reversedDomain + q->applicationName() + pidSuffix;
      if ( bus->registerService(serviceName) == QDBusConnectionInterface::ServiceNotRegistered ) {
          kError(101) << "Couldn't register name '" << serviceName << "' with DBUS - another process owns it already!" << endl;
          ::exit(126);
      }
  }
  QDBusConnection::sessionBus().registerObject(QLatin1String("/MainApplication"), q,
                                               QDBusConnection::ExportScriptableSlots |
                                               QDBusConnection::ExportScriptableProperties |
                                               QDBusConnection::ExportAdaptors);

  // Trigger creation of locale.
  (void) KGlobal::locale();

  KSharedConfig::Ptr config = componentData.config();
  QByteArray readOnly = qgetenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && q->applicationName() != QLatin1String("kdialog"))
  {
    if (KAuthorized::authorize(QLatin1String("warn_unwritable_config")))
       config->isConfigWritable(true);
  }

  if (q->type() == KApplication::GuiClient)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    oldXErrorHandler = XSetErrorHandler( kde_x_errhandler );
    oldXIOErrorHandler = XSetIOErrorHandler( kde_xio_errhandler );
#endif

    // Trigger initial settings
    KGlobalSettings::self()->activate();

    KMessage::setMessageHandler( new KMessageBoxMessageHandler(0) );

    KCheckAccelerators::initiateIfNeeded(q);
    KGestureMap::self()->installEventFilterOnMe( q );

    q->connect(KToolInvocation::self(), SIGNAL(kapplication_hook(QStringList&, QByteArray&)),
               q, SLOT(_k_slot_KToolInvocation_hook(QStringList&,QByteArray&)));
  }

#ifdef Q_WS_MAC
  if (q->type() == KApplication::GuiClient) {
      // This is a QSystemTrayIcon instead of K* because we can't be sure q is a QWidget
      QSystemTrayIcon *trayIcon; //krazy:exclude=qclasses
      if (QSystemTrayIcon::isSystemTrayAvailable()) //krazy:exclude=qclasses
      {
          trayIcon = new QSystemTrayIcon(q); //krazy:exclude=qclasses
          trayIcon->setIcon(q->windowIcon());
          /* it's counter-intuitive, but once you do setIcon it's already set the
             dock icon... ->show actually shows an icon in the menu bar too  :P */
          // trayIcon->show();
      }
  }
#endif

  qRegisterMetaType<KUrl>();
  qRegisterMetaType<KUrl::List>();

#ifdef Q_WS_WIN
  KApplication_init_windows();
#endif
}

KApplication* KApplication::kApplication()
{
    return KApp;
}

KConfig* KApplication::sessionConfig()
{
    if (!d->pSessionConfig) // create an instance specific config object
        d->pSessionConfig = new KConfig( d->sessionConfigName(), KConfig::SimpleConfig );
    return d->pSessionConfig;
}

void KApplication::reparseConfiguration()
{
    KGlobal::config()->reparseConfiguration();
}

void KApplication::quit()
{
    QApplication::quit();
}

void KApplication::disableSessionManagement() {
  d->bSessionManagement = false;
}

void KApplication::enableSessionManagement() {
  d->bSessionManagement = true;
#ifdef Q_WS_X11
  // Session management support in Qt/KDE is awfully broken.
  // If konqueror disables session management right after its startup,
  // and enables it later (preloading stuff), it won't be properly
  // saved on session shutdown.
  // I'm not actually sure why it doesn't work, but saveState()
  // doesn't seem to be called on session shutdown, possibly
  // because disabling session management after konqueror startup
  // disabled it somehow. Forcing saveState() here for this application
  // seems to fix it.
  if( mySmcConnection ) {
        SmcRequestSaveYourself( mySmcConnection, SmSaveLocal, False,
				SmInteractStyleAny,
				False, False );

	// flush the request
	IceFlush(SmcGetIceConnection(mySmcConnection));
  }
#endif
}

void KApplication::commitData( QSessionManager& sm )
{
    d->session_save = true;
    bool canceled = false;

    foreach (KSessionManager *it, KSessionManager::sessionClients()) {
        if ( ( canceled = !it->commitData( sm ) ) )
            break;
    }

    if ( canceled )
        sm.cancel();

    if ( sm.allowsInteraction() ) {
        QWidgetList donelist, todolist;
        QWidget* w;

commitDataRestart:
        todolist = QApplication::topLevelWidgets();

        for ( int i = 0; i < todolist.size(); ++i ) {
            w = todolist.at( i );
            if( !w )
                break;

            if ( donelist.contains( w ) )
                continue;

            if ( !w->isHidden() && !w->inherits( "KMainWindow" ) ) {
                QCloseEvent e;
                sendEvent( w, &e );
                if ( !e.isAccepted() )
                    break; //canceled

                donelist.append( w );

                //grab the new list that was just modified by our closeevent
                goto commitDataRestart;
            }
        }
    }

    if ( !d->bSessionManagement )
        sm.setRestartHint( QSessionManager::RestartNever );
    else
        sm.setRestartHint( QSessionManager::RestartIfRunning );
    d->session_save = false;
}

#ifdef Q_WS_X11
static void checkRestartVersion( QSessionManager& sm )
{
    Display* dpy = QX11Info::display();
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* data;
    if( dpy != NULL && XGetWindowProperty( dpy, RootWindow( dpy, 0 ), XInternAtom( dpy, "KDE_SESSION_VERSION", False ),
        0, 1, False, AnyPropertyType, &type, &format, &nitems, &after, &data ) == Success ) {
        if( type == XA_CARDINAL && format == 32 ) {
            int version = *( long* ) data;
            if( version == KDE_VERSION_MAJOR ) { // we run in our native session
                XFree( data );
                return; // no need to wrap
            }
        }
        XFree( data );
    }
    if( getenv( "KDE_SESSION_VERSION" ) != NULL && atoi( getenv( "KDE_SESSION_VERSION" )) == KDE_VERSION_MAJOR )
        return; // we run in our native session, no need to wrap
#define NUM_TO_STRING2( num ) #num
#define NUM_TO_STRING( num ) NUM_TO_STRING2( num )
    QString wrapper = KStandardDirs::findExe( "kde" NUM_TO_STRING( KDE_VERSION_MAJOR ) ); // "kde4", etc.
#undef NUM_TO_STRING
#undef NUM_TO_STRING2
    if( !wrapper.isEmpty()) {
        QStringList restartCommand = sm.restartCommand();
        restartCommand.prepend( wrapper );
        sm.setRestartCommand( restartCommand );
    }
}
#endif // Q_WS_X11

void KApplication::saveState( QSessionManager& sm )
{
    d->session_save = true;
#ifdef Q_WS_X11
    static bool firstTime = true;
    mySmcConnection = (SmcConn) sm.handle();

    if ( !d->bSessionManagement ) {
        sm.setRestartHint( QSessionManager::RestartNever );
	d->session_save = false;
        return;
    }
    else
	sm.setRestartHint( QSessionManager::RestartIfRunning );

    if ( firstTime ) {
        firstTime = false;
	d->session_save = false;
        return; // no need to save the state.
    }

    // remove former session config if still existing, we want a new
    // and fresh one. Note that we do not delete the config file here,
    // this is done by the session manager when it executes the
    // discard commands. In fact it would be harmful to remove the
    // file here, as the session might be stored under a different
    // name, meaning the user still might need it eventually.
    if ( d->pSessionConfig ) {
        delete d->pSessionConfig;
        d->pSessionConfig = 0;
    }

    // tell the session manager about our new lifecycle
    QStringList restartCommand = sm.restartCommand();

    QByteArray multiHead = qgetenv("KDE_MULTIHEAD");
    if (multiHead.toLower() == "true") {
        // if multihead is enabled, we save our -display argument so that
        // we are restored onto the correct head... one problem with this
        // is that the display is hard coded, which means we cannot restore
        // to a different display (ie. if we are in a university lab and try,
        // try to restore a multihead session, our apps could be started on
        // someone else's display instead of our own)
        QByteArray displayname = qgetenv("DISPLAY");
        if (! displayname.isNull()) {
            // only store the command if we actually have a DISPLAY
            // environment variable
            restartCommand.append(QLatin1String("-display"));
            restartCommand.append(QLatin1String(displayname));
        }
        sm.setRestartCommand( restartCommand );
    }

#ifdef Q_WS_X11
    checkRestartVersion( sm );
#endif

    // finally: do session management
    emit saveYourself(); // for compatibility
    bool canceled = false;
    foreach(KSessionManager* it, KSessionManager::sessionClients()) {
      if(canceled) break;
      canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( d->pSessionConfig ) {
        d->pSessionConfig->sync();
        QStringList discard;
        discard  << QLatin1String("rm") << KStandardDirs::locateLocal("config", d->sessionConfigName());
        sm.setDiscardCommand( discard );
    } else {
	sm.setDiscardCommand( QStringList( QLatin1String("") ) );
    }

    if ( canceled )
        sm.cancel();
#endif
    d->session_save = false;
}

bool KApplication::sessionSaving() const
{
    return d->session_save;
}

void KApplicationPrivate::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if (args && args->isSet("style"))
    {
        extern QString kde_overrideStyle; // see KGlobalSettings. Should we have a static setter?
        QString reqStyle(args->getOption("style").toLower());
        if (QStyleFactory::keys().contains(reqStyle, Qt::CaseInsensitive))
            kde_overrideStyle = reqStyle;
        else
            qWarning() << i18n("The style '%1' was not found", reqStyle);
    }

    if ( q->type() != KApplication::Tty ) {
        if (args && args->isSet("icon"))
        {
            q->setWindowIcon(KIcon(args->getOption("icon")));
        }
        else {
            q->setWindowIcon(KIcon(componentData.aboutData()->programIconName()));
        }
    }

    if (!args)
        return;

    if (args->isSet("config"))
    {
        QString config = args->getOption("config");
        componentData.setConfigName(config);
    }

    bool nocrashhandler = (!qgetenv("KDE_DEBUG").isEmpty());
    if (!nocrashhandler && args->isSet("crashhandler"))
    {
        // enable drkonqi
        KCrash::setDrKonqiEnabled(true);
    }
    // Always set the app name, can be usefuls for apps that call setEmergencySaveFunction or enable AutoRestart
    KCrash::setApplicationName(args->appName());
    if (!QCoreApplication::applicationDirPath().isEmpty()) {
        KCrash::setApplicationPath(QCoreApplication::applicationDirPath());
    }

#ifdef Q_WS_X11
    if ( args->isSet( "waitforwm" ) ) {
        Atom type;
        (void) q->desktop(); // trigger desktop creation, we need PropertyNotify events for the root window
        int format;
        unsigned long length, after;
        unsigned char *data;
        while ( XGetWindowProperty( QX11Info::display(), QX11Info::appRootWindow(), atom_NetSupported,
				    0, 1, false, AnyPropertyType, &type, &format,
                                    &length, &after, &data ) != Success || !length ) {
            if ( data )
                XFree( data );
            XEvent event;
            XWindowEvent( QX11Info::display(), QX11Info::appRootWindow(), PropertyChangeMask, &event );
        }
        if ( data )
            XFree( data );
    }
#endif

#ifndef Q_WS_WIN
    if (args->isSet("smkey"))
    {
        sessionKey = args->getOption("smkey");
    }
#endif
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
#ifdef Q_WS_X11
  if ( d->oldXErrorHandler != NULL )
      XSetErrorHandler( d->oldXErrorHandler );
  if ( d->oldXIOErrorHandler != NULL )
      XSetIOErrorHandler( d->oldXIOErrorHandler );
  if ( d->oldIceIOErrorHandler != NULL )
      IceSetIOErrorHandler( d->oldIceIOErrorHandler );
#endif

  delete d;
  KApp = 0;

#ifdef Q_WS_X11
  mySmcConnection = 0;
#endif
}


#ifdef Q_WS_X11
class KAppX11HackWidget: public QWidget
{
public:
    bool publicx11Event( XEvent * e) { return x11Event( e ); }
};
#endif



#ifdef Q_WS_X11
bool KApplication::x11EventFilter( XEvent *_event )
{
    switch ( _event->type ) {
        case ClientMessage:
        {
#if KDE_IS_VERSION( 3, 90, 90 )
#ifdef __GNUC__
#warning This should be already in Qt, check.
#endif
#endif
        // Workaround for focus stealing prevention not working when dragging e.g. text from KWrite
        // to KDesktop -> the dialog asking for filename doesn't get activated. This is because
        // Qt-3.2.x doesn't have concept of qt_x_user_time at all, and Qt-3.3.0b1 passes the timestamp
        // in the XdndDrop message in incorrect field (and doesn't update qt_x_user_time either).
        // Patch already sent, future Qt version should have this fixed.
            if( _event->xclient.message_type == kde_xdnd_drop )
                { // if the message is XdndDrop
                if( _event->xclient.data.l[ 1 ] == 1 << 24     // and it's broken the way it's in Qt-3.2.x
                    && _event->xclient.data.l[ 2 ] == 0
                    && _event->xclient.data.l[ 4 ] == 0
                    && _event->xclient.data.l[ 3 ] != 0 )
                    {
                    if( QX11Info::appUserTime() == 0
                        || NET::timestampCompare( _event->xclient.data.l[ 3 ], QX11Info::appUserTime() ) > 0 )
                        { // and the timestamp looks reasonable
                        QX11Info::setAppUserTime(_event->xclient.data.l[ 3 ]); // update our qt_x_user_time from it
                        }
                    }
                else // normal DND, only needed until Qt updates qt_x_user_time from XdndDrop
                    {
                    if( QX11Info::appUserTime() == 0
                        || NET::timestampCompare( _event->xclient.data.l[ 2 ], QX11Info::appUserTime() ) > 0 )
                        { // the timestamp looks reasonable
                        QX11Info::setAppUserTime(_event->xclient.data.l[ 2 ]); // update our qt_x_user_time from it
                        }
                    }
                }
        }
        default: break;
    }

    if (x11Filter) {
        foreach (const QWeakPointer< QWidget >& wp, *x11Filter) {
            if( QWidget* w = wp.data())
                if ( static_cast<KAppX11HackWidget*>( w )->publicx11Event(_event))
                    return true;
        }
    }

    return false;
}
#endif // Q_WS_X11

void KApplication::updateUserTimestamp( int time )
{
#if defined Q_WS_X11
    if( time == 0 )
    { // get current X timestamp
        Window w = XCreateSimpleWindow( QX11Info::display(), QX11Info::appRootWindow(), 0, 0, 1, 1, 0, 0, 0 );
        XSelectInput( QX11Info::display(), w, PropertyChangeMask );
        unsigned char data[ 1 ];
        XChangeProperty( QX11Info::display(), w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
        XEvent ev;
        XWindowEvent( QX11Info::display(), w, PropertyChangeMask, &ev );
        time = ev.xproperty.time;
        XDestroyWindow( QX11Info::display(), w );
    }
    if( QX11Info::appUserTime() == 0
        || NET::timestampCompare( time, QX11Info::appUserTime()) > 0 ) // time > appUserTime
        QX11Info::setAppUserTime(time);
    if( QX11Info::appTime() == 0
        || NET::timestampCompare( time, QX11Info::appTime()) > 0 ) // time > appTime
        QX11Info::setAppTime(time);
#endif
}

unsigned long KApplication::userTimestamp() const
{
#if defined Q_WS_X11
    return QX11Info::appUserTime();
#else
    return 0;
#endif
}

void KApplication::updateRemoteUserTimestamp( const QString& service, int time )
{
#if defined Q_WS_X11
    Q_ASSERT(service.contains('.'));
    if( time == 0 )
        time = QX11Info::appUserTime();
    QDBusInterface(service, QLatin1String("/MainApplication"),
            QString(QLatin1String("org.kde.KApplication")))
        .call(QLatin1String("updateUserTimestamp"), time);
#endif
}


QString KApplication::tempSaveName( const QString& pFilename )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName";
      aFilename = QFileInfo( QDir( QLatin1String(".") ), pFilename ).absoluteFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homePath() + QLatin1String("/autosave/") );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absolutePath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( '/', QLatin1String("\\!") )
    .prepend( QLatin1Char('#') )
    .append( QLatin1Char('#') )
    .prepend( QLatin1Char('/') ).prepend( aAutosaveDir.absolutePath() );

  return aFilename;
}


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName";
      aFilename = QFileInfo( QDir( QLatin1String(".") ), pFilename ).absoluteFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homePath() + QLatin1String("/autosave/") );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absolutePath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( QLatin1String("/"), QLatin1String("\\!") )
      .prepend( QLatin1Char('#') )
      .append( QLatin1Char('#') )
      .prepend( QLatin1Char('/') )
      .prepend( aAutosaveDir.absolutePath() );

  if( QFile( aFilename ).exists() )
    {
      bRecover = true;
      return aFilename;
    }
  else
    {
      bRecover = false;
      return pFilename;
    }
}


void KApplication::setTopWidget( QWidget *topWidget )
{
    if( !topWidget )
      return;

    // set the specified caption
    if ( !topWidget->inherits("KMainWindow") ) { // KMainWindow does this already for us
        topWidget->setWindowTitle(KGlobal::caption());
    }

#ifdef Q_WS_X11
    // set the app startup notification window property
    KStartupInfo::setWindowStartupId(topWidget->winId(), startupId());
#endif
}

QByteArray KApplication::startupId() const
{
    return d->startup_id;
}

void KApplication::setStartupId( const QByteArray& startup_id )
{
    if( startup_id == d->startup_id )
        return;
#if defined Q_WS_X11
    KStartupInfo::handleAutoAppStartedSending(); // finish old startup notification if needed
#endif
    if( startup_id.isEmpty())
        d->startup_id = "0";
    else
        {
        d->startup_id = startup_id;
#if defined Q_WS_X11
        KStartupInfoId id;
        id.initId( startup_id );
        long timestamp = id.timestamp();
        if( timestamp != 0 )
            updateUserTimestamp( timestamp );
#endif
        }
}

void KApplication::clearStartupId()
{
    d->startup_id = "0";
}

// Qt reads and unsets the value and doesn't provide any way to reach the value,
// so steal it from it beforehand. If Qt gets API for taking (reading and unsetting)
// the startup id from it, this can be dumped.
void KApplicationPrivate::preread_app_startup_id()
{
#if defined Q_WS_X11
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();
    KStartupInfo::resetStartupEnv();
    startup_id_tmp = new QByteArray( id.id());
#endif
}

// read the startup notification env variable, save it and unset it in order
// not to propagate it to processes started from this app
void KApplicationPrivate::read_app_startup_id()
{
#if defined Q_WS_X11
    startup_id = *startup_id_tmp;
    delete startup_id_tmp;
    startup_id_tmp = NULL;
#endif
}

// Hook called by KToolInvocation
void KApplicationPrivate::_k_slot_KToolInvocation_hook(QStringList& envs,QByteArray& startup_id)
{
#ifdef Q_WS_X11
    if (QX11Info::display()) {
        QByteArray dpystring(XDisplayString(QX11Info::display()));
        envs << QString::fromLatin1( QByteArray("DISPLAY=") + dpystring );
    } else {
        const QByteArray dpystring( qgetenv( "DISPLAY" ));
        if(!dpystring.isEmpty())
            envs << QString::fromLatin1( QByteArray("DISPLAY=") + dpystring );
    }

    if(startup_id.isEmpty())
    	startup_id = KStartupInfo::createNewStartupId();
#else
    Q_UNUSED(envs);
    Q_UNUSED(startup_id);
#endif
}

void KApplication::setSynchronizeClipboard(bool synchronize)
{
    KClipboardSynchronizer::self()->setSynchronizing(synchronize);
    KClipboardSynchronizer::self()->setReverseSynchronizing(synchronize);
}

#include "kapplication.moc"

