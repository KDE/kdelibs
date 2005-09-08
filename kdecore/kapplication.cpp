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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
        */

#include "config.h"

#undef QT_NO_TRANSLATION
#include <qtranslator.h>
#define QT_NO_TRANSLATION
#include <q3stylesheet.h> // no equivilant in Qt4
#include <qtextedit.h>
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
#ifndef QT_NO_SQL
#include <q3sqlpropertymap.h> // no equivilant in Qt4
#endif

#include <QList>

#undef QT_NO_TRANSLATION
#include "kapplication.h"
#define QT_NO_TRANSLATION
#include "kaboutdata.h"
#include "kaccel.h"
#include "kauthorized.h"
#include "kcheckaccelerators.h"
#include "kclipboard.h"
#include "kcmdlineargs.h"
#include "kcodecs.h"
#include "kconfig.h"
#include "kcrash.h"
#include "kdatastream.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kglobalaccel.h"
#include "kglobalsettings.h"
#include "kiconloader.h"
#include "kkeynative.h"
#include "klibloader.h"
#include "klocale.h"
#include "kmacroexpander.h"
#include "kmath.h"
#include "kmimesourcefactory.h"
#include "kprotocolinfo.h"
#include "kshell.h"
#include "ksimpleconfig.h"
#include "kstandarddirs.h"
#include "kstdaccel.h"
#include "kstringhandler.h"
#include "krandom.h"

#if defined Q_WS_X11
#include <QtGui/qx11info_x11.h>
#include <kstartupinfo.h>
#endif

#include <dcopclient.h>
#include <dcopref.h>

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
#endif

#ifndef Q_WS_WIN
#include <KDE-ICE/ICElib.h>
#else
typedef void* IceIOErrorHandler;
#include <windows.h>
//KDE4: remove
#define Button1Mask (1<<8)
#define Button2Mask (1<<9)
#define Button3Mask (1<<10)
#endif

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#endif

#if defined Q_WS_X11
#include <kipc.h>
#endif

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#include <qimage.h>
#endif

#include "kappdcopiface.h"
#include <qevent.h>
#include <QX11Info>
#include <QDesktopWidget>
#include <QMetaObject>

// exported for kdm kfrontend
KDE_EXPORT bool kde_have_kipc = true; // magic hook to disable kipc in kdm
bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;
static DCOPClient* s_DCOPClient = 0L;
static bool s_dcopClientNeedsPostInit = false;

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
void KApplication_init_windows(bool GUIenabled);

class QAssistantClient;
#endif

/*
  Private data to make keeping binary compatibility easier
 */
class KApplication::Private
{
public:
  Private()
    :   refCount( 1 ),
	oldIceIOErrorHandler( 0 ),
	checkAccelerators( 0 ),
	overrideStyle( QString::null ),
	startup_id( "0" ),
	app_started_timer( NULL ),
	m_KAppDCOPInterface( 0L ),
	session_save( false )
#ifdef Q_WS_X11
	,oldXErrorHandler( NULL )
	,oldXIOErrorHandler( NULL )
#elif defined Q_WS_WIN
	,qassistantclient( 0 )
#endif
  {
  }

  ~Private()
  {
#ifdef Q_WS_WIN
     delete qassistantclient;
#endif
  }


