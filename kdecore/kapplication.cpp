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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
        */

// $Id$

#include "config.h"

#undef QT_NO_TRANSLATION
#include <qtranslator.h>
#define QT_NO_TRANSLATION
#undef Unsorted
#include <qdir.h>
#include <qptrcollection.h>
#include <qwidgetlist.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qsessionmanager.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qstylesheet.h>
#include <qpixmapcache.h>
#include <qtooltip.h>
#include <qstylefactory.h>
#ifndef QT_NO_SQL
#include <qsqlpropertymap.h>
#endif

#undef QT_NO_TRANSLATION
#include "kapplication.h"
#define QT_NO_TRANSLATION
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kclipboard.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobalsettings.h>
#include <kcrash.h>
#include <kdatastream.h>
#include <klibloader.h>
#include <kmimesourcefactory.h>
#include <kstdaccel.h>
#include <kaccel.h>
#include <kcheckaccelerators.h>
#include <qptrdict.h>

#include <kstartupinfo.h>

#include <dcopclient.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#include "kwin.h"

#include <fcntl.h>
#include <stdlib.h> // getenv(), srand(), rand()
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#ifndef Q_WS_QWS //FIXME(E): NetWM should talk to QWS...
#include <netwm.h>
#endif

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#endif
#include <KDE-ICE/ICElib.h>

#if defined(Q_WS_X11)
// defined by X11 headers
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#endif

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#endif

#include <kipc.h>

#include "kappdcopiface.h"

bool kde_have_kipc = true; // magic hook to disable kipc in kdm

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;
DCOPClient *KApplication::s_DCOPClient = 0L;
bool KApplication::s_dcopClientNeedsPostInit = false;

static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
static Atom atom_KdeNetUserTime;

template class QPtrList<KSessionManaged>;

#ifdef Q_WS_X11
extern "C" {
static int kde_xio_errhandler( Display * )
{
  return kapp->xioErrhandler();
}

static int kde_x_errhandler( Display *dpy, XErrorEvent *err )
{
    char errstr[256];
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    if ( err->error_code != BadWindow )
        kdWarning() << "KDE detected X Error: " << errstr << " " << err->error_code
		<< "\n  Major opcode:  " << err->request_code << endl;
    return 0;
}
}
#endif

extern "C" {
static void kde_ice_ioerrorhandler( IceConn conn )
{
    if(kapp)
        kapp->iceIOErrorHandler( conn );
    // else ignore the error for now
}
}

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
  KApplicationPrivate()
  {
    actionRestrictions = false;
    refCount = 1;
    oldIceIOErrorHandler = 0;
    checkAccelerators = 0;
    overrideStyle=QString::null;
    startup_id = "0";
    m_KAppDCOPInterface = 0L;
  }

  ~KApplicationPrivate()
  {}

  bool actionRestrictions : 1;
  /**
   * This counter indicates when to exit the application.
   * It starts at 1, is decremented by the "last window close" event, but
   * is incremented by operations that should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  int refCount;
  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  QString overrideStyle;
  QString geometry_arg;
  QCString startup_id;
  KAppDCOPInterface *m_KAppDCOPInterface;

#if QT_VERSION < 0x030100
    QString sessionKey;
#endif
    QString pSessionConfigFile;
};


static QPtrList<QWidget>*x11Filter = 0;
static bool autoDcopRegistration = true;

void KApplication::installX11EventFilter( QWidget* filter )
{
    if ( !filter )
        return;
    if (!x11Filter)
        x11Filter = new QPtrList<QWidget>;
    connect ( filter, SIGNAL( destroyed() ), this, SLOT( x11FilterDestroyed() ) );
    x11Filter->append( filter );
}

void KApplication::x11FilterDestroyed()
{
    if ( !x11Filter || !sender() )
        return;
    QWidget *w = static_cast<QWidget *>(const_cast<QObject *>(sender()));
    x11Filter->removeRef( w );
    if ( x11Filter->isEmpty() ) {
        delete x11Filter;
        x11Filter = 0;
    }
}

bool KApplication::notify(QObject *receiver, QEvent *event)
{
    QEvent::Type t = event->type();
    if ((t == QEvent::AccelOverride) || (t == QEvent::KeyPress))
    {
       static const KShortcut& _selectAll = KStdAccel::selectAll();
       if (receiver && receiver->inherits("QLineEdit"))
       {
          QLineEdit *edit = static_cast<QLineEdit *>(receiver);
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
       if (receiver && receiver->inherits("QTextEdit"))
       {
          QTextEdit *medit = static_cast<QTextEdit *>(receiver);
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
    return QApplication::notify(receiver, event);
}


// the help class for session management communication
static QPtrList<KSessionManaged>* sessionClients()
{
    static QPtrList<KSessionManaged>* session_clients = 0L;
    if ( !session_clients )
        session_clients = new QPtrList<KSessionManaged>;
    return session_clients;
}

/*
  Auxiliary function to calculate a a session config name used for the
  instance specific config object.
  Syntax:  "session/<appname>_<sessionId>"
 */
QString KApplication::sessionConfigName() const
{
#if QT_VERSION < 0x030100
    return QString("session/%1_%2_%3").arg(name()).arg(sessionId()).arg(d->sessionKey);
#else
    return QString("session/%1_%2_%3").arg(name()).arg(sessionId()).arg(sessionKey());
#endif
}

