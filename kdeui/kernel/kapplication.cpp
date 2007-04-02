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

#include "config.h"

#undef QT_NO_TRANSLATION
#include <qtranslator.h>
#define QT_NO_TRANSLATION
#include <qdir.h>
#include <qfile.h>
#include <qicon.h>
#include <qsessionmanager.h>
#include <qstylefactory.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qlist.h>
#include <QtDBus/QtDBus>
#include <qmetatype.h>

#undef QT_NO_TRANSLATION
#include "kapplication.h"
#define QT_NO_TRANSLATION
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
#include "kiconloader.h"
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
#include "kwm.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <unistd.h>
#if defined Q_WS_X11
//#ifndef Q_WS_QWS //FIXME(E): NetWM should talk to QWS...
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
#include <QSystemTrayIcon>
#endif

#include <qevent.h>
#include <kcomponentdata.h>
#include <klibloader.h>

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;

#ifdef Q_WS_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom kde_xdnd_drop;
#endif

// duplicated from patched Qt, so that there won't be unresolved symbols if Qt gets
// replaced by unpatched one
KDEUI_EXPORT bool qt_qclipboard_bailout_hack = false;

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
class KApplication::Private
{
public:
  Private(const QByteArray &cName)
      : componentData(cName),
      checkAccelerators(0),
      startup_id("0"),
      app_started_timer(0),
      session_save(false)
#ifdef Q_WS_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
  {
  }

  Private(const KComponentData &cData)
      : componentData(cData),
      checkAccelerators(0),
      startup_id("0"),
      app_started_timer(0),
      session_save(false)
#ifdef Q_WS_X11
      , oldIceIOErrorHandler(0)
      , oldXErrorHandler(0)
      , oldXIOErrorHandler(0)
#endif
  {
  }

  Private()
      : componentData(KCmdLineArgs::aboutData()),
      checkAccelerators(0),
      startup_id( "0" ),
      app_started_timer( 0 ),
      session_save( false )
#ifdef Q_WS_X11
      ,oldIceIOErrorHandler( 0 )
      ,oldXErrorHandler( 0 )
      ,oldXIOErrorHandler( 0 )
#endif
  {
  }

  ~Private()
  {
  }

  KComponentData componentData;
  KCheckAccelerators* checkAccelerators;
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
};


static QList<const QWidget*> *x11Filter = 0;

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
        x11Filter = new QList<const QWidget *>;
    connect ( filter, SIGNAL( destroyed() ), this, SLOT( x11FilterDestroyed() ) );
    x11Filter->append( filter );
}

void KApplication::x11FilterDestroyed()
{
    removeX11EventFilter( static_cast< const QWidget* >( sender()));
}

void KApplication::removeX11EventFilter( const QWidget* filter )
{
    if ( !x11Filter || !filter )
        return;
    x11Filter->removeAll( filter );
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
                connect( d->app_started_timer, SIGNAL( timeout()), SLOT( checkAppStartedSlot()));
            }
            if( !d->app_started_timer->isActive()) {
                d->app_started_timer->setSingleShot( true );
                d->app_started_timer->start( 0 );
            }
        }
    }
    return QApplication::notify(receiver, event);
}

void KApplication::checkAppStartedSlot()
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
QString KApplication::sessionConfigName() const
{
#ifdef QT_NO_SESSIONMANAGER
#error QT_NO_SESSIONMANAGER was set, this will not compile. Reconfigure Qt with Session management support.
#endif
    QString sessKey = sessionKey();
    if ( sessKey.isEmpty() && !d->sessionKey.isEmpty() )
        sessKey = d->sessionKey;
    return QString(QLatin1String("session/%1_%2_%3")).arg(applicationName()).arg(sessionId()).arg(sessKey);
}

#ifdef Q_WS_X11
static SmcConn mySmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif

KApplication::KApplication(bool GUIenabled)
    : QApplication(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), GUIenabled),
    d(new Private)
{
    read_app_startup_id();
    setApplicationName(QLatin1String(d->componentData.componentName()));
    setOrganizationDomain( KCmdLineArgs::about->organizationDomain() );
    installSigpipeHandler();
    init(GUIenabled);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap)
    : QApplication(dpy, KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), visual, colormap),
    d(new Private)
{
    read_app_startup_id();
    setApplicationName(QLatin1String(d->componentData.componentName()));
    setOrganizationDomain( KCmdLineArgs::about->organizationDomain() );
    installSigpipeHandler();
    init();
}