  bool guiEnabled : 1;
  /**
   * This counter indicates when to exit the application.
   * It starts at 1, is decremented in KMainWindow when the last window is closed, but
   * is incremented by operations that should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  int refCount;
  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  QString overrideStyle;
  QString geometry_arg;
  QByteArray startup_id;
  QTimer* app_started_timer;
  KAppDCOPInterface *m_KAppDCOPInterface;
  bool session_save;
#ifdef Q_WS_X11
  int (*oldXErrorHandler)(Display*,XErrorEvent*);
  int (*oldXIOErrorHandler)(Display*);
#elif defined Q_WS_WIN
  QAssistantClient* qassistantclient;
#endif

    QString sessionKey;
    QString pSessionConfigFile;
};


static QList<const QWidget*> *x11Filter = 0;
static bool autoDcopRegistration = true;

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

// FIXME: remove this when we've get a better method of
// customizing accelerator handling -- hopefully in Qt.
// For now, this is set whenever an accelerator is overridden
// in KAccelEventHandler so that the AccelOverride isn't sent twice. -- ellis, 19/10/02
extern bool kde_g_bKillAccelOverride;

bool KApplication::notify(QObject *receiver, QEvent *event)
{
    QEvent::Type t = event->type();
    if (kde_g_bKillAccelOverride)
    {
       kde_g_bKillAccelOverride = false;
       // Indicate that the accelerator has been overridden.
       if (t == QEvent::ShortcutOverride)
       {
          static_cast<QKeyEvent *>(event)->accept();
          return true;
       }
       else
          kdWarning(125) << "kde_g_bKillAccelOverride set, but received an event other than AccelOverride." << endl;
    }

    if ((t == QEvent::ShortcutOverride) || (t == QEvent::KeyPress))
    {
       static const KShortcut& _selectAll = KStdAccel::selectAll();
       QLineEdit *edit = ::qobject_cast<QLineEdit *>(receiver);
       if (edit)
       {
          // We have a keypress for a lineedit...
          QKeyEvent *kevent = static_cast<QKeyEvent *>(event);
          KKey key(kevent);
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
          if (key == KKey(Qt::CTRL + Qt::Key_U))
          {
             if (t == QEvent::KeyPress)
             {
                if (!edit->isReadOnly())
                {
                   QString t(edit->text());
                   t = t.mid(edit->cursorPosition());
                   edit->validateAndSet(t, 0, 0, 0);
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
          if (_selectAll.contains(KKey(kevent)))
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
            if( !d->app_started_timer->isActive())
                d->app_started_timer->start( 0, true );
        }
        if( w->isTopLevel() && ( w->windowIcon().isNull()))
        {
            // icon() cannot be null pixmap, it'll be the "unknown" icon - so check if there is this application icon
            static QPixmap* ic = NULL;
            if( ic == NULL )
                ic = new QPixmap( KGlobal::iconLoader()->loadIcon( iconName(),
                    KIcon::NoGroup, 0, KIcon::DefaultState, NULL, true ));
            if( !ic->isNull())
            {
                w->setWindowIcon( *ic );
#if defined Q_WS_X11
                KWin::setIcons( w->winId(), *ic, miniIcon());
#endif
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
    QString sessKey = sessionKey();
    if ( sessKey.isEmpty() && !d->sessionKey.isEmpty() )
        sessKey = d->sessionKey;
    return QString("session/%1_%2_%3").arg(name()).arg(sessionId()).arg(sessKey);
}

#ifdef Q_WS_X11
static SmcConn mySmcConnection = 0;
static SmcConn tmpSmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif
static QTime* smModificationTime = 0;

KApplication::KApplication( bool allowStyles, bool GUIenabled ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( KCmdLineArgs::about), d (new Private)
{
    pIcon = 0L;
    pMiniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    installSigpipeHandler();
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

#ifdef Q_WS_X11
KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap,
		            bool allowStyles ) :
  QApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual, colormap ),
  KInstance( KCmdLineArgs::about), d (new Private)
{
    pIcon = 0L;
    pMiniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( Display *dpy, Qt::HANDLE visual, Qt::HANDLE colormap,
		            bool allowStyles, KInstance * _instance ) :
  QApplication( dpy, *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                visual, colormap ),
  KInstance( _instance ), d (new Private)
{
    pIcon = 0L;
    pMiniIcon = 0L;
    read_app_startup_id();
    useStyles = allowStyles;
    setName( instanceName() );
    installSigpipeHandler();
    parseCommandLine( );
    init( true );
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}
#endif

KApplication::KApplication( bool allowStyles, bool GUIenabled, KInstance* _instance ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( _instance ), d (new Private)
{
    pIcon = 0L;
    pMiniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    installSigpipeHandler();
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName,
                           bool allowStyles, bool GUIenabled ) :
  QApplication( display ), KInstance(rAppName), d (new Private)
{
    pIcon = 0L;
    pMiniIcon = 0L;
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;

    Q_ASSERT (!rAppName.isEmpty());
    setName(rAppName);

    installSigpipeHandler();
    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}
#endif

int KApplication::xioErrhandler( Display* dpy )
{
    if(kapp)
    {
        emit shutDown();
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
    emit shutDown();

#ifdef Q_WS_X11
    if ( d->oldIceIOErrorHandler != NULL )
      (*d->oldIceIOErrorHandler)( conn );
#endif
    exit( 1 );
}

class KDETranslator : public QTranslator
{
public:
  KDETranslator(QObject *parent) : QTranslator(parent, "kdetranslator") {}
  virtual QString translate(const char* context,
					 const char *sourceText,
					 const char* message) const
  {
    return KGlobal::locale()->translateQt(context, sourceText, message);
  }
};

void KApplication::init(bool GUIenabled)
{
  d->guiEnabled = GUIenabled;
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
  Q_ASSERT( KGlobal::instance()->aboutData() );
  setApplicationName( KGlobal::instance()->aboutData()->appName());



#ifdef Q_WS_X11 //FIXME(E)
  // create all required atoms in _one_ roundtrip to the X server
  if ( GUIenabled ) {
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

  dcopAutoRegistration();
  dcopClientPostInit();

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
  KAuthorized *authorized=KAuthorized::self();
  QByteArray readOnly = getenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && (qstrcmp(name(), "kdialog") != 0))
  {
    if (authorized->authorize("warn_unwritable_config"))
       config->checkConfigFilesWritable(true);
  }

  if (GUIenabled)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    d->oldXErrorHandler = XSetErrorHandler( kde_x_errhandler );
    d->oldXIOErrorHandler = XSetIOErrorHandler( kde_xio_errhandler );
#endif

    connect( this, SIGNAL( aboutToQuit() ), this, SIGNAL( shutDown() ) );

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
  if (GUIenabled) {
      QPixmap pixmap = KGlobal::iconLoader()->loadIcon( KCmdLineArgs::appName(),
              KIcon::NoGroup, KIcon::SizeLarge, KIcon::DefaultState, 0L, false );
      if (!pixmap.isNull()) {
          QImage i = pixmap.convertToImage().convertDepth(32).smoothScale(40, 40);
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
  bool rtl = reverseLayout();
  installTranslator(new KDETranslator(this));
  setReverseLayout( rtl );
  if (i18n( "_: Dear Translator! Translate this string to the string 'LTR' in "
	 "left-to-right languages (as english) or to 'RTL' in right-to-left "
	 "languages (such as Hebrew and Arabic) to get proper widget layout." ) == "RTL")
	setReverseLayout( !rtl );

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + QString::fromLatin1(name()) + '/');
  pSessionConfig = 0L;
  bSessionManagement = true;

#ifdef Q_WS_X11
  // register a communication window for desktop changes (Matthias)
  if (GUIenabled && kde_have_kipc )
  {
    smw = new QWidget(0,0);
    long data = 1;
    XChangeProperty(QX11Info::display(), smw->winId(),
		    atom_DesktopWindow, atom_DesktopWindow,
		    32, PropModeReplace, (unsigned char *)&data, 1);
  }
  d->oldIceIOErrorHandler = IceSetIOErrorHandler( kde_ice_ioerrorhandler );
#elif defined(Q_WS_WIN)
  KApplication_init_windows(GUIenabled);
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
      ::exit(127);
   }
   do {
      if (waitpid(pid, &status, 0) == -1) {
         if (errno != EINTR)
            return -1;
       } else
            return status;
   } while(1);
}


DCOPClient *KApplication::dcopClient()
{
  if (s_DCOPClient)
    return s_DCOPClient;

  s_DCOPClient = new DCOPClient;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
  if (args && args->isSet("dcopserver"))
  {
    s_DCOPClient->setServerAddress( args->getOption("dcopserver"));
  }
  if( kapp ) {
    connect(s_DCOPClient, SIGNAL(attachFailed(const QString &)),
            kapp, SLOT(dcopFailure(const QString &)));
    connect(s_DCOPClient, SIGNAL(blockUserInput(bool) ),
            kapp, SLOT(dcopBlockUserInput(bool)) );
  }
  else
    s_dcopClientNeedsPostInit = true;

  DCOPClient::setMainClient( s_DCOPClient );
  return s_DCOPClient;
}

void KApplication::dcopClientPostInit()
{
  if( s_dcopClientNeedsPostInit )
    {
    s_dcopClientNeedsPostInit = false;
    connect(s_DCOPClient, SIGNAL(blockUserInput(bool) ),
            SLOT(dcopBlockUserInput(bool)) );
    s_DCOPClient->bindToApp(); // Make sure we get events from the DCOPClient.
    }
}

void KApplication::dcopAutoRegistration()
{
  if (autoDcopRegistration)
     {
     ( void ) dcopClient();
     if( dcopClient()->appId().isEmpty())
         dcopClient()->registerAs(name());
     }
}

void KApplication::disableAutoDcopRegistration()
{
  autoDcopRegistration = false;
}

KConfig* KApplication::sessionConfig()
{
    if (!pSessionConfig) // create an instance specific config object
        pSessionConfig = new KConfig( sessionConfigName(), false, false);
    return pSessionConfig;
}

void KApplication::ref()
{
    d->refCount++;
    //kdDebug() << "KApplication::ref() : refCount = " << d->refCount << endl;
}

void KApplication::deref()
{
    d->refCount--;
    //kdDebug() << "KApplication::deref() : refCount = " << d->refCount << endl;
    if ( d->refCount <= 0 )
        quit();
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


bool KApplication::requestShutDown(
    ShutdownConfirm confirm, ShutdownType sdtype, ShutdownMode sdmode )
{
#ifdef Q_WS_X11
    QApplication::syncX();
    /*  use ksmserver's dcop interface if necessary  */
    if ( confirm == ShutdownConfirmYes ||
         sdtype != ShutdownTypeDefault ||
         sdmode != ShutdownModeDefault )
    {
        QByteArray data;
        QDataStream arg(&data, QIODevice::WriteOnly);
        arg << (int)confirm << (int)sdtype << (int)sdmode;
	return dcopClient()->send( "ksmserver", "ksmserver",
                                   "logout(int,int,int)", data );
    }