#ifndef Q_WS_QWS
static SmcConn mySmcConnection = 0;
static SmcConn tmpSmcConnection = 0;
#else
// FIXME(E): Implement for Qt Embedded
// Possibly "steal" XFree86's libSM?
#endif
static QTime* smModificationTime = 0;

KApplication::KApplication( int& argc, char** argv, const QCString& rAppName,
                            bool allowStyles, bool GUIenabled ) :
  QApplication( argc, argv, GUIenabled ), KInstance(rAppName),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate())
{
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    Q_ASSERT (!rAppName.isEmpty());
    setName(rAppName);

    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( bool allowStyles, bool GUIenabled ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( KCmdLineArgs::about),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate)
{
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}

KApplication::KApplication( bool allowStyles, bool GUIenabled, KInstance* _instance ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( _instance ),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate)
{
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;
    setName( instanceName() );

    parseCommandLine( );
    init(GUIenabled);
}

#ifdef Q_WS_X11
KApplication::KApplication(Display *display, int& argc, char** argv, const QCString& rAppName,
                           bool allowStyles, bool GUIenabled ) :
  QApplication( display ), KInstance(rAppName),
  display(0L),
  d (new KApplicationPrivate())
{
    read_app_startup_id();
    if (!GUIenabled)
       allowStyles = false;
    useStyles = allowStyles;

    Q_ASSERT (!rAppName.isEmpty());
    setName(rAppName);

    KCmdLineArgs::initIgnore(argc, argv, rAppName.data());
    parseCommandLine( );
    init(GUIenabled);
    d->m_KAppDCOPInterface = new KAppDCOPInterface(this);
}
#endif

int KApplication::xioErrhandler()
{
    if(kapp)
    {
        emit shutDown();
        exit( 1 );
    }
  return 0;
}

void KApplication::iceIOErrorHandler( _IceConn *conn )
{
    emit shutDown();

    if ( d->oldIceIOErrorHandler != NULL )
      (*d->oldIceIOErrorHandler)( conn );

    exit( 1 );
}

class KDETranslator : public QTranslator
{
public:
  KDETranslator(QObject *parent) : QTranslator(parent, "kdetranslator") {}
  virtual QTranslatorMessage findMessage(const char* context,
					 const char *sourceText,
					 const char* message) const
  {
    QTranslatorMessage res;
    res.setTranslation(KGlobal::locale()->translateQt(context, sourceText, message));
    return res;
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

  if( KProcessController::theKProcessController == 0 )
      (void) new KProcessController();

  (void) KClipboard::self();
  
  QApplication::setDesktopSettingsAware( false );

  KApp = this;


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

      atoms[n] = &atom_KdeNetUserTime;
      names[n++] = (char *) "_KDE_NET_USER_TIME";

      XInternAtoms( qt_xdisplay(), names, n, FALSE, atoms_return );
      for (int i = 0; i < n; i++ )
	  *atoms[i] = atoms_return[i];
  }
#endif

  dcopAutoRegistration();
  dcopClientPostInit();

  smw = 0;

  // Initial KIPC event mask.
  kipcEventMask = (1 << KIPC::StyleChanged) | (1 << KIPC::PaletteChanged) |
                  (1 << KIPC::FontChanged) | (1 << KIPC::BackgroundChanged) |
                  (1 << KIPC::ToolbarStyleChanged) | (1 << KIPC::SettingsChanged);

  // Trigger creation of locale.
  (void) KGlobal::locale();

  KConfig* config = KGlobal::config();
  d->actionRestrictions = config->hasGroup("KDE Action Restrictions" );

  if (GUIenabled)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    XSetErrorHandler( kde_x_errhandler );
    XSetIOErrorHandler( kde_xio_errhandler );
#endif

    connect( this, SIGNAL( aboutToQuit() ), this, SIGNAL( shutDown() ) );

#ifdef Q_WS_X11 //FIXME(E)
    display = desktop()->x11Display();
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
//    kdisplaySetPalette(); done by kdisplaySetStyle
    propagateSettings(SETTINGS_QT);

    // Set default mime-source factory
    QMimeSourceFactory::setDefaultFactory (mimeSourceFactory());

    KConfigGroupSaver saver( config, "Development" );
    if( config->hasKey( "CheckAccelerators" ) || config->hasKey( "AutoCheckAccelerators" ))
        d->checkAccelerators = new KCheckAccelerators( this );
  }

  // save and restore the RTL setting, as installTranslator calls qt_detectRTLLanguage,
  // which makes it impossible to use the -reverse cmdline switch with KDE apps
  bool rtl = reverseLayout();
  installTranslator(new KDETranslator(this));
  setReverseLayout( rtl );
  if (i18n( "_: Dear Translator! Translate this string to the string 'LTR' in "
	 "left-to-right languages (as english) or to 'RTL' in right-to-left "
	 "languages (such as Hebrew and Arabic) to get proper widget layout." ) == "RTL")
  	setReverseLayout( true );

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
    XChangeProperty(qt_xdisplay(), smw->winId(),
		    atom_DesktopWindow, atom_DesktopWindow,
		    32, PropModeReplace, (unsigned char *)&data, 1);
  }
