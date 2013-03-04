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

#include "kdeversion.h"
#include <config-kdeui.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QSessionManager>
#include <QStyleFactory>
#include <QtCore/QTimer>
#include <QWidget>
#include <QIcon>
#include <QtCore/QList>
#include <QtDBus/QtDBus>
#include <QtCore/QMetaType>

#include "kcoreauthorized.h"
#include "kaboutdata.h"
#include "kcheckaccelerators.h"
#include "kcrash.h"
#include "kconfig.h"
#include "kcmdlineargs.h"
#include "kglobalsettings.h"
#include "kdebug.h"
#include <kglobal.h>
#include "klocale.h"
#include "klocalizedstring.h"
#include "ksessionmanager.h"
#include "ktoolinvocation.h"
#include "kgesturemap.h"
#include "kurl.h"
#include "kmessage.h"
#include "kmessageboxmessagehandler.h"
#include <kconfiggui.h>
#include <kusertimestamp.h>

#if HAVE_X11
#include <qx11info_x11.h>
#include <kstartupinfo.h>
#endif

#include <sys/types.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#ifndef Q_OS_WIN
#include "kwindowsystem.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // srand(), rand()
#include <unistd.h>
#if HAVE_X11
#include <netwm.h>
#endif

#if HAVE_PATHS_H
#include <paths.h>
#endif

#if HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>

#include <QX11Info>
#endif

#ifdef Q_OS_MAC
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

#include <qstandardpaths.h>
#include <QActionEvent>
#include <kcomponentdata.h>

KApplication* KApplication::KApp = 0L;

#if HAVE_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom kde_xdnd_drop;
#endif

template class QList<KSessionManager*>;

#ifdef Q_OS_WIN
void KApplication_init_windows();
#endif

static KApplicationPrivate* kapp_priv = 0;

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
  KApplicationPrivate(KApplication* q, const QByteArray &cName)
      : q(q)
      , componentData(cName)
      , app_started_timer(0)
      , session_save(false)
#if HAVE_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
      kapp_priv = this;
  }

  KApplicationPrivate(KApplication* q, const KComponentData &cData)
      : q(q)
      , componentData(cData)
      , app_started_timer(0)
      , session_save(false)
#if HAVE_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
      kapp_priv = this;
  }

  KApplicationPrivate(KApplication *q)
      : q(q)
      , componentData(KCmdLineArgs::aboutData())
      , app_started_timer( 0 )
      , session_save( false )
#if HAVE_X11
      , oldIceIOErrorHandler( 0 )
      , oldXErrorHandler( 0 )
      , oldXIOErrorHandler( 0 )
#endif
      , pSessionConfig( 0 )
      , bSessionManagement( true )
  {
      kapp_priv = this;
  }

  ~KApplicationPrivate()
  {
  }

#ifndef KDE3_SUPPORT
  KConfig *config() { return KSharedConfig::openConfig().data(); }
#endif

#if HAVE_X11
  int xErrhandler( Display*, void* );
  int xioErrhandler( Display* );
  void iceIOErrorHandler( _IceConn *conn );
#endif

  void _k_x11FilterDestroyed();
  void _k_checkAppStartedSlot();
  void _k_slot_KToolInvocation_hook(QStringList&, QByteArray&);

  void init(bool GUIenabled=true);
  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)

  KApplication *q;
  KComponentData componentData;
  QTimer* app_started_timer;
  bool session_save;

#if HAVE_X11
  IceIOErrorHandler oldIceIOErrorHandler;
  int (*oldXErrorHandler)(Display*,XErrorEvent*);
  int (*oldXIOErrorHandler)(Display*);
#endif

  QString sessionKey;
  QString pSessionConfigFile;

  KConfig* pSessionConfig; //instance specific application config object
  bool bSessionManagement;
};

#if HAVE_X11

extern "C" {
static int kde_xio_errhandler( Display * dpy )
{
  return kapp_priv->xioErrhandler( dpy );
}

static int kde_x_errhandler( Display *dpy, XErrorEvent *err )
{
  return kapp_priv->xErrhandler( dpy, err );
}
}
#endif

static QList< QPointer< QWidget > > *x11Filter = 0;

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
        x11Filter = new QList< QPointer< QWidget > >;
    connect ( filter, SIGNAL(destroyed()), this, SLOT(_k_x11FilterDestroyed()) );
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
    // removeAll doesn't work, creating QPointer to something that's about to be deleted aborts
    // x11Filter->removeAll( const_cast< QWidget* >( filter ));
    for( QMutableListIterator< QPointer< QWidget > > it( *x11Filter );
         it.hasNext();
         ) {
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
#if HAVE_X11
        if( w->isTopLevel() && !KStartupInfo::startupId().isEmpty()) // TODO better done using window group leader?
            KStartupInfo::setWindowStartupId( w->winId(), KStartupInfo::startupId());
#endif
        if( w->isTopLevel() && !( w->windowFlags() & Qt::X11BypassWindowManagerHint ) && w->windowType() != Qt::Popup && !event->spontaneous())
        {
            if( d->app_started_timer == NULL )
            {
                d->app_started_timer = new QTimer( this );
                connect( d->app_started_timer, SIGNAL(timeout()), SLOT(_k_checkAppStartedSlot()));
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
#if HAVE_X11
    KStartupInfo::handleAutoAppStartedSending();
#endif
}

#if HAVE_X11
static SmcConn mySmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif

KApplication::KApplication(bool GUIenabled)
    : QApplication(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), GUIenabled),
    d(new KApplicationPrivate(this))
{
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init(GUIenabled);
}

KApplication::KApplication(bool GUIenabled, const KComponentData &cData)
    : QApplication(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), GUIenabled),
    d (new KApplicationPrivate(this, cData))
{
    setApplicationName(d->componentData.componentName());
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    d->init(GUIenabled);
}