    if ( mySmcConnection ) {
        // we already have a connection to the session manager, use it.
        SmcRequestSaveYourself( mySmcConnection, SmSaveBoth, True,
				SmInteractStyleAny,
				confirm == ShutdownConfirmNo, True );

	// flush the request
	IceFlush(SmcGetIceConnection(mySmcConnection));
        return true;
    }

    // open a temporary connection, if possible

    propagateSessionManager();
    QByteArray smEnv = ::getenv("SESSION_MANAGER");
    if (smEnv.isEmpty())
        return false;

    if (! tmpSmcConnection) {
	char cerror[256];
	char* myId = 0;
	char* prevId = 0;
	SmcCallbacks cb;
	tmpSmcConnection = SmcOpenConnection( 0, 0, 1, 0,
					      0, &cb,
					      prevId,
					      &myId,
					      255,
					      cerror );
	::free( myId ); // it was allocated by C
	if (!tmpSmcConnection )
	    return false;
    }

    SmcRequestSaveYourself( tmpSmcConnection, SmSaveBoth, True,
			    SmInteractStyleAny, False, True );

    // flush the request
    IceFlush(SmcGetIceConnection(tmpSmcConnection));
    return true;
#else
    // FIXME(E): Implement for Qt Embedded
    return false;
#endif
}