#else
  // FIXME(E): Implement for Qt Embedded
#endif

  d->oldIceIOErrorHandler = IceSetIOErrorHandler( kde_ice_ioerrorhandler );
}

static int my_system (const char *command) {
   int pid, status;

   QApplication::flushX();
   pid = fork();
   if (pid == -1)
      return -1;
   if (pid == 0) {
      const char* shell = "/bin/sh";
      execl(shell, shell, "-c", command, 0L);
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

  s_DCOPClient = new DCOPClient();
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
  if (args->isSet("dcopserver"))
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
    if (pSessionConfig)
        return pSessionConfig;

    // create an instance specific config object
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
    sessionClients()->remove( this );
    sessionClients()->append( this );
}

KSessionManaged::~KSessionManaged()
{
    sessionClients()->remove( this );
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
        QDataStream arg(data, IO_WriteOnly);
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
        return TRUE;
    }

    // open a temporary connection, if possible

    propagateSessionManager();
    QCString smEnv = ::getenv("SESSION_MANAGER");
    if (smEnv.isEmpty())
        return FALSE;

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
	    return FALSE;
    }

    SmcRequestSaveYourself( tmpSmcConnection, SmSaveBoth, True,
			    SmInteractStyleAny, False, True );

    // flush the request
    IceFlush(SmcGetIceConnection(tmpSmcConnection));
    return TRUE;
#else
    // FIXME(E): Implement for Qt Embedded
    return false;
#endif
}

void KApplication::propagateSessionManager()
{
    QCString fName = QFile::encodeName(locateLocal("socket", "KSMserver"));
    QCString display = ::getenv(DISPLAY);
    // strip the screen number from the display
    display.replace(QRegExp("\\.[0-9]+$"), "");
    int i;
    while( (i = display.find(':')) >= 0)
       display[i] = '_';

    fName += "_"+display;
    QCString smEnv = ::getenv("SESSION_MANAGER");
    bool check = smEnv.isEmpty();
    if ( !check && smModificationTime ) {
         QFileInfo info( fName );
         QTime current = info.lastModified().time();
         check = current > *smModificationTime;
    }
    if ( check ) {
        delete smModificationTime;
        QFile f( fName );
        if ( !f.open( IO_ReadOnly ) )
            return;
        QFileInfo info ( f );
        smModificationTime = new QTime( info.lastModified().time() );
        QTextStream t(&f);
        t.setEncoding( QTextStream::Latin1 );
        QString s = t.readLine();
        f.close();
        ::setenv( "SESSION_MANAGER", s.latin1(), TRUE  );
    }
}

void KApplication::commitData( QSessionManager& sm )
{
    bool cancelled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !cancelled;
         it = sessionClients()->next() ) {
        cancelled = !it->commitData( sm );
    }
    if ( cancelled )
        sm.cancel();

    if ( sm.allowsInteraction() ) {
        QWidgetList done;
        QWidgetList *list = QApplication::topLevelWidgets();
        bool cancelled = FALSE;
        QWidget* w = list->first();
        while ( !cancelled && w ) {
            if ( !w->testWState( WState_ForceHide ) && !w->inherits("KMainWindow") ) {
                QCloseEvent e;
                sendEvent( w, &e );
                cancelled = !e.isAccepted();
                if ( !cancelled )
                    done.append( w );
                delete list; // one never knows...
                list = QApplication::topLevelWidgets();
                w = list->first();
            } else {
                w = list->next();
            }
            while ( w && done.containsRef( w ) )
                w = list->next();
        }
        delete list;
    }


    if ( !bSessionManagement ) {
        sm.setRestartHint( QSessionManager::RestartNever );
        return;
    }
}

void KApplication::saveState( QSessionManager& sm )
{
#ifndef Q_WS_QWS
    static bool firstTime = true;
    mySmcConnection = (SmcConn) sm.handle();

    if ( !bSessionManagement ) {
        sm.setRestartHint( QSessionManager::RestartNever );
        return;
    }

#if QT_VERSION < 0x030100
    {
        // generate a new session key
        timeval tv;
        gettimeofday( &tv, 0 );
        d->sessionKey  = QString::number( tv.tv_sec ) + "_" + QString::number(tv.tv_usec);
    }
#endif

    if ( firstTime ) {
        firstTime = false;
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
#if QT_VERSION < 0x030100
    restartCommand.clear();
    restartCommand  << argv()[0] << "-session" << sm.sessionId() << "-smkey" << d->sessionKey;
    sm.setRestartCommand( restartCommand );
#endif


    QCString multiHead = getenv("KDE_MULTIHEAD");
    if (multiHead.lower() == "true") {
        // if multihead is enabled, we save our -display argument so that
        // we are restored onto the correct head... one problem with this
        // is that the display is hard coded, which means we cannot restore
        // to a different display (ie. if we are in a university lab and try,
        // try to restore a multihead session, our apps could be started on
        // someone else's display instead of our own)
        QCString displayname = getenv(DISPLAY);
        if (! displayname.isNull()) {
            // only store the command if we actually have a DISPLAY
            // environment variable
            restartCommand.append("-display");
            restartCommand.append(displayname);
        }
        sm.setRestartCommand( restartCommand );
    }


    // finally: do session management
    emit saveYourself(); // for compatiblity
    bool cancelled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !cancelled;
         it = sessionClients()->next() ) {
        cancelled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( pSessionConfig ) {
        pSessionConfig->sync();
        QStringList discard;
        discard  << "rm" << locateLocal("config", sessionConfigName());
        sm.setDiscardCommand( discard );
    }

    if ( cancelled )
        sm.cancel();
#else
    // FIXME(E): Implement for Qt Embedded
#endif
}

void KApplication::startKdeinit()
{
  // Try to launch kdeinit.
  QString srv = KStandardDirs::findExe(QString::fromLatin1("kdeinit"));
  if (srv.isEmpty())
     srv = KStandardDirs::findExe(QString::fromLatin1("kdeinit"), KDEDIR+QString::fromLatin1("/bin"));
  if (srv.isEmpty())
     return;
  if (kapp && (Tty != kapp->type()))
    setOverrideCursor( Qt::waitCursor );
  my_system(QFile::encodeName(srv)+" --suicide");
  if (kapp && (Tty != kapp->type()))
    restoreOverrideCursor();
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
     QString msgStr(i18n("There was an error setting up inter-process\n"
                      "communications for KDE. The message returned\n"
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
           i18n("OK")
         );
     }
     else
     {
       fprintf(stderr, "%s\n", msgStr.local8Bit().data());
     }

     return;
  }
}