#if HAVE_X11
int KApplicationPrivate::xioErrhandler( Display* dpy )
{
    oldXIOErrorHandler( dpy );
    exit( 1 );
    return 0;
}

int KApplicationPrivate::xErrhandler( Display* dpy, void* err_ )
{
    XErrorEvent* err = static_cast< XErrorEvent* >( err_ );
    if(kapp)
    {
        // add KDE specific stuff here
        oldXErrorHandler( dpy, err );
    }
    const QByteArray fatalXError = qgetenv("KDE_FATAL_X_ERROR");
    if (!fatalXError.isEmpty()) {
        abort();
    }
    return 0;
}

void KApplicationPrivate::iceIOErrorHandler( _IceConn *conn )
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    http://thread.gmane.org/gmane.comp.kde.devel.frameworks/1122
    emit kapp->aboutToQuit();
#endif

    if ( oldIceIOErrorHandler != NULL )
      (*oldIceIOErrorHandler)( conn );
    exit( 1 );
}
#endif

// (only called by KUniqueApplication, no need to export)
bool s_kuniqueapplication_startCalled = false;

void KApplicationPrivate::init(bool GUIenabled)
{
  if ((getuid() != geteuid()) ||
      (getgid() != getegid()))
  {
     fprintf(stderr, "The KDE libraries are not designed to run with suid privileges.\n");
     ::exit(127);
  }

#ifdef Q_OS_MAC
  mac_initialize_dbus();
#endif

  KApplication::KApp = q;

  extern KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface;
  kde_kdebug_enable_dbus_interface = true;

  parseCommandLine();

  QApplication::setDesktopSettingsAware( false );

#if HAVE_X11
  // create all required atoms in _one_ roundtrip to the X server
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
#endif


  // sanity checking, to make sure we've connected
  extern void qDBusBindToApplication();
  qDBusBindToApplication();
  QDBusConnectionInterface *bus = 0;
  if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
      kFatal(240) << "Session bus not found" << endl <<
                  "To circumvent this problem try the following command (with Linux and bash)" << endl <<
                  "export $(dbus-launch)";
      ::exit(125);
  }

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
          kError(240) << "Couldn't register name '" << serviceName << "' with DBUS - another process owns it already!" << endl;
          ::exit(126);
      }
  }
  QDBusConnection::sessionBus().registerObject(QLatin1String("/MainApplication"), q,
                                               QDBusConnection::ExportScriptableSlots |
                                               QDBusConnection::ExportScriptableProperties |
                                               QDBusConnection::ExportAdaptors);

  // Trigger creation of locale.
  (void) KLocale::global();

  KSharedConfig::Ptr config = componentData.config();
  QByteArray readOnly = qgetenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && q->applicationName() != QLatin1String("kdialog"))
  {
    if (KAuthorized::authorize(QLatin1String("warn_unwritable_config")))
       config->isConfigWritable(true);
  }

#if HAVE_X11
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

    q->connect(KToolInvocation::self(), SIGNAL(kapplication_hook(QStringList&,QByteArray&)),
               q, SLOT(_k_slot_KToolInvocation_hook(QStringList&,QByteArray&)));

#ifdef Q_OS_MAC
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
#endif

  qRegisterMetaType<KUrl>();
  qRegisterMetaType<QList<KUrl> >();
  qRegisterMetaType<QList<QUrl> >();

#ifdef Q_OS_WIN
  KApplication_init_windows();
#endif
}

KApplication* KApplication::kApplication()
{
    return KApp;
}

KConfig* KApplication::sessionConfig()
{
    return KConfigGui::sessionConfig();
}

void KApplication::reparseConfiguration()
{
    KSharedConfig::openConfig()->reparseConfiguration();
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
#if HAVE_X11
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

#if HAVE_X11
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
    QString wrapper = QStandardPaths::findExecutable( "kde" NUM_TO_STRING( KDE_VERSION_MAJOR ) ); // "kde4", etc.
#undef NUM_TO_STRING
#undef NUM_TO_STRING2
    if( !wrapper.isEmpty()) {
        QStringList restartCommand = sm.restartCommand();
        restartCommand.prepend( wrapper );
        sm.setRestartCommand( restartCommand );
    }
}
#endif // HAVE_X11