void KApplication::propagateSessionManager()
{
#ifdef Q_WS_X11
    QByteArray fName = QFile::encodeName(locateLocal("socket", "KSMserver"));
    QString display = QString::fromLocal8Bit( ::getenv(DISPLAY) );
    // strip the screen number from the display
    display.remove(QRegExp("\\.[0-9]+$"));
    int i;
    while( (i = display.indexOf(':')) >= 0)
       display[i] = '_';

    fName += '_';
    fName += display.toLocal8Bit();
    QByteArray smEnv = ::getenv("SESSION_MANAGER");
    bool check = smEnv.isEmpty();
    if ( !check && smModificationTime ) {
         QFileInfo info( fName );
         QTime current = info.lastModified().time();
         check = current > *smModificationTime;
    }
    if ( check ) {
        delete smModificationTime;
        QFile f( fName );
        if ( !f.open( QIODevice::ReadOnly ) )
            return;
        QFileInfo info ( f );
        smModificationTime = new QTime( info.lastModified().time() );
        QTextStream t(&f);
        t.setEncoding( QTextStream::Latin1 );
        QString s = t.readLine();
        f.close();
        ::setenv( "SESSION_MANAGER", s.latin1(), true  );
    }
#endif
}

void KApplication::commitData( QSessionManager& sm )
{
    d->session_save = true;
    bool canceled = false;

    foreach (KSessionManaged *it, *sessionClients()) {
      if(canceled) break;
      canceled = !it->commitData( sm );
    }

    if ( canceled )
        sm.cancel();

    if ( sm.allowsInteraction() ) {
        QWidgetList done;
        QWidgetList list = QApplication::topLevelWidgets();
        bool canceled = false;
        QWidget* w = list.first();
		int count = 0;
        while ( !canceled && w ) {
            if ( !( w->testAttribute( Qt::WA_WState_Hidden )) && !w->inherits("KMainWindow") ) {
                QCloseEvent e;
                sendEvent( w, &e );
                canceled = !e.isAccepted();
                if ( !canceled )
                    done.append( w );
				//grab the new list that was just modified by our closeevent
                list = QApplication::topLevelWidgets();
                w = list.first();
				count = 0;
            } else {
				//loop over the widgets
				count++;
                w = list[ count ];
            }
            while ( w && done.contains( w ) )
				//loop over the widgets
				count++;
                w = list[ count ];
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
        QByteArray displayname = getenv(DISPLAY);
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
  QString srv = KStandardDirs::findExe(QString::fromLatin1("kdeinit"));
  if (srv.isEmpty())
     srv = KStandardDirs::findExe(QString::fromLatin1("kdeinit"), KGlobal::dirs()->kfsstnd_defaultbindir());
  if (srv.isEmpty())
     return;
  if (kapp && (Tty != kapp->type()))
    setOverrideCursor( Qt::WaitCursor );
  my_system(QFile::encodeName(srv)+" --suicide");
  if (kapp && (Tty != kapp->type()))
    restoreOverrideCursor();
#endif
}

void KApplication::dcopFailure(const QString &msg)
{
  static int failureCount = 0;
  failureCount++;
  if (failureCount == 1)
  {
     startKdeinit();
     return;
  }
  if (failureCount == 2)
  {
#ifdef Q_WS_WIN
     KGlobal::config()->setGroup("General");
     if (KGlobal::config()->readBoolEntry("ignoreDCOPFailures", false))
         return;
#endif
     QString msgStr(i18n("There was an error setting up inter-process "
                      "communications for KDE. The message returned "
                      "by the system was:\n\n"));
     msgStr += msg;
     msgStr += i18n("\n\nPlease check that the \"dcopserver\" program is running!");

     if (Tty != kapp->type())
     {
       QMessageBox::critical
         (
           kapp->mainWidget(),
           i18n("DCOP communications error (%1)").arg(kapp->caption()),
           msgStr,
           i18n("&OK")
         );
     }
     else
     {
       fprintf(stderr, "%s\n", msgStr.local8Bit().data());
     }

     return;
  }
}

void KApplication::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if ( !args ) return;

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
          fprintf(stderr, "%s", i18n("The style %1 was not found\n").arg(reqStyle).local8Bit().data());
    }

    if (args->isSet("caption"))
    {
       aCaption = QString::fromLocal8Bit(args->getOption("caption"));
    }

    if (args->isSet("miniicon"))
    {
       const char *tmp = args->getOption("miniicon");
       if (!pMiniIcon) {
         pMiniIcon = new QPixmap;
       }
       *pMiniIcon = SmallIcon(tmp);
       aMiniIconName = tmp;
    }

    if (args->isSet("icon"))
    {
       const char *tmp = args->getOption("icon");
       if (!pIcon) {
          pIcon = new QPixmap;
       }
       *pIcon = DesktopIcon( tmp );
       aIconName = tmp;
       if (!pMiniIcon) {
         pMiniIcon = new QPixmap;
       }
       if (pMiniIcon->isNull())
       {
          *pMiniIcon = SmallIcon( tmp );
          aMiniIconName = tmp;
       }
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

    if (args->isSet("geometry"))
    {
        d->geometry_arg = args->getOption("geometry");
    }

    if (args->isSet("smkey"))
    {
        d->sessionKey = args->getOption("smkey");
    }

}

QString KApplication::geometryArgument() const
{
    return d->geometry_arg;
}

QPixmap KApplication::icon() const
{
  if( !pIcon) {
      pIcon = new QPixmap;
  }
  if( pIcon->isNull()) {
      *pIcon = DesktopIcon( instanceName() );
  }
  return *pIcon;
}

QString KApplication::iconName() const
{
  return aIconName.isNull() ? (QString)instanceName() : aIconName;
}

QPixmap KApplication::miniIcon() const
{
  if (!pMiniIcon) {
      pMiniIcon = new QPixmap;
  }
  if (pMiniIcon->isNull()) {
      *pMiniIcon = SmallIcon( instanceName() );
  }
  return *pMiniIcon;
}

QString KApplication::miniIconName() const
{
  return aMiniIconName.isNull() ? (QString)instanceName() : aMiniIconName;
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
  delete pMiniIcon;
  pMiniIcon = 0L;
  delete pIcon;
  pIcon = 0L;
  delete d->m_KAppDCOPInterface;

  // First call the static deleters and then call KLibLoader::cleanup()
  // The static deleters may delete libraries for which they need KLibLoader.
  // KLibLoader will take care of the remaining ones.
  KGlobal::deleteStaticDeleters();
  KLibLoader::cleanUp();

  delete smw;

  // close down IPC
  delete s_DCOPClient;
  s_DCOPClient = 0L;

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
  delete smModificationTime;
  smModificationTime = 0;

  // close the temporary smc connection
  if (tmpSmcConnection) {
      SmcCloseConnection( tmpSmcConnection, 0, 0 );
      tmpSmcConnection = 0;
  }
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

void KApplication::dcopBlockUserInput( bool b )
{
    kapp_block_user_input = b;
}

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
                        || ( _event->xclient.data.l[ 3 ] - QX11Info::appUserTime() ) < 100000U )
                        { // and the timestamp looks reasonable
                        QX11Info::setAppUserTime(_event->xclient.data.l[ 3 ]); // update our qt_x_user_time from it
                        }
                    }
                else // normal DND, only needed until Qt updates qt_x_user_time from XdndDrop
                    {
                    if( QX11Info::appUserTime() == 0
                        || ( _event->xclient.data.l[ 2 ] - QX11Info::appUserTime() ) < 100000U )
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
                if (useStyles)
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
                KGlobalAccel::blockShortcuts(arg);
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

void KApplication::updateUserTimestamp( quint32 time )
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
        || time - QX11Info::appUserTime() < 1000000000U ) // check time > qt_x_user_time, handle wrapping
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