static const KCmdLineOptions qt_options[] =
{
  //FIXME: Check if other options are specific to Qt/X11
#ifdef Q_WS_X11
   { "display <displayname>", I18N_NOOP("Use the X-server display 'displayname'."), 0},
#else
   { "display <displayname>", I18N_NOOP("Use the QWS display 'displayname'."), 0},
#endif
   { "session <sessionId>", I18N_NOOP("Restore the application for the given 'sessionId'."), 0},
   { "cmap", I18N_NOOP("Causes the application to install a private color\nmap on an 8-bit display."), 0},
   { "ncols <count>", I18N_NOOP("Limits the number of colors allocated in the color\ncube on an 8-bit display, if the application is\nusing the QApplication::ManyColor color\nspecification."), 0},
   { "nograb", I18N_NOOP("tells Qt to never grab the mouse or the keyboard."), 0},
   { "dograb", I18N_NOOP("running under a debugger can cause an implicit\n-nograb, use -dograb to override."), 0},
   { "sync", I18N_NOOP("switches to synchronous mode for debugging."), 0},
   { "fn", 0, 0},
   { "font <fontname>", I18N_NOOP("defines the application font."), 0},
   { "bg", 0, 0},
   { "background <color>", I18N_NOOP("sets the default background color and an\napplication palette (light and dark shades are\ncalculated)."), 0},
   { "fg", 0, 0},
   { "foreground <color>", I18N_NOOP("sets the default foreground color."), 0},
   { "btn", 0, 0},
   { "button <color>", I18N_NOOP("sets the default button color."), 0},
   { "name <name>", I18N_NOOP("sets the application name."), 0},
   { "title <title>", I18N_NOOP("sets the application title (caption)."), 0},
#ifdef Q_WS_X11
   { "visual TrueColor", I18N_NOOP("forces the application to use a TrueColor visual on\nan 8-bit display."), 0},
   { "inputstyle <inputstyle>", I18N_NOOP("sets XIM (X Input Method) input style. Possible\nvalues are onthespot, overthespot, offthespot and\nroot."), 0 },
   { "im <XIM server>", I18N_NOOP("set XIM server."),0},
   { "noxim", I18N_NOOP("disable XIM."), 0 },
#endif
#ifdef Q_WS_QWS
   { "qws", I18N_NOOP("forces the application to run as QWS Server."), 0},
#endif
   { "reverse", I18N_NOOP("mirrors the whole layout of widgets."), 0},
   { 0, 0, 0 }
};

static const KCmdLineOptions kde_options[] =
{
   { "caption <caption>",       I18N_NOOP("Use 'caption' as name in the titlebar."), 0},
   { "icon <icon>",             I18N_NOOP("Use 'icon' as the application icon."), 0},
   { "miniicon <icon>",         I18N_NOOP("Use 'icon' as the icon in the titlebar."), 0},
   { "config <filename>",       I18N_NOOP("Use alternative configuration file."), 0},
   { "dcopserver <server>",     I18N_NOOP("Use the DCOP Server specified by 'server'."), 0},
   { "nocrashhandler",          I18N_NOOP("Disable crash handler, to get core dumps."), 0},
   { "waitforwm",          I18N_NOOP("Waits for a WM_NET compatible windowmanager."), 0},
   { "style <style>", I18N_NOOP("sets the application GUI style."), 0},
   { "geometry <geometry>", I18N_NOOP("sets the client geometry of the main widget."), 0},
#if QT_VERSION < 0x030100
   { "smkey <sessionKey>", I18N_NOOP("Define a 'sessionKey' for the session id. Only valid with -session"), 0},
#endif
   { 0, 0, 0 }
};

void
KApplication::addCmdLineOptions()
{
   KCmdLineArgs::addCmdLineOptions(qt_options, "Qt", "qt");
   KCmdLineArgs::addCmdLineOptions(kde_options, "KDE", "kde");
}