void KApplication::saveState( QSessionManager& sm )
{
    d->session_save = true;
#ifdef __GNUC__
#warning TODO: QSessionManager::handle() is gone in Qt5!
#endif
#if HAVE_X11 && QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
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
    delete d->pSessionConfig;
    d->pSessionConfig = 0;

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

    checkRestartVersion( sm );

    // finally: do session management
    emit saveYourself(); // for compatibility
    bool canceled = false;
    foreach(KSessionManager* it, KSessionManager::sessionClients()) {
      if(canceled) break;
      canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( KConfigGui::hasSessionConfig() ) {
        KConfigGui::sessionConfig()->sync();
        QStringList discard;
        discard  << QLatin1String("rm") << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + '/' + KConfigGui::sessionConfigName();
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

    if (args && args->isSet("config"))
    {
        QString config = args->getOption("config");
        componentData.setConfigName(config);
    }

    if (args && args->isSet("icon")) {
        q->setWindowIcon(QIcon::fromTheme(args->getOption("icon")));
    } else {
        q->setWindowIcon(QIcon::fromTheme(componentData.aboutData()->programIconName()));
    }

    if (!args)
        return;

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

#if HAVE_X11
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

#ifndef Q_OS_WIN
    if (args->isSet("smkey"))
    {
        sessionKey = args->getOption("smkey");
    }
#endif
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
#if HAVE_X11
  if ( d->oldXErrorHandler != NULL )
      XSetErrorHandler( d->oldXErrorHandler );
  if ( d->oldXIOErrorHandler != NULL )
      XSetIOErrorHandler( d->oldXIOErrorHandler );
  if ( d->oldIceIOErrorHandler != NULL )
      IceSetIOErrorHandler( d->oldIceIOErrorHandler );
#endif

  delete d;
  KApp = 0;

#if HAVE_X11
  mySmcConnection = 0;
#endif
}

#ifdef __GNUC__
#warning TODO kapp->installX11EventFilter needs to be ported to nativeEvent filters.
#endif
#if 0 // replaced with QWidget::nativeEvent in Qt5
class KAppX11HackWidget: public QWidget
{
public:
    bool publicx11Event( XEvent * e) { return x11Event( e ); } // no such method anymore!
};
bool KApplication::x11EventFilter( XEvent *_event )
{
    if (x11Filter) {
        foreach (const QPointer< QWidget >& wp, *x11Filter) {
            if( QWidget* w = wp.data())
                if ( static_cast<KAppX11HackWidget*>( w )->publicx11Event(_event))
                    return true;
        }
    }

    return false;
}
#endif

void KApplication::updateUserTimestamp( int time )
{
    KUserTimestamp::updateUserTimestamp(time);
}

unsigned long KApplication::userTimestamp() const
{
    return KUserTimestamp::userTimestamp();
}

void KApplication::updateRemoteUserTimestamp( const QString& service, int time )
{
#if HAVE_X11
    Q_ASSERT(service.contains('.'));
    if( time == 0 )
        time = QX11Info::appUserTime();
    QDBusInterface(service, QLatin1String("/MainApplication"),
            QString(QLatin1String("org.kde.KApplication")))
        .call(QLatin1String("updateUserTimestamp"), time);
#endif
}


#ifndef KDE_NO_DEPRECATED
QString KApplication::tempSaveName( const QString& pFilename )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(240) << "Relative filename passed to KApplication::tempSaveName";
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
            aAutosaveDir.setPath(QDir::tempPath());
        }
    }

  aFilename.replace( '/', QLatin1String("\\!") )
    .prepend( QLatin1Char('#') )
    .append( QLatin1Char('#') )
    .prepend( QLatin1Char('/') ).prepend( aAutosaveDir.absolutePath() );

  return aFilename;
}
#endif


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(240) << "Relative filename passed to KApplication::tempSaveName";
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
          aAutosaveDir.setPath( QDir::tempPath() );
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

#if HAVE_X11
    // set the app startup notification window property
    KStartupInfo::setWindowStartupId(topWidget->winId(), startupId());
#endif
}

QByteArray KApplication::startupId() const
{
    return KStartupInfo::startupId();
}

void KApplication::setStartupId( const QByteArray& startup_id )
{
    KStartupInfo::setStartupId( startup_id );
}

void KApplication::clearStartupId()
{
    KStartupInfo::setStartupId( "0" );
}

// Hook called by KToolInvocation
void KApplicationPrivate::_k_slot_KToolInvocation_hook(QStringList& envs,QByteArray& startup_id)
{
#if HAVE_X11
    if (QX11Info::display()) {
        QByteArray dpystring(XDisplayString(QX11Info::display()));
        envs << QLatin1String("DISPLAY=") + dpystring;
    } else {
        const QByteArray dpystring( qgetenv( "DISPLAY" ));
        if(!dpystring.isEmpty())
            envs << QLatin1String("DISPLAY=") + dpystring;
    }

    if(startup_id.isEmpty())
        startup_id = KStartupInfo::createNewStartupId();
#else
    Q_UNUSED(envs);
    Q_UNUSED(startup_id);
#endif
}

#include "moc_kapplication.cpp"