KApplication::KApplication(Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap, const KComponentData &cData)
    : QApplication(dpy, KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), visual, colormap),
    d (new Private(cData))
{
    read_app_startup_id();
    setApplicationName(QLatin1String(d->componentData.componentName()));
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    init();
}
#endif

KApplication::KApplication(bool GUIenabled, const KComponentData &cData)
    : QApplication(KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv(), GUIenabled),
    d (new Private(cData))
{
    read_app_startup_id();
    setApplicationName(QLatin1String(d->componentData.componentName()));
    setOrganizationDomain(d->componentData.aboutData()->organizationDomain());
    installSigpipeHandler();
    init();
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName,
        bool GUIenabled)
    : QApplication(display),
    d(new Private(rAppName))
{
    Q_UNUSED(GUIenabled);
    read_app_startup_id();
    setApplicationName(QLatin1String(rAppName));
    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    init();
}
#endif

int KApplication::xioErrhandler( Display* dpy )
{
    if(kapp)
    {
        emit aboutToQuit();
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

class KDETranslator : public QTranslator
{
public:
  KDETranslator(QObject *parent) : QTranslator(parent)
  {
    setObjectName(QLatin1String("kdetranslator"));
  }

  virtual QString translate(const char* context,
					 const char *sourceText,
					 const char* message) const
  {
    return KGlobal::locale()->translateQt(context, sourceText, message);
  }
};

void KApplication::init(bool GUIenabled)
{
  if ((getuid() != geteuid()) ||
      (getgid() != getegid()))
  {
     fprintf(stderr, "The KDE libraries are not designed to run with suid privileges.\n");
     ::exit(127);
  }

  if ( type() == GuiClient )
  {
    QStringList plugins = KGlobal::dirs()->resourceDirs( "qtplugins" );
    QStringList::Iterator it = plugins.begin();
    while (it != plugins.end()) {
      addLibraryPath( *it );
      ++it;
    }
  }

  KApp = this;

  parseCommandLine();

  if(GUIenabled)
    (void) KClipboardSynchronizer::self();

  extern KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface;
  kde_kdebug_enable_dbus_interface = true;

  QApplication::setDesktopSettingsAware( false );

#ifdef Q_WS_X11 //FIXME(E)
  // create all required atoms in _one_ roundtrip to the X server
  if ( type() == GuiClient ) {
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
  }

  extern bool s_kuniqueapplication_startCalled;
  if ( bus && !s_kuniqueapplication_startCalled ) // don't register again if KUniqueApplication did so already
  {
      QStringList parts = organizationDomain().split(QLatin1Char('.'), QString::SkipEmptyParts);
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
      const QString serviceName = reversedDomain + applicationName() + pidSuffix;
      if ( bus->registerService(serviceName) == QDBusConnectionInterface::ServiceNotRegistered ) {
          kError(101) << "Couldn't register name '" << serviceName << "' with DBUS - another process owns it already!" << endl;
          ::exit(126);
      }
  }
  QDBusConnection::sessionBus().registerObject(QLatin1String("/MainApplication"), this,
                                               QDBusConnection::ExportScriptableSlots |
                                               QDBusConnection::ExportScriptableProperties |
                                               QDBusConnection::ExportAdaptors);

  // Trigger creation of locale.
  (void) KGlobal::locale();

  KSharedConfig::Ptr config = d->componentData.config();
  QByteArray readOnly = getenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && applicationName() != QLatin1String("kdialog"))
  {
    if (KAuthorized::authorize(QLatin1String("warn_unwritable_config")))
       config->checkConfigFilesWritable(true);
  }

  if (type() == GuiClient)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    d->oldXErrorHandler = XSetErrorHandler( kde_x_errhandler );
    d->oldXIOErrorHandler = XSetIOErrorHandler( kde_xio_errhandler );
#endif

    // Trigger initial settings
    KGlobalSettings::self();

    KMessage::setMessageHandler( new KMessageBoxMessageHandler(0) );

    d->checkAccelerators = new KCheckAccelerators( this );
    KGestureMap::self()->installEventFilterOnMe( this );

    connect(KToolInvocation::self(), SIGNAL(kapplication_hook(QStringList&, QByteArray&)),
            this, SLOT(slot_KToolInvocation_hook(QStringList&,QByteArray&)));
    connect(KLibLoader::self(), SIGNAL(kapplication_hook_clearClipboard()),
            this, SLOT(slot_KLibLoader_hook_clearClipboard()));
  }

#ifdef Q_WS_MAC
  if (type() == GuiClient) {
      QSystemTrayIcon *trayIcon;
      QPixmap pixmap = KIconLoader::global()->loadIcon( KCmdLineArgs::appName(),
              K3Icon::NoGroup, K3Icon::SizeEnormous, K3Icon::DefaultState, 0L, false );
      if (!pixmap.isNull() && QSystemTrayIcon::isSystemTrayAvailable())
      {
          trayIcon = new QSystemTrayIcon(this);
          trayIcon->setIcon(QIcon(pixmap));
          /* it's counter-intuitive, but once you do setIcon it's already set the
             dock icon... ->show actually shows an icon in the menu bar too  :P */
          // trayIcon->show();
      }
  }
#endif


  // save and restore the RTL setting, as installTranslator calls qt_detectRTLLanguage,
  // which makes it impossible to use the -reverse cmdline switch with KDE apps
  // FIXME is this still needed? it looks like QApplication takes care of this
  bool rtl = isRightToLeft();
  installTranslator(new KDETranslator(this));
  setLayoutDirection( rtl ? Qt::RightToLeft:Qt::LeftToRight);
  if (i18nc( "Dear Translator! Translate this string to the string 'LTR' in "
	 "left-to-right languages (as English) or to 'RTL' in right-to-left "
	 "languages (such as Hebrew and Arabic) to get proper widget layout.",
         "LTR" ) == QLatin1String("RTL"))
      rtl = !rtl;
  setLayoutDirection( rtl ? Qt::RightToLeft:Qt::LeftToRight);

  pSessionConfig = 0L;
  bSessionManagement = true;

  qRegisterMetaType<KUrl>();
  qRegisterMetaType<KUrl::List>();

#ifdef Q_WS_WIN
  KApplication_init_windows();
#endif
}