void KApplication::parseCommandLine( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");

    if (args->isSet("config"))
    {
        QString config = QString::fromLocal8Bit(args->getOption("config"));
        setConfigName(config);
    }

    if (args->isSet("style"))
    {

       QStringList styles = QStyleFactory::keys();
       QString reqStyle(args->getOption("style").lower());

	   for (QStringList::ConstIterator it = styles.begin(); it != styles.end(); ++it)
		   if ((*it).lower() == reqStyle)
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
       aMiniIconPixmap = SmallIcon(tmp);
       aMiniIconName = tmp;
    }

    if (args->isSet("icon"))
    {
       const char *tmp = args->getOption("icon");
       aIconPixmap = DesktopIcon( tmp );
       aIconName = tmp;
       if (aMiniIconPixmap.isNull())
       {
          aMiniIconPixmap = SmallIcon( tmp );
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
        while ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), atom_NetSupported,
				    0, 1, FALSE, AnyPropertyType, &type, &format,
                                    &length, &after, &data ) != Success || !length ) {
            if ( data )
                XFree( data );
            XEvent event;
            XWindowEvent( qt_xdisplay(), qt_xrootwin(), PropertyChangeMask, &event );
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

#if QT_VERSION < 0x030100
    if (args->isSet("smkey"))
    {
        d->sessionKey = args->getOption("smkey");
    }
#endif

}

QString KApplication::geometryArgument() const
{
    return d->geometry_arg;
}

QPixmap KApplication::icon() const
{
  if( aIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication *>(this);
      that->aIconPixmap = DesktopIcon( instanceName() );
  }
  return aIconPixmap;
}

QString KApplication::iconName() const
{
  return aIconName.isNull() ? (QString)instanceName() : aIconName;
}

QPixmap KApplication::miniIcon() const
{
  if (aMiniIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication *>(this);
      that->aMiniIconPixmap = SmallIcon( instanceName() );
  }
  return aMiniIconPixmap;
}

