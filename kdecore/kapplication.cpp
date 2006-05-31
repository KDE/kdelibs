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
#include <q3stylesheet.h> // no equivilant in Qt4
#include <qtextedit.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qicon.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qmetaobject.h>
#include <qpixmapcache.h>
#include <qregexp.h>
#include <qsessionmanager.h>
#include <qstylefactory.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qwidget.h>
#include <qlist.h>
#include <dbus/qdbus.h>

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
#include "kglobalaccel.h"
#include "kglobalsettings.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kiconloader.h"
#include "klibloader.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kmimesourcefactory.h"
#include "kstdaccel.h"

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
#include "kwin.h"
#endif

#include <fcntl.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#if defined Q_WS_X11
//#ifndef Q_WS_QWS //FIXME(E): NetWM should talk to QWS...
#include <netwm.h>
#endif

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>

#include <kipc.h>
#include <QX11Info>
#endif

#ifdef Q_WS_MACX
// ick
#undef Status
#include <Carbon/Carbon.h>
#include <qimage.h>
#endif

#include <qevent.h>
#include <QDesktopWidget>
#include <QMetaObject>

// exported for kdm kfrontend
KDE_EXPORT bool kde_have_kipc = true; // magic hook to disable kipc in kdm
bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;

#ifdef Q_WS_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom kde_xdnd_drop;
#endif

// duplicated from patched Qt, so that there won't be unresolved symbols if Qt gets
// replaced by unpatched one
KDECORE_EXPORT bool qt_qclipboard_bailout_hack = false;

template class QList<KSessionManaged*>;

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

extern "C" {
static void kde_ice_ioerrorhandler( IceConn conn )
{
    if(kapp)
        kapp->iceIOErrorHandler( conn );
    // else ignore the error for now
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
  Private()
    :   oldIceIOErrorHandler( 0 ),
	checkAccelerators( 0 ),
	overrideStyle( QString() ),
	startup_id( "0" ),
	app_started_timer( NULL ),
	session_save( false )
#ifdef Q_WS_X11
	,oldXErrorHandler( NULL )
	,oldXIOErrorHandler( NULL )
#endif
  {
  }

  ~Private()
  {
  }


  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  QString overrideStyle;
  QByteArray startup_id;
  QTimer* app_started_timer;
  bool session_save;
#ifdef Q_WS_X11
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

    if ((t == QEvent::ShortcutOverride) || (t == QEvent::KeyPress))
    {
       static const KShortcut& _selectAll = KStdAccel::selectAll();
       QLineEdit *edit = ::qobject_cast<QLineEdit *>(receiver);
       if (edit)
       {
          // We have a keypress for a lineedit...
          QKeyEvent *kevent = static_cast<QKeyEvent *>(event);
          int key = kevent->key() | kevent->modifiers();
          if (_selectAll.contains(key))
          {
             if (t == QEvent::KeyPress)
             {
                edit->selectAll();
                return true;
             }
             else
             {
                kevent->accept();
             }
          }
          // Ctrl-U deletes from start of line.
          if (key == Qt::CTRL + Qt::Key_U)
          {
             if (t == QEvent::KeyPress)
             {
                if (!edit->isReadOnly())
                {
                   QString t(edit->text());
                   t = t.mid(edit->cursorPosition());
                   // TODO: how to port correctly?
                   // edit->validateAndSet(t, 0, 0, 0);
                   edit->setText( t );
                }
                return true;
             }
             else
             {
                kevent->accept();
             }

          }
       }
       QTextEdit *medit = ::qobject_cast<QTextEdit *>(receiver);
       if (medit)
       {
          // We have a keypress for a multilineedit...
          QKeyEvent *kevent = static_cast<QKeyEvent *>(event);
          if (_selectAll.contains(kevent->key() | kevent->modifiers()))
          {
             if (t == QEvent::KeyPress)
             {
                medit->selectAll();
                return true;
             }
             else
             {
                kevent->accept();
             }
          }
       }
    }
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

// the help class for session management communication
static QList<KSessionManaged *>* sessionClients()
{
    static QList<KSessionManaged*>* session_clients = 0L;
    if ( !session_clients )
         session_clients = new QList<KSessionManaged *>;
    return session_clients;
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
    return QString("session/%1_%2_%3").arg(applicationName()).arg(sessionId()).arg(sessKey);
}

#ifdef Q_WS_X11
static SmcConn mySmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif

KApplication::KApplication( bool GUIenabled ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( KCmdLineArgs::about ), d (new Private)
{

    read_app_startup_id();
    setApplicationName(instanceName());
    installSigpipeHandler();
    parseCommandLine( );
    init();
}

#ifdef Q_WS_X11
KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap ) :
  QApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual, colormap ),
  KInstance( KCmdLineArgs::about ), d (new Private)
{
    read_app_startup_id();
    setApplicationName(instanceName());
    installSigpipeHandler();
    parseCommandLine( );
    init();
}

KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap,
		            KInstance * _instance ) :
  QApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual, colormap ),
  KInstance( _instance ), d (new Private)
{
    read_app_startup_id();
    setApplicationName(instanceName());
    installSigpipeHandler();
    parseCommandLine( );
    init();
}
#endif