void KApplication::updateRemoteUserTimestamp( const QByteArray& dcopId, quint32 time )
{
#if defined Q_WS_X11
    if( time == 0 )
        time = QX11Info::appUserTime();
    DCOPRef( dcopId, "MainApplication-Interface" ).call( "updateUserTimestamp", time );
#endif
}

void KApplication::invokeEditSlot( const char *slot )
{
  if ( focusWidget() );
    QMetaObject::invokeMethod( focusWidget(), slot );
}

void KApplication::addKipcEventMask(int id)
{
    if (id >= 32)
    {
        kdDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask |= (1 << id);
}

void KApplication::removeKipcEventMask(int id)
{
    if (id >= 32)
    {
        kdDebug(101) << "Cannot use KIPC event mask for message IDs >= 32\n";
        return;
    }
    kipcEventMask &= ~(1 << id);
}

void KApplication::enableStyles()
{
    if (!useStyles)
    {
        useStyles = true;
        applyGUIStyle();
    }
}

void KApplication::disableStyles()
{
    useStyles = false;
}

void KApplication::applyGUIStyle()
{
    if ( !useStyles ) return;

    KConfigGroup pConfig (KGlobal::config(), "General");
    QString defaultStyle = "plastique";// = KStyle::defaultStyle(); ### wait for KStyle4
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

QString KApplication::caption() const
{
  // Caption set from command line ?
  if( !aCaption.isNull() )
        return aCaption;
  else
      // We have some about data ?
      if ( KGlobal::instance()->aboutData() )
        return KGlobal::instance()->aboutData()->programName();
      else
        // Last resort : application name
        return name();
}


//
// 1999-09-20: Espen Sand
// An attempt to simplify consistent captions.
//
QString KApplication::makeStdCaption( const QString &userCaption,
                                      bool withAppName, bool modified ) const
{
  QString s = userCaption.isEmpty() ? caption() : userCaption;

  // If the document is modified, add '[modified]'.
  if (modified)
      s += QString::fromUtf8(" [") + i18n("modified") + QString::fromUtf8("]");

  if ( !userCaption.isEmpty() ) {
      // Add the application name if:
      // User asked for it, it's not a duplication  and the app name (caption()) is not empty
      if ( withAppName && !caption().isNull() && !userCaption.endsWith(caption())  )
	  s += QString::fromUtf8(" - ") + caption();
  }

  return s;
}

QPalette KApplication::createApplicationPalette()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver( config, "General" );
    return createApplicationPalette( config, KGlobalSettings::contrast() );
}

QPalette KApplication::createApplicationPalette( KConfig *config, int contrast_ )
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

    QColor background = config->readColorEntry( "background", &kde34Background );
    QColor foreground = config->readColorEntry( "foreground", new QColor( Qt::black ) );
    QColor button = config->readColorEntry( "buttonBackground", &kde34Button );
    QColor buttonText = config->readColorEntry( "buttonForeground", new QColor( Qt::black ) );
    QColor highlight = config->readColorEntry( "selectBackground", &kde34Blue );
    QColor highlightedText = config->readColorEntry( "selectForeground", new QColor( Qt::white ) );
    QColor base = config->readColorEntry( "windowBackground", new QColor( Qt::white ) );
    QColor baseText = config->readColorEntry( "windowForeground", new QColor( Qt::black ) );
    QColor link = config->readColorEntry( "linkColor", &kde34Link );
    QColor visitedLink = config->readColorEntry( "visitedLinkColor", &kde34VisitedLink );

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


    QColorGroup disabledgrp(disfg, background,
                            background.light(highlightVal),
                            background.dark(lowlightVal),
                            background.dark(120),
                            background.dark(120), base);

    QColorGroup colgrp(foreground, background, background.light(highlightVal),
                       background.dark(lowlightVal),
                       background.dark(120),
                       baseText, base);

    int inlowlightVal = lowlightVal-25;
    if(inlowlightVal < 120)
        inlowlightVal = 120;

    colgrp.setColor(QColorGroup::Highlight, highlight);
    colgrp.setColor(QColorGroup::HighlightedText, highlightedText);
    colgrp.setColor(QColorGroup::Button, button);
    colgrp.setColor(QColorGroup::ButtonText, buttonText);
    colgrp.setColor(QColorGroup::Midlight, background.light(110));
    colgrp.setColor(QColorGroup::Link, link);
    colgrp.setColor(QColorGroup::LinkVisited, visitedLink);

    disabledgrp.setColor(QColorGroup::Button, button);

    QColor disbtntext = buttonText;
    disbtntext.hsv( &h, &s, &v );
    if (v > 128)
	// dark button, light buttonText - need a darker disabled buttonText
	disbtntext = disbtntext.dark(lowlightVal);
    else if (disbtntext != Qt::black)
	// light buttonText, dark button - need a lighter disabled buttonText - but only if !black
	disbtntext = disbtntext.light(highlightVal);
    else
	// black button - use darkgray disabled buttonText
	disbtntext = Qt::darkGray;

    disabledgrp.setColor(QColorGroup::ButtonText, disbtntext);
    disabledgrp.setColor(QColorGroup::Midlight, background.light(110));
    disabledgrp.setColor(QColorGroup::Highlight, highlight.dark(120));
    disabledgrp.setColor(QColorGroup::Link, link);
    disabledgrp.setColor(QColorGroup::LinkVisited, visitedLink);

    return QPalette(colgrp, disabledgrp, colgrp);
}