QString KApplication::miniIconName() const
{
  return aMiniIconName.isNull() ? (QString)instanceName() : aMiniIconName;
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
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

  delete KProcessController::theKProcessController;

  if ( d->oldIceIOErrorHandler != NULL )
      IceSetIOErrorHandler( d->oldIceIOErrorHandler );

  delete d;
  KApp = 0;

#ifndef Q_WS_QWS
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
    if ( activeWindow() ) {
	switch ( _event->type ) {
	    case ButtonPress:
	    case ButtonRelease:
	    case XKeyPress:
	    {
		timeval tv;
		gettimeofday( &tv, NULL );
		unsigned long now = tv.tv_sec * 10 + tv.tv_usec / 100000;
		XChangeProperty(qt_xdisplay(), activeWindow()->winId(),
				atom_KdeNetUserTime, XA_CARDINAL,
				32, PropModeReplace, (unsigned char *)&now, 1);
	    }
	    break;
	    default: break;
	}
    }


    if ( kapp_block_user_input ) {
        switch ( _event->type  ) {
        case ButtonPress:
        case ButtonRelease:
        case XKeyPress:
        case XKeyRelease:
        case MotionNotify:
            return TRUE;
        default:
            break;
        }
    }

    if (x11Filter) {
        for (QWidget *w=x11Filter->first(); w; w=x11Filter->next()) {
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
#endif

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

    KConfig pConfig( QString::null, true );
    pConfig.setGroup("General");
    QString defaultStyle = (QPixmap::defaultDepth() > 8 ? "HighColor" : "Default");
    QString styleStr = pConfig.readEntry("widgetStyle", defaultStyle);

    if (d->overrideStyle.isEmpty()) {
      // ### add check wether we already use the correct style to return then
      // (workaround for Qt misbehaviour to avoid double style initialization)

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

void KApplication::kdisplaySetPalette()
{
    int contrast_ = KGlobalSettings::contrast();

    // the following is temporary and will soon dissappear (Matthias, 3.August 1999 )
    KConfigBase* config = KGlobal::config();
    KConfigGroupSaver saver( config, "General" );

    QColor kde2Gray(220, 220, 220);
    QColor kde2Blue;
    if (QPixmap::defaultDepth() > 8)
      kde2Blue.setRgb(84, 112, 152);
    else
      kde2Blue.setRgb(0, 0, 192);

    QColor background = config->readColorEntry( "background", &kde2Gray );
    QColor foreground = config->readColorEntry( "foreground", &black );
    QColor button = config->readColorEntry( "buttonBackground", &background );
    QColor buttonText = config->readColorEntry( "buttonForeground", &foreground );
    QColor highlight = config->readColorEntry( "selectBackground", &kde2Blue);
    QColor highlightedText = config->readColorEntry( "selectForeground", &white );
    QColor base = config->readColorEntry( "windowBackground", &white );
    QColor baseText = config->readColorEntry( "windowForeground", &black );
    QColor link = config->readColorEntry( "linkColor", &blue );
    QColor visitedLink = config->readColorEntry( "visitedLinkColor", &magenta );


    /*
     * WARNING WARNING WARNING
     *
     * For reasons I do not understand the code below is duplicated in
     * kdebase/kcontrol/krdb/krdb.cpp
     *
     * If you change it here, change it there as well
     */


    int highlightVal, lowlightVal;
    highlightVal = 100 + (2*contrast_+4)*16/10;
    lowlightVal = 100 + (2*contrast_+4)*10;

    QColor disfg = foreground;

    int h, s, v;
    disfg.hsv( &h, &s, &v );
    if (v > 128)
	// dark bg, light fg - need a darker disabled fg
	disfg = disfg.dark(lowlightVal);
    else if (disfg != black)
	// light bg, dark fg - need a lighter disabled fg - but only if !black
	disfg = disfg.light(highlightVal);
    else
	// black fg - use darkgrey disabled fg
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
    else if (disbtntext != black)
	// light buttonText, dark button - need a lighter disabled buttonText - but only if !black
	disbtntext = disbtntext.light(highlightVal);
    else
	// black button - use darkgrey disabled buttonText
	disbtntext = Qt::darkGray;

    disabledgrp.setColor(QColorGroup::ButtonText, disbtntext);
    disabledgrp.setColor(QColorGroup::Midlight, background.light(110));
    disabledgrp.setColor(QColorGroup::Link, link);
    disabledgrp.setColor(QColorGroup::LinkVisited, visitedLink);

    QPalette newPal(colgrp, disabledgrp, colgrp);
/*
    if(QPixmap::defaultDepth() > 8){
        QColorGroup iGrp(colgrp);
        iGrp.setColor(QColorGroup::Button, colgrp.button().light(115));
        iGrp.setColor(QColorGroup::ButtonText, colgrp.buttonText().light(115));
        iGrp.setColor(QColorGroup::Text, colgrp.text().light(115));
        iGrp.setColor(QColorGroup::Dark, colgrp.dark().light(115));
        iGrp.setColor(QColorGroup::Mid, colgrp.mid().light(115));
        iGrp.setColor(QColorGroup::Midlight, colgrp.midlight().light(115));
        iGrp.setColor(QColorGroup::Light, colgrp.light().light(115));
        newPal.setInactive(iGrp);
    }
*/
    QApplication::setPalette(newPal, true);

    //style().polish(newPal);
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
    QStyleSheet* sheet = QStyleSheet::defaultSheet();
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
    if (num < 200)
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
    b = !config->readBoolEntry("EffectNoTooltip", false);
    QToolTip::setGloballyEnabled( b );

    emit settingsChanged(arg);
}

void KApplication::installKDEPropertyMap()
{
#ifndef QT_NO_SQL
    // QSqlPropertyMap takes ownership of the new default map.
    QSqlPropertyMap *kdeMap = new QSqlPropertyMap;
    kdeMap->insert( "KComboBox", "currentItem" );
    kdeMap->insert( "KDatePicker", "getDate" );
    kdeMap->insert( "KEditListBox", "currentItem" );
    kdeMap->insert( "KFontCombo", "currentFont" );
    kdeMap->insert( "KHistoryCombo", "currentItem" );
    kdeMap->insert( "KListBox", "currentItem" );
    kdeMap->insert( "KLineEdit", "text" );
    kdeMap->insert( "KPasswordEdit", "text" );
    kdeMap->insert( "KRestrictedLine", "text" );
    kdeMap->insert( "KSqueezedTextLabel", "text" );
    kdeMap->insert( "KTextBrowser", "source" );
    kdeMap->insert( "KURLRequester", "url" );
    QSqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

void KApplication::invokeHelp( const QString& anchor,
                               const QString& _appname) const
{
   QString url;
   QString appname;
   if (_appname.isEmpty())
     appname = name();
   else
     appname = _appname;

   if (!anchor.isEmpty())
     url = QString("help:/%1?anchor=%2").arg(appname).arg(anchor);
   else
     url = QString("help:/%1/index.html").arg(appname);

   QString error;

   if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, "", true))
   {
      kdWarning() << "Could not launch help:\n" << error << endl;
      return;
   }
}

void KApplication::invokeHTMLHelp( const QString& _filename, const QString& topic ) const
{
   kdWarning() << "invoking HTML help is deprecated! use docbook and invokeHelp!\n";

   QString filename;

   if( _filename.isEmpty() )
     filename = QString(name()) + "/index.html";
   else
     filename = _filename;

   QString url;
   if (!topic.isEmpty())
     url = QString("help:/%1#%2").arg(filename).arg(topic);
   else
     url = QString("help:/%1").arg(filename);

   QString error;

   if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, "", true))
   {
      kdWarning() << "Could not launch help:\n" << error << endl;
      return;
   }
}


void KApplication::invokeMailer(const QString &address, const QString &subject)
{
   invokeMailer(address, QString::null, QString::null, subject, QString::null, QString::null, QStringList());
}

void KApplication::invokeMailer(const KURL &mailtoURL)
{
   QString address = KURL::decode_string(mailtoURL.path()), subject, cc, bcc, body, attach;
   QStringList queries = QStringList::split('&', mailtoURL.query().mid(1));
   for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
   {
     QString q = (*it).lower();
     if (q.startsWith("subject="))
       subject = KURL::decode_string((*it).mid(8));
     else
     if (q.startsWith("cc="))
       cc = KURL::decode_string((*it).mid(3));
     else
     if (q.startsWith("bcc="))
       bcc = KURL::decode_string((*it).mid(4));
     else
     if (q.startsWith("body="))
       body = KURL::decode_string((*it).mid(5));
     //else
     //  if (q.startsWith("attach="))
     //    attach = KURL::decode_string((*it).mid(7));
   }

   invokeMailer( address, cc, bcc, subject, body, QString::null, QStringList() );
}

void KApplication::invokeMailer(const QString &to, const QString &cc, const QString &bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs)
{
   KConfig config("emaildefaults");
   config.setGroup( QString::fromLatin1("PROFILE_Default") );
   QString command = config.readEntry("EmailClient");

   if (command.isEmpty() || command == QString::fromLatin1("kmail")
       || command.right(6) == "/kmail")
     command = QString::fromLatin1("kmail --composer -s %s -c %c -b %b --body %B --attach %A %t");

   // TODO: Take care of the preferred terminal app (instead of hardcoding
   // Konsole), this will probably require a rewrite of the configurable
   // terminal client option because the placeholder for the program which
   // has to be executed by the terminal has to be supplied (e.g. something
   // like '/opt/kde2/bin/konsole -e %p'). - Frerich
   if (config.readBoolEntry("TerminalClient", false))
      command = "konsole -e " + command;

   // WARNING: This will only work as long as the path of the
   // email client doesn't contain spaces (this is currently
   // impossible due to an evil hack in kcmemail but should
   // be changed after KDE 2.0!). - Frerich
   QStringList cmdTokens = QStringList::split(' ', command.simplifyWhiteSpace());
   QString cmd = cmdTokens[0];
   cmdTokens.remove(cmdTokens.begin());
   QString lastToken;
   QStringList newTokens;

   for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); ++it)
   {
     if ((*it).find("%t") >= 0)
       (*it).replace(QRegExp("%t"), to);
     else
     if ((*it).find("%s") >= 0)
       (*it).replace(QRegExp("%s"), subject);
     else
     if ((*it).find("%c") >= 0)
       (*it).replace(QRegExp("%c"), cc);
     else
     if ((*it).find("%b") >= 0)
       (*it).replace(QRegExp("%b"), bcc);
     else
     if ((*it).find("%B") >= 0)
       (*it).replace(QRegExp("%B"), body);
     else
     if ((*it).find("%A") >= 0)
     {
         QStringList::ConstIterator urlit = attachURLs.begin();
         QStringList::ConstIterator urlend = attachURLs.end();
         if ( urlit != urlend )
         {
             (*it).replace(QRegExp("%A"), (*urlit));
             ++urlit;
             QStringList::Iterator nextit = it; nextit++;
             for ( ; urlit != urlend ; ++urlit )
             {
                 it = cmdTokens.insert( nextit, lastToken );
                 it = cmdTokens.insert( nextit, (*urlit) );
             }
         } else
             (*it).replace(QRegExp("%A"), QString::null);
     }
     lastToken = (*it);
   }
   QString error;

   if (kdeinitExec(cmd, cmdTokens, &error))
   {
      kdWarning() << "Could not launch mail client:\n" << error << endl;
   }
}