KApplication::KApplication( bool GUIenabled, KInstance* _instance ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( _instance ), d (new Private)
{
    read_app_startup_id();
    setApplicationName(instanceName());
    installSigpipeHandler();
    parseCommandLine( );
    init();
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName,
                           bool GUIenabled ) :
  QApplication( display ), KInstance(rAppName), d (new Private)
{
    Q_UNUSED(GUIenabled);
    read_app_startup_id();
    setApplicationName(rAppName);
    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
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
    setObjectName("kdetranslator");
  }

  virtual QString translate(const char* context,
					 const char *sourceText,
					 const char* message) const
  {
    return KGlobal::locale()->translateQt(context, sourceText, message);
  }
};

void KApplication::init()
{
  if ((getuid() != geteuid()) ||
      (getgid() != getegid()))
  {
     fprintf(stderr, "The KDE libraries are not designed to run with suid privileges.\n");
     ::exit(127);
  }

  KProcessController::ref();

  (void) KClipboardSynchronizer::self();

  QApplication::setDesktopSettingsAware( false );

  KApp = this;

#ifdef Q_WS_X11 //FIXME(E)
  // create all required atoms in _one_ roundtrip to the X server
  if ( type() == GuiClient ) {
      const int max = 20;
      Atom* atoms[max];
      char* names[max];
      Atom atoms_return[max];
      int n = 0;

      atoms[n] = &kipcCommAtom;
      names[n++] = (char *) "KIPC_COMM_ATOM";

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
  QDBusBusService *bus = 0;
  if (!QDBus::sessionBus().isConnected() || !(bus = QDBus::sessionBus().busService()))
      kFatal(101) << "Session bus not found" << endl;
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
  bus->requestName(reversedDomain + applicationName(), QDBusBusService::AllowReplacingName);
  QDBus::sessionBus().registerObject("/MainApplication", this, QDBusConnection::ExportSlots);

  smw = 0;

  // Initial KIPC event mask.
#if defined Q_WS_X11
  kipcEventMask = (1 << KIPC::StyleChanged) | (1 << KIPC::PaletteChanged) |
                  (1 << KIPC::FontChanged) | (1 << KIPC::BackgroundChanged) |
                  (1 << KIPC::ToolbarStyleChanged) | (1 << KIPC::SettingsChanged) |
                  (1 << KIPC::ClipboardConfigChanged) | (1 << KIPC::BlockShortcuts);
#endif

  // Trigger creation of locale.
  (void) KGlobal::locale();

  KConfig* config = KGlobal::config();
  QByteArray readOnly = getenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && applicationName() != QLatin1String("kdialog"))
  {
    if (KAuthorized::authorize("warn_unwritable_config"))
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

    {
        QStringList plugins = KGlobal::dirs()->resourceDirs( "qtplugins" );
        QStringList::Iterator it = plugins.begin();
        while (it != plugins.end()) {
            addLibraryPath( *it );
            ++it;
        }
    }
    kdisplaySetStyle();
    kdisplaySetFont();
    propagateSettings(SETTINGS_QT);

    // Set default mime-source factory
    // XXX: This is a hack. Make our factory the default factory, but add the
    // previous default factory to the list of factories. Why? When the default
    // factory can't resolve something, it iterates in the list of factories.
    // But it QWhatsThis only uses the default factory. So if there was already
    // a default factory (which happens when using an image library using uic),
    // we prefer KDE's factory and so we put that old default factory in the
    // list and use KDE as the default. This may speed up things as well.
    Q3MimeSourceFactory* oldDefaultFactory = Q3MimeSourceFactory::takeDefaultFactory();
    Q3MimeSourceFactory::setDefaultFactory( mimeSourceFactory() );
    if ( oldDefaultFactory ) {
        Q3MimeSourceFactory::addFactory( oldDefaultFactory );
    }

    d->checkAccelerators = new KCheckAccelerators( this );
  }

#ifdef Q_WS_MACX
  if (type() == GuiClient) {
      QPixmap pixmap = KGlobal::iconLoader()->loadIcon( KCmdLineArgs::appName(),
              K3Icon::NoGroup, K3Icon::SizeEnormous, K3Icon::DefaultState, 0L, false );
      if (!pixmap.isNull()) {
          QImage i = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
          for(int y = 0; y < i.height(); y++) {
              uchar *l = i.scanLine(y);
              for(int x = 0; x < i.width(); x+=4)
                  *(l+x) = 255;
          }
          CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
          CGDataProviderRef dp = CGDataProviderCreateWithData(NULL,
                  i.bits(), i.numBytes(), NULL);
          CGImageRef ir = CGImageCreate(i.width(), i.height(), 8, 32, i.bytesPerLine(),
                  cs, kCGImageAlphaNoneSkipFirst, dp,
                  0, 0, kCGRenderingIntentDefault);
          //cleanup
          SetApplicationDockTileImage(ir);
          CGImageRelease(ir);
          CGColorSpaceRelease(cs);
          CGDataProviderRelease(dp);
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
	 "left-to-right languages (as english) or to 'RTL' in right-to-left "
	 "languages (such as Hebrew and Arabic) to get proper widget layout.",
         "LTR" ) == "RTL")
      rtl = !rtl;
  setLayoutDirection( rtl ? Qt::RightToLeft:Qt::LeftToRight);

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + applicationName() + '/');
  pSessionConfig = 0L;
  bSessionManagement = true;