void KApplication::kdisplaySetPalette()
{
#ifdef Q_WS_MACX
    //Can I have this on other platforms, please!? --Sam
    {
        KConfig *config = KGlobal::config();
        KConfigGroupSaver saver( config, "General" );
        bool do_not_set_palette = false;
        if(config->readBoolEntry("nopaletteChange", &do_not_set_palette))
            return;
    }
#endif
    QApplication::setPalette( createApplicationPalette(), true);
    emit kdisplayPaletteChanged();
    emit appearanceChanged();
}


void KApplication::kdisplaySetFont()
{
    QApplication::setFont(KGlobalSettings::generalFont(), true);
    QApplication::setFont(KGlobalSettings::menuFont(), true, "QMenuBar");
    QApplication::setFont(KGlobalSettings::menuFont(), true, "QPopupMenu");
    QApplication::setFont(KGlobalSettings::menuFont(), true, "KPopupTitle");

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
    if (useStyles)
    {
        applyGUIStyle();
        emit kdisplayStyleChanged();
        emit appearanceChanged();
    }
}


void KApplication::propagateSettings(SettingsCategory arg)
{
    KConfigBase* config = KGlobal::config();
    KConfigGroupSaver saver( config, "KDE" );

    int num = config->readNumEntry("CursorBlinkRate", QApplication::cursorFlashTime());
    if ((num != 0) && (num < 200))
        num = 200;
    if (num > 2000)
        num = 2000;
    QApplication::setCursorFlashTime(num);
    num = config->readNumEntry("DoubleClickInterval", QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);
    num = config->readNumEntry("StartDragTime", QApplication::startDragTime());
    QApplication::setStartDragTime(num);
    num = config->readNumEntry("StartDragDist", QApplication::startDragDistance());
    QApplication::setStartDragDistance(num);
    num = config->readNumEntry("WheelScrollLines", QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    bool b = config->readBoolEntry("EffectAnimateMenu", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, b);
    b = config->readBoolEntry("EffectFadeMenu", false);
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, b);
    b = config->readBoolEntry("EffectAnimateCombo", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, b);
    b = config->readBoolEntry("EffectAnimateTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, b);
    b = config->readBoolEntry("EffectFadeTooltip", false);
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, b);
    //b = !config->readBoolEntry("EffectNoTooltip", false);
    //QToolTip::setGloballyEnabled( b ); ###

    emit settingsChanged(arg);
}