KConfig* KApplication::sessionConfig()
{
    if (!pSessionConfig) // create an instance specific config object
        pSessionConfig = new KConfig( sessionConfigName(), KConfig::NoGlobals );
    return pSessionConfig;
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
  bSessionManagement = false;
}

void KApplication::enableSessionManagement() {
  bSessionManagement = true;
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

    if ( !bSessionManagement )
        sm.setRestartHint( QSessionManager::RestartNever );
    else
        sm.setRestartHint( QSessionManager::RestartIfRunning );
    d->session_save = false;
}

void KApplication::saveState( QSessionManager& sm )
{
    d->session_save = true;
#ifdef Q_WS_X11
    static bool firstTime = true;
    mySmcConnection = (SmcConn) sm.handle();

    if ( !bSessionManagement ) {
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
    if ( pSessionConfig ) {
        delete pSessionConfig;
        pSessionConfig = 0;
    }

    // tell the session manager about our new lifecycle
    QStringList restartCommand = sm.restartCommand();

    QByteArray multiHead = getenv("KDE_MULTIHEAD");
    if (multiHead.toLower() == "true") {
        // if multihead is enabled, we save our -display argument so that
        // we are restored onto the correct head... one problem with this
        // is that the display is hard coded, which means we cannot restore
        // to a different display (ie. if we are in a university lab and try,
        // try to restore a multihead session, our apps could be started on
        // someone else's display instead of our own)
        QByteArray displayname = getenv("DISPLAY");
        if (! displayname.isNull()) {
            // only store the command if we actually have a DISPLAY
            // environment variable
            restartCommand.append(QLatin1String("-display"));
            restartCommand.append(QLatin1String(displayname));
        }
        sm.setRestartCommand( restartCommand );
    }


    // finally: do session management
    emit saveYourself(); // for compatibility
    bool canceled = false;
    foreach(KSessionManager* it, KSessionManager::sessionClients()) {
      if(canceled) break;
      canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( pSessionConfig ) {
        pSessionConfig->sync();
        QStringList discard;
        discard  << QLatin1String("rm") << KStandardDirs::locateLocal("config", sessionConfigName());
        sm.setDiscardCommand( discard );
    } else {
	sm.setDiscardCommand( QStringList( QLatin1String("") ) );
    }

    if ( canceled )
        sm.cancel();
#else
    // FIXME(E): Implement for Qt Embedded
#endif
    d->session_save = false;
}

bool KApplication::sessionSaving() const
{
    return d->session_save;
}

void KApplication::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if ( type() != Tty ) {
        if (args && args->isSet("icon"))
        {
            QPixmap largeIcon = DesktopIcon(QFile::decodeName(args->getOption("icon")));
            QIcon icon = windowIcon();
            icon.addPixmap(largeIcon, QIcon::Normal, QIcon::On);
            setWindowIcon(icon);
        }
        else {
            QIcon icon = windowIcon();
            QPixmap largeIcon = DesktopIcon(QFile::decodeName(d->componentData.componentName()));
            icon.addPixmap(largeIcon, QIcon::Normal, QIcon::On);
            setWindowIcon(icon);
        }
    }

    if (!args)
        return;

    if (args->isSet("config"))
    {
        QString config = QString::fromLocal8Bit(args->getOption("config"));
        d->componentData.setConfigName(config);
    }

    if (args->isSet("style"))
    {
        extern KDEUI_EXPORT QString kde_overrideStyle; // see KGlobalSettings. Should we have a static setter?
        QStringList styles = QStyleFactory::keys();
        QString reqStyle(QLatin1String(args->getOption("style").toLower()));

        for (QStringList::ConstIterator it = styles.begin(); it != styles.end(); ++it)
            if ((*it).toLower() == reqStyle) {
                kde_overrideStyle = *it;
                break;
            }

        if (kde_overrideStyle.isEmpty())
            fprintf(stderr, "%s", i18n("The style %1 was not found\n", reqStyle).toLocal8Bit().data());
    }

    bool nocrashhandler = (getenv("KDE_DEBUG") != NULL);
    if (!nocrashhandler && args->isSet("crashhandler"))
    {
        // set default crash handler / set emergency save function to nothing
        KCrash::setCrashHandler(KCrash::defaultCrashHandler);
        KCrash::setEmergencySaveFunction(NULL);

        KCrash::setApplicationName(QLatin1String(args->appName()));
    }

#ifdef Q_WS_X11
    if ( args->isSet( "waitforwm" ) ) {
        Atom type;
        (void) desktop(); // trigger desktop creation, we need PropertyNotify events for the root window
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
#else
    // FIXME(E): Implement for Qt Embedded
#endif

    if (args->isSet("smkey"))
    {
        d->sessionKey = QLatin1String(args->getOption("smkey"));
    }

}

extern void kDebugCleanup();

KApplication::~KApplication()
{
  // First call the static deleters and then call KLibLoader::cleanup()
  // The static deleters may delete libraries for which they need KLibLoader.
  // KLibLoader will take care of the remaining ones.
  KGlobal::deleteStaticDeleters();

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
#else
  // FIXME(E): Implement for Qt Embedded
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
#warning This should be already in Qt, check.
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
        foreach (const QWidget *w, *x11Filter) {
            if (((KAppX11HackWidget*) w)->publicx11Event(_event))
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
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
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
    .prepend( QLatin1Char('/') ).prepend( aAutosaveDir.absolutePath() );

  return aFilename;
}


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
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

#if defined Q_WS_X11
//#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    // set the app startup notification window property
    KStartupInfo::setWindowStartupId( topWidget->winId(), startupId());
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

// read the startup notification env variable, save it and unset it in order
// not to propagate it to processes started from this app
void KApplication::read_app_startup_id()
{
#if defined Q_WS_X11
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();
    KStartupInfo::resetStartupEnv();
    d->startup_id = id.id();
#endif
}

// Hook called by KToolInvocation
void KApplication::slot_KToolInvocation_hook(QStringList& envs,QByteArray& startup_id)
{
#ifdef Q_WS_X11
    if (QX11Info::display()) {
        QByteArray dpystring(XDisplayString(QX11Info::display()));
        envs << QString::fromLatin1( QByteArray("DISPLAY=") + dpystring );
    } else if( getenv( "DISPLAY" )) {
        QByteArray dpystring( getenv( "DISPLAY" ));
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

void KApplication::slot_KLibLoader_hook_clearClipboard()
{
    kDebug() << k_funcinfo << endl;
    if( clipboard()->ownsSelection()) {
	clipboard()->setText(
            clipboard()->text( QClipboard::Selection ), QClipboard::Selection );
    }
    if( clipboard()->ownsClipboard()) {
	clipboard()->setText(
            clipboard()->text( QClipboard::Clipboard ), QClipboard::Clipboard );
    }
}

#include "kapplication.moc"