void KApplication::invokeBrowser( const QString &url )
{
   QString error;

   if (startServiceByDesktopName("kfmclient", url, &error, 0, 0, "", true))
   {
      kdWarning() << "Could not launch browser:\n" << error << endl;
      return;
   }
}

QCString
KApplication::launcher()
{
   return "klauncher";
}

static int
startServiceInternal( const QCString &function,
              const QString& _name, const QStringList &URLs,
              QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   struct serviceResult
   {
      int result;
      QCString dcopName;
      QString error;
      pid_t pid;
   };

   // Register app as able to send DCOP messages
   DCOPClient *dcopClient;
   if (kapp)
      dcopClient = kapp->dcopClient();
   else
      dcopClient = new DCOPClient;

   if (!dcopClient->isAttached())
   {
      if (!dcopClient->attach())
      {
         if (error)
            *error = i18n("Could not register with DCOP.\n");
         return -1;
      }
   }
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << _name << URLs;
   QCString replyType;
   QByteArray replyData;
   QCString _launcher = KApplication::launcher();
   QValueList<QCString> envs;
#ifdef Q_WS_X11
   if (qt_xdisplay()) {
       QCString dpystring(XDisplayString(qt_xdisplay()));
       envs.append( QCString("DISPLAY=") + dpystring );
   }
#endif
   stream << envs;
   if( !startup_id.isNull()) // not kdeinit_exec
       stream << startup_id << noWait;

   if (!dcopClient->call(_launcher, _launcher,
        function, params, replyType, replyData))
   {
        if (error)
           *error = i18n("KLauncher could not be reached via DCOP.\n");
        if (!kapp)
           delete dcopClient;
        return -1;
   }
   if (!kapp)
      delete dcopClient;

   if (noWait)
      return 0;

   QDataStream stream2(replyData, IO_ReadOnly);
   serviceResult result;
   stream2 >> result.result >> result.dcopName >> result.error >> result.pid;
   if (dcopService)
      *dcopService = result.dcopName;
   if (error)
      *error = result.error;
   if (pid)
      *pid = result.pid;
   return result.result;
}