void KApplication::installKDEPropertyMap()
{
#ifndef QT_NO_SQL
    static bool installed = false;
    if (installed) return;
    installed = true;
    /**
     * If you are adding a widget that was missing please
     * make sure to also add it to KConfigDialogManager's retrieveSettings()
     * function.
     * Thanks.
     */
    // QSqlPropertyMap takes ownership of the new default map.
    Q3SqlPropertyMap *kdeMap = new Q3SqlPropertyMap;
    kdeMap->insert( "KColorButton", "color" );
    kdeMap->insert( "KComboBox", "currentItem" );
    kdeMap->insert( "KDatePicker", "date" );
    kdeMap->insert( "KDateWidget", "date" );
    kdeMap->insert( "KDateTimeWidget", "dateTime" );
    kdeMap->insert( "KEditListBox", "items" );
    kdeMap->insert( "KFontCombo", "family" );
    kdeMap->insert( "KFontRequester", "font" );
    kdeMap->insert( "KFontChooser", "font" );
    kdeMap->insert( "KHistoryCombo", "currentItem" );
    kdeMap->insert( "KListBox", "currentItem" );
    kdeMap->insert( "KLineEdit", "text" );
    kdeMap->insert( "KRestrictedLine", "text" );
    kdeMap->insert( "KSqueezedTextLabel", "text" );
    kdeMap->insert( "KTextBrowser", "source" );
    kdeMap->insert( "KTextEdit", "text" );
    kdeMap->insert( "KURLRequester", "url" );
    kdeMap->insert( "KPasswordEdit", "password" );
    kdeMap->insert( "KIntNumInput", "value" );
    kdeMap->insert( "KIntSpinBox", "value" );
    kdeMap->insert( "KDoubleNumInput", "value" );
    // Temp til fixed in QT then enable ifdef with the correct version num
    kdeMap->insert( "QGroupBox", "checked" );
    kdeMap->insert( "QTabWidget", "currentPage" );
    Q3SqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

void KApplication::cut()
{
  invokeEditSlot( SLOT( cut() ) );
}

void KApplication::copy()
{
  invokeEditSlot( SLOT( copy() ) );
}

void KApplication::paste()
{
  invokeEditSlot( SLOT( paste() ) );
}

void KApplication::clear()
{
  invokeEditSlot( SLOT( clear() ) );
}

void KApplication::selectAll()
{
  invokeEditSlot( SLOT( selectAll() ) );
}

QByteArray
KApplication::launcher()
{
   return "klauncher";
}

QString KApplication::tempSaveName( const QString& pFilename ) const
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
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
        bool& bRecover ) const
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
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