#ifdef Q_WS_X11
  // register a communication window for desktop changes (Matthias)
  if (type() == GuiClient && kde_have_kipc )
  {
    smw = new QWidget();
    long data = 1;
    XChangeProperty(QX11Info::display(), smw->winId(),
		    atom_DesktopWindow, atom_DesktopWindow,
		    32, PropModeReplace, (unsigned char *)&data, 1);
  }
  d->oldIceIOErrorHandler = IceSetIOErrorHandler( kde_ice_ioerrorhandler );
#elif defined(Q_WS_WIN)
  KApplication_init_windows();
#else
  // FIXME(E): Implement for Qt Embedded
#endif
}

static int my_system (const char *command) {
   int pid, status;

   QApplication::flush();
   pid = fork();
   if (pid == -1)
      return -1;
   if (pid == 0) {
      const char* shell = "/bin/sh";
      execl(shell, shell, "-c", command, (void *)0);
      ::_exit(127);
   }
   do {
      if (waitpid(pid, &status, 0) == -1) {
         if (errno != EINTR)
            return -1;
       } else
            return status;
   } while(1);
}

KConfig* KApplication::sessionConfig()
{
    if (!pSessionConfig) // create an instance specific config object
        pSessionConfig = new KConfig( sessionConfigName(), false, false);
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

KSessionManaged::KSessionManaged()
{
    sessionClients()->removeAll( this );
    sessionClients()->append( this );
}

KSessionManaged::~KSessionManaged()
{
    sessionClients()->removeAll( this );
}

bool KSessionManaged::saveState(QSessionManager&)
{
    return true;
}

bool KSessionManaged::commitData(QSessionManager&)
{
    return true;
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

    foreach (KSessionManaged *it, *sessionClients()) {
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
            restartCommand.append("-display");
            restartCommand.append(displayname);
        }
        sm.setRestartCommand( restartCommand );
    }


    // finally: do session management
    emit saveYourself(); // for compatibility
    bool canceled = false;
    foreach(KSessionManaged* it, *sessionClients()) {
      if(canceled) break;
      canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( pSessionConfig ) {
        pSessionConfig->sync();
        QStringList discard;
        discard  << "rm" << locateLocal("config", sessionConfigName());
        sm.setDiscardCommand( discard );
    } else {
	sm.setDiscardCommand( QStringList( "" ) );
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

void KApplication::startKdeinit()
{
#ifndef Q_WS_WIN //TODO
  // Try to launch kdeinit.
  QString srv = KStandardDirs::findExe(QLatin1String("kdeinit"));
  if (srv.isEmpty())
     srv = KStandardDirs::findExe(QLatin1String("kdeinit"), KGlobal::dirs()->kfsstnd_defaultbindir());
  if (srv.isEmpty())
     return;
  if (kapp && (Tty != kapp->type()))
    setOverrideCursor( Qt::WaitCursor );
  my_system(QFile::encodeName(srv)+" --suicide");
  if (kapp && (Tty != kapp->type()))
    restoreOverrideCursor();
#endif
}

void KApplication::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if ( type() != Tty ) {
        if (args && args->isSet("icon"))
        {
            QPixmap largeIcon = DesktopIcon(args->getOption("icon"));
            QIcon icon = windowIcon();
            icon.addPixmap(largeIcon, QIcon::Normal, QIcon::On);
            setWindowIcon(icon);
        }
        else {
            QIcon icon = windowIcon();
            QPixmap largeIcon = DesktopIcon(instanceName());
            icon.addPixmap(largeIcon, QIcon::Normal, QIcon::On);
            setWindowIcon(icon);
        }
    }

    if (!args)
        return;

    if (args->isSet("config"))
    {
        QString config = QString::fromLocal8Bit(args->getOption("config"));
        setConfigName(config);
    }

    if (args->isSet("style"))
    {

       QStringList styles = QStyleFactory::keys();
       QString reqStyle(args->getOption("style").toLower());

	   for (QStringList::ConstIterator it = styles.begin(); it != styles.end(); ++it)
		   if ((*it).toLower() == reqStyle)
		   {
			   d->overrideStyle = *it;
			   break;
		   }

       if (d->overrideStyle.isEmpty())
          fprintf(stderr, "%s", i18n("The style %1 was not found\n", reqStyle).toLocal8Bit().data());
    }

    bool nocrashhandler = (getenv("KDE_DEBUG") != NULL);
    if (!nocrashhandler && args->isSet("crashhandler"))
    {
        // set default crash handler / set emergency save function to nothing
        KCrash::setCrashHandler(KCrash::defaultCrashHandler);
        KCrash::setEmergencySaveFunction(NULL);

        KCrash::setApplicationName(QString(args->appName()));
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
        d->sessionKey = args->getOption("smkey");
    }

}

extern void kDebugCleanup();

KApplication::~KApplication()
{
  // First call the static deleters and then call KLibLoader::cleanup()
  // The static deleters may delete libraries for which they need KLibLoader.
  // KLibLoader will take care of the remaining ones.
  KGlobal::deleteStaticDeleters();
  KLibLoader::cleanUp();

  delete smw;

  KProcessController::deref();

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



static bool kapp_block_user_input = false;

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

    if ( kapp_block_user_input ) {
        switch ( _event->type  ) {
        case ButtonPress:
        case ButtonRelease:
        case XKeyPress:
        case XKeyRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            return true;
        default:
            break;
        }
    }

    if (x11Filter) {
        foreach (const QWidget *w, *x11Filter) {
            if (((KAppX11HackWidget*) w)->publicx11Event(_event))
                return true;
        }
    }

    if ((_event->type == ClientMessage) &&
            (_event->xclient.message_type == kipcCommAtom))
    {
        XClientMessageEvent *cme = (XClientMessageEvent *) _event;

        int id = cme->data.l[0];
        int arg = cme->data.l[1];
        if ((id < 32) && (kipcEventMask & (1 << id)))
        {
            switch (id)
            {
            case KIPC::StyleChanged:
                KGlobal::config()->reparseConfiguration();
                kdisplaySetStyle();
                break;

            case KIPC::ToolbarStyleChanged:
                KGlobal::config()->reparseConfiguration();
                emit toolbarAppearanceChanged(arg);
                break;

            case KIPC::PaletteChanged:
                KGlobal::config()->reparseConfiguration();
                kdisplaySetPalette();
                break;

            case KIPC::FontChanged:
                KGlobal::config()->reparseConfiguration();
                KGlobalSettings::rereadFontSettings();
                kdisplaySetFont();
                break;

            case KIPC::BackgroundChanged:
                emit backgroundChanged(arg);
                break;

            case KIPC::SettingsChanged:
                KGlobal::config()->reparseConfiguration();
                if (arg == SETTINGS_PATHS)
                    KGlobalSettings::rereadPathSettings();
                else if (arg == SETTINGS_MOUSE)
                    KGlobalSettings::rereadMouseSettings();
                propagateSettings((SettingsCategory)arg);
                break;

            case KIPC::IconChanged:
                QPixmapCache::clear();
                KGlobal::config()->reparseConfiguration();
                KGlobal::instance()->newIconLoader();
                emit iconChanged(arg);
                break;

            case KIPC::ClipboardConfigChanged:
                KClipboardSynchronizer::newConfiguration(arg);
                break;

            case KIPC::BlockShortcuts:
                // FIXME KAccel port
                //KGlobalAccel::blockShortcuts(arg);
                emit kipcMessage(id, arg); // some apps may do additional things
                break;
            }
        }
        else if (id >= 32)
        {
            emit kipcMessage(id, arg);
        }
        return true;
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
    QDBusInterfacePtr(service, "/MainApplication", "org.kde.KApplication")->call("updateUserTimestamp", time);
#endif
}

void KApplication::addKipcEventMask(int id)
{
    if (id >= 32)
    {
        kDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask |= (1 << id);
}

void KApplication::removeKipcEventMask(int id)
{
    if (id >= 32)
    {
        kDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask &= ~(1 << id);
}

void KApplication::applyGUIStyle()
{
    KConfigGroup pConfig (KGlobal::config(), "General");
#ifdef Q_WS_MACX
    QString defaultStyle = "macintosh";
#else
    QString defaultStyle = "plastique";// = KStyle::defaultStyle(); ### wait for KStyle4
#endif
    QString styleStr = pConfig.readEntry("widgetStyle", defaultStyle);

    if (d->overrideStyle.isEmpty()) {
      // ### add check whether we already use the correct style to return then
      // (workaround for Qt misbehavior to avoid double style initialization)

      QStyle* sp = QStyleFactory::create( styleStr );

      // If there is no default style available, try falling back any available style
      if ( !sp && styleStr != defaultStyle)
          sp = QStyleFactory::create( defaultStyle );
      if ( !sp )
          sp = QStyleFactory::create( *(QStyleFactory::keys().begin()) );
      setStyle(sp);
    }
    else
        setStyle(d->overrideStyle);
    // Reread palette from config file.
    kdisplaySetPalette();
}

QPalette KApplication::createApplicationPalette()
{
    KConfigGroup cg( KGlobal::config(), "General" );
    return createApplicationPalette( &cg, KGlobalSettings::contrast() );
}

QPalette KApplication::createApplicationPalette( KConfigBase *config, int contrast_ )
{
    QColor kde34Background( 239, 239, 239 );
    QColor kde34Blue( 103,141,178 );

    QColor kde34Button;
    if ( QPixmap::defaultDepth() > 8 )
      kde34Button.setRgb( 221, 223, 228 );
    else
      kde34Button.setRgb( 220, 220, 220 );

    QColor kde34Link( 0, 0, 238 );
    QColor kde34VisitedLink( 82, 24, 139 );

    QColor background = qvariant_cast<QColor>(config->readEntry( "background", kde34Background ));
    QColor foreground = qvariant_cast<QColor>(config->readEntry( "foreground", QColor(Qt::black) ));
    QColor button = qvariant_cast<QColor>(config->readEntry( "buttonBackground", kde34Button ));
    QColor buttonText = qvariant_cast<QColor>(config->readEntry( "buttonForeground", QColor(Qt::black) ));
    QColor highlight = qvariant_cast<QColor>(config->readEntry( "selectBackground", kde34Blue ));
    QColor highlightedText = qvariant_cast<QColor>(config->readEntry( "selectForeground", QColor(Qt::white) ));
    QColor base = qvariant_cast<QColor>(config->readEntry( "windowBackground", QColor(Qt::white) ));
    QColor baseText = qvariant_cast<QColor>(config->readEntry( "windowForeground", QColor(Qt::black) ));
    QColor link = qvariant_cast<QColor>(config->readEntry( "linkColor", kde34Link ));
    QColor visitedLink = qvariant_cast<QColor>(config->readEntry( "visitedLinkColor", kde34VisitedLink ));

    int highlightVal, lowlightVal;
    highlightVal = 100 + (2*contrast_+4)*16/10;
    lowlightVal = 100 + (2*contrast_+4)*10;

    QColor disfg = foreground;

    int h, s, v;
    disfg.getHsv( &h, &s, &v );
    if (v > 128)
        // dark bg, light fg - need a darker disabled fg
        disfg = disfg.dark(lowlightVal);
    else if (disfg != Qt::black)
        // light bg, dark fg - need a lighter disabled fg - but only if !black
        disfg = disfg.light(highlightVal);
    else
        // black fg - use darkgray disabled fg
        disfg = Qt::darkGray;

    QPalette palette;
    palette.setColor( QPalette::Active, QPalette::Foreground, foreground );
    palette.setColor( QPalette::Active, QPalette::Window, background );
    palette.setColor( QPalette::Active, QPalette::Light, background.light( highlightVal ) );
    palette.setColor( QPalette::Active, QPalette::Dark, background.dark( lowlightVal ) );
    palette.setColor( QPalette::Active, QPalette::Midlight, background.dark( 120 ) );
    palette.setColor( QPalette::Active, QPalette::Text, baseText );
    palette.setColor( QPalette::Active, QPalette::Base, base );

    palette.setColor( QPalette::Active, QPalette::Highlight, highlight );
    palette.setColor( QPalette::Active, QPalette::HighlightedText, highlightedText );
    palette.setColor( QPalette::Active, QPalette::Button, button );
    palette.setColor( QPalette::Active, QPalette::ButtonText, buttonText );
    palette.setColor( QPalette::Active, QPalette::Midlight, background.light( 110 ) );
    palette.setColor( QPalette::Active, QPalette::Link, link );
    palette.setColor( QPalette::Active, QPalette::LinkVisited, visitedLink );

    palette.setColor( QPalette::Disabled, QPalette::Foreground, disfg );
    palette.setColor( QPalette::Disabled, QPalette::Window, background );
    palette.setColor( QPalette::Disabled, QPalette::Light, background.light( highlightVal ) );
    palette.setColor( QPalette::Disabled, QPalette::Dark, background.dark( lowlightVal ) );
    palette.setColor( QPalette::Disabled, QPalette::Midlight, background.dark( 120 ) );
    palette.setColor( QPalette::Disabled, QPalette::Text, background.dark( 120 ) );
    palette.setColor( QPalette::Disabled, QPalette::Base, base );
    palette.setColor( QPalette::Disabled, QPalette::Button, button );


    int inlowlightVal = lowlightVal-25;
    if (inlowlightVal < 120)
        inlowlightVal = 120;

    QColor disbtntext = buttonText;
    disbtntext.getHsv( &h, &s, &v );
    if (v > 128)
        // dark button, light buttonText - need a darker disabled buttonText
        disbtntext = disbtntext.dark(lowlightVal);
    else if (disbtntext != Qt::black)
        // light buttonText, dark button - need a lighter disabled buttonText - but only if !black
        disbtntext = disbtntext.light(highlightVal);
    else
        // black button - use darkgray disabled buttonText
        disbtntext = Qt::darkGray;

    palette.setColor( QPalette::Disabled, QPalette::Highlight, highlight.dark( 120 ) );
    palette.setColor( QPalette::Disabled, QPalette::ButtonText, disbtntext );
    palette.setColor( QPalette::Disabled, QPalette::Midlight, background.light( 110 ) );
    palette.setColor( QPalette::Disabled, QPalette::Link, link );
    palette.setColor( QPalette::Disabled, QPalette::LinkVisited, visitedLink );

    return palette;
}


void KApplication::kdisplaySetPalette()
{
#ifdef Q_WS_MACX
    //Can I have this on other platforms, please!? --Sam
    {
        KConfigGroup cg( KGlobal::config(), "General" );
        bool do_not_set_palette = false;
        if(cg.readEntry("nopaletteChange", QVariant(&do_not_set_palette)).toBool())
            return;
    }
#endif
    QApplication::setPalette( createApplicationPalette() );
    emit kdisplayPaletteChanged();
    emit appearanceChanged();
}


void KApplication::kdisplaySetFont()
{
    QApplication::setFont(KGlobalSettings::generalFont());
    QApplication::setFont(KGlobalSettings::menuFont(), "QMenuBar");
    QApplication::setFont(KGlobalSettings::menuFont(), "QPopupMenu");
    QApplication::setFont(KGlobalSettings::menuFont(), "KPopupTitle");

    // "patch" standard QStyleSheet to follow our fonts
    Q3StyleSheet* sheet = Q3StyleSheet::defaultSheet();
    sheet->item ("pre")->setFontFamily (KGlobalSettings::fixedFont().family());
    sheet->item ("code")->setFontFamily (KGlobalSettings::fixedFont().family());
    sheet->item ("tt")->setFontFamily (KGlobalSettings::fixedFont().family());

    emit kdisplayFontChanged();
    emit appearanceChanged();
}


void KApplication::kdisplaySetStyle()
{
    applyGUIStyle();
    emit kdisplayStyleChanged();
    emit appearanceChanged();
}


void KApplication::propagateSettings(SettingsCategory arg)
{
    KConfigGroup cg( KGlobal::config(), "KDE" );

    int num = cg.readEntry("CursorBlinkRate", QApplication::cursorFlashTime());
    if ((num != 0) && (num < 200))
        num = 200;
    if (num > 2000)
        num = 2000;
    QApplication::setCursorFlashTime(num);
    num = cg.readEntry("DoubleClickInterval", QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);
    num = cg.readEntry("StartDragTime", QApplication::startDragTime());
    QApplication::setStartDragTime(num);
    num = cg.readEntry("StartDragDist", QApplication::startDragDistance());
    QApplication::setStartDragDistance(num);
    num = cg.readEntry("WheelScrollLines", QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    bool b = cg.readEntry("EffectAnimateMenu", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, b);
    b = cg.readEntry("EffectFadeMenu", false);
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, b);
    b = cg.readEntry("EffectAnimateCombo", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, b);
    b = cg.readEntry("EffectAnimateTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, b);
    b = cg.readEntry("EffectFadeTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, b);
    //b = !cg.readEntry("EffectNoTooltip", false);
    //QToolTip::setGloballyEnabled( b ); ###

    emit settingsChanged(arg);
}

QString KApplication::tempSaveName( const QString& pFilename )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = QFileInfo( QDir( "." ), pFilename ).absoluteFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homePath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absolutePath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absolutePath() );

  return aFilename;
}


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover )
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = QFileInfo( QDir( "." ), pFilename ).absoluteFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homePath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absolutePath() ) )
        {
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absolutePath() );

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
        topWidget->setWindowTitle( KInstance::caption() );
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

void KApplication::virtual_hook( int id, void* data )
{ KInstance::virtual_hook( id, data ); }

void KSessionManaged::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kapplication.moc"