int
KApplication::startServiceByName( const QString& _name, const QString &URL,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByName( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopPath( const QString& _name, const QString &URL,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopPath( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopName( const QString& _name, const QString &URL,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::startServiceByDesktopName( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id, bool noWait )
{
   return startServiceInternal(
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)",
                      _name, URLs, error, dcopService, pid, startup_id, noWait);
}

int
KApplication::kdeinitExec( const QString& name, const QStringList &args,
                           QString *error, int *pid )
{
   return startServiceInternal("kdeinit_exec(QString,QStringList,QValueList<QCString>)",
        name, args, error, 0, pid, QCString(), false);
}

int
KApplication::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid )
{
   return startServiceInternal("kdeinit_exec_wait(QString,QStringList,QValueList<QCString>)",
        name, args, error, 0, pid, QCString(), false);
}

QString KApplication::tempSaveName( const QString& pFilename ) const
{
  QString aFilename;

  if( pFilename[0] != '/' )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = QFileInfo( QDir( "." ), pFilename ).absFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
        {
          // Last chance: use _PATH_TMP
          aAutosaveDir.setPath( _PATH_TMP );
        }
    }

  aFilename.replace( QRegExp( "/" ),"\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

  return aFilename;
}


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover ) const
{
  QString aFilename;

  if( pFilename[0] != '/' )
    {
      kdWarning(101) << "Relative filename passed to KApplication::tempSaveName" << endl;
      aFilename = QFileInfo( QDir( "." ), pFilename ).absFilePath();
    }
  else
    aFilename = pFilename;

  QDir aAutosaveDir( QDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
    {
      if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
        {
          // Last chance: use _PATH_TMP
          aAutosaveDir.setPath( _PATH_TMP );
        }
    }

  aFilename.replace( QRegExp( "/" ), "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

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
  int pos = dirName.findRev('/');
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
  if( topWidget != 0 )
  {
#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    Window leader = topWidget->winId();
#endif
    QCString string_buffer = instanceName();

    char * argv = string_buffer.data();
#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    XSetCommand(display, leader, &argv, 1);

    XClassHint hint;
    hint.res_name = string_buffer.data();
    hint.res_class = string_buffer.data();
    XSetClassHint(display, leader, &hint);

    XWMHints *hints = XAllocWMHints();
    hints->window_group = leader;
    hints->input = True;
    hints->flags = WindowGroupHint | InputHint;
    XSetWMHints(display, leader, hints);
    XFree(reinterpret_cast<char *>(hints));

    NETWinInfo info(qt_xdisplay(), topWidget->winId(), qt_xrootwin(),
        NET::WMName | NET::WMPid
        );

    // Set the _NET_WM_PID Atom to the pid of this process.
    info.setPid(getpid());
#endif

    // set the specified caption
    if ( !topWidget->inherits("KMainWindow") ) { // KMainWindow does this already for us
        topWidget->setCaption( caption() );
#ifndef Q_WS_QWS // FIXME(E): Implement for Qt/Embedded
        info.setName( caption().utf8().data() );
#endif
    }

    // set the specified icons
    topWidget->setIcon( icon() ); //standard X11
#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    KWin::setIcons(topWidget->winId(), icon(), miniIcon() ); // NET_WM hints for KWin

    // set a short icon text
    XSetIconName( qt_xdisplay(), topWidget->winId(), caption().utf8() );

    // set the app startup notification window property
    KStartupInfo::setWindowStartupId( topWidget->winId(), startupId());
#endif
  }
}

QCString KApplication::startupId() const
{
    return d->startup_id;
}

void KApplication::setStartupId( const QCString& startup_id )
{
    if( startup_id.isEmpty())
        d->startup_id = "0";
    else
        d->startup_id = startup_id;
}

// read the startup notification env variable, save it and unset it in order
// not to propagate it to processes started from this app
void KApplication::read_app_startup_id()
{
#ifdef Q_WS_X11
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();
    KStartupInfo::resetStartupEnv();
    d->startup_id = id.id();
#endif
}

int KApplication::random()
{
   static int init = false;
   if (!init)
   {
      unsigned int seed;
      init = true;
      int fd = open("/dev/urandom", O_RDONLY);
      if (fd <= 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
      {
            // No /dev/urandom... try something else.
            srand(getpid());
            seed = rand()+time(0);
      }
      if (fd >= 0) close(fd);
      srand(seed);
   }
   return rand();
}

QString KApplication::randomString(int length)
{
   if (length <=0 ) return QString::null;

   QString str;
   while (--length)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str += char(r);
      // so what if I work backwards?
   }
   return str;
}

bool KApplication::authorize(const QString &genericAction)
{
   if (!d->actionRestrictions)
      return true;

   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "KDE Action Restrictions" );
   return config->readBoolEntry(genericAction, true);
}

bool KApplication::authorizeKAction(const char *action)
{
   if (!d->actionRestrictions || !action)
      return true;

   static const QString &action_prefix = KGlobal::staticQString( "action/" );

   return authorize(action_prefix + action);
}

uint KApplication::keyboardModifiers()
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &keybstate );
    return keybstate & 0x00ff;
}

uint KApplication::mouseState()
{
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &keybstate );
    return keybstate & 0xff00;
}

void KApplication::virtual_hook( int id, void* data )
{ KInstance::virtual_hook( id, data ); }

void KSessionManaged::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kapplication.moc"
#define KeyPress XKeyPress