bool checkAccess(const QString& pathname, int mode)
{
  int accessOK = access( QFile::encodeName(pathname), mode );
  if ( accessOK == 0 )
    return true;  // OK, I can really access the file

  // else
  // if we want to write the file would be created. Check, if the
  // user may write to the directory to create the file.
  if ( (mode & W_OK) == 0 )
    return false;   // Check for write access is not part of mode => bail out


  if (!access( QFile::encodeName(pathname), F_OK)) // if it already exists
      return false;

  //strip the filename (everything until '/' from the end
  QString dirName(pathname);
  int pos = dirName.lastIndexOf('/');
  if ( pos == -1 )
    return false;   // No path in argument. This is evil, we won't allow this
  else if ( pos == 0 ) // don't turn e.g. /root into an empty string
      pos = 1;

  dirName.truncate(pos); // strip everything starting from the last '/'

  accessOK = access( QFile::encodeName(dirName), W_OK );
  // -?- Can I write to the accessed diretory
  if ( accessOK == 0 )
    return true;  // Yes
  else
    return false; // No
}

void KApplication::setTopWidget( QWidget *topWidget )
{
  if( !topWidget )
      return;

    // set the specified caption
    if ( !topWidget->inherits("KMainWindow") ) { // KMainWindow does this already for us
        topWidget->setCaption( caption() );
    }

    // set the specified icons
    topWidget->setIcon( icon() ); //standard X11
#if defined Q_WS_X11
//#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    KWin::setIcons(topWidget->winId(), icon(), miniIcon() ); // NET_WM hints for KWin

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

int KApplication::random()
{
   return KRandom::random();
}

QString KApplication::randomString(int length)
{
   return KRandom::randomString(length);
}

Qt::ButtonState KApplication::keyboardMouseState()
{
    int ret = 0;
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint state;
    XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &state );
    // transform the same way like Qt's qt_x11_translateButtonState()
    if( state & Button1Mask )
        ret |= Qt::LeftButton;
    if( state & Button2Mask )
        ret |= Qt::MidButton;
    if( state & Button3Mask )
        ret |= Qt::RightButton;
    if( state & ShiftMask )
        ret |= Qt::ShiftModifier;
    if( state & ControlMask )
        ret |= Qt::ControlModifier;
    if( state & KKeyNative::modXAlt() )
        ret |= Qt::AltModifier;
    if( state & KKeyNative::modXWin() )
        ret |= Qt::MetaModifier;
#elif defined(Q_WS_WIN)
    const bool mousebtn_swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(VK_LBUTTON))
        ret |= (mousebtn_swapped ? Qt::RightButton : Qt::LeftButton);
    if (GetAsyncKeyState(VK_MBUTTON))
        ret |= Qt::MidButton;
    if (GetAsyncKeyState(VK_RBUTTON))
        ret |= (mousebtn_swapped ? Qt::LeftButton : Qt::RightButton);
    if (GetAsyncKeyState(VK_SHIFT))
        ret |= Qt::ShiftModifier;
    if (GetAsyncKeyState(VK_CONTROL))
        ret |= Qt::ControlModifier;
    if (GetAsyncKeyState(VK_MENU))
        ret |= Qt::AltModifier;
    if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN))
        ret |= Qt::MetaModifier;
#else
    //TODO: other platforms
#endif
    return static_cast< Qt::ButtonState >( ret );
}

void KApplication::installSigpipeHandler()
{
#ifdef Q_OS_UNIX
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigemptyset( &act.sa_mask );
    act.sa_flags = 0;
    sigaction( SIGPIPE, &act, 0 );
#endif
}

void KApplication::sigpipeHandler(int)
{
    int saved_errno = errno;
    // Using kdDebug from a signal handler is not a good idea.
#ifndef NDEBUG
    char msg[1000];
    sprintf(msg, "*** SIGPIPE *** (ignored, pid = %ld)\n", (long) getpid());
    write(2, msg, strlen(msg));
#endif

    // Do nothing.
    errno = saved_errno;
}

bool KApplication::guiEnabled()
{
    return kapp && kapp->d->guiEnabled;
}

#include "kapplication.moc"
