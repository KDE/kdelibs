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
#include <qmetaobject.h>
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
#include <kstyle.h>
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
#include "kcheckaccelerators.h"
#include <qptrdict.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <kprotocolinfo.h>
#include <kkeynative.h>
#include <kmdcodec.h>
#include <kglobalaccel.h>

#if defined Q_WS_X11
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

// exported for kdm kfrontend
KDE_EXPORT bool kde_have_kipc = true; // magic hook to disable kipc in kdm
bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;
DCOPClient *KApplication::s_DCOPClient = 0L;
bool KApplication::s_dcopClientNeedsPostInit = false;

#ifdef Q_WS_X11
static Atom atom_DesktopWindow;
static Atom atom_NetSupported;
extern Time qt_x_time;
extern Time qt_x_user_time;
static Atom kde_xdnd_drop;
#endif

// duplicated from patched Qt, so that there won't be unresolved symbols if Qt gets
// replaced by unpatched one
KDECORE_EXPORT bool qt_qclipboard_bailout_hack = false;

template class QPtrList<KSessionManaged>;

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
class KApplicationPrivate
{
public:
  KApplicationPrivate()
    :   actionRestrictions( false ),
	refCount( 1 ),
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

  ~KApplicationPrivate()
  {
#ifdef Q_WS_WIN
     delete qassistantclient;
#endif
  }


  bool actionRestrictions : 1;
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
  QCString startup_id;
  QTimer* app_started_timer;
  KAppDCOPInterface *m_KAppDCOPInterface;
  bool session_save;
#ifdef Q_WS_X11
  int (*oldXErrorHandler)(Display*,XErrorEvent*);
  int (*oldXIOErrorHandler)(Display*);
#elif defined Q_WS_WIN
  QAssistantClient* qassistantclient;
#endif

  class URLActionRule
  {
  public:
#define checkExactMatch(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[s.length()-1] == '!') \
        { b = false; s.truncate(s.length()-1); } \
        else b = true;
#define checkStartWildCard(s, b) \
        if (s.isEmpty()) b = true; \
        else if (s[0] == '*') \
        { b = true; s = s.mid(1); } \
        else b = false;
#define checkEqual(s, b) \
        b = (s == "=");

     URLActionRule(const QString &act,
                   const QString &bProt, const QString &bHost, const QString &bPath,
                   const QString &dProt, const QString &dHost, const QString &dPath,
                   bool perm)
                   : action(act),
                     baseProt(bProt), baseHost(bHost), basePath(bPath),
                     destProt(dProt), destHost(dHost), destPath(dPath),
                     permission(perm)
                   {
                      checkExactMatch(baseProt, baseProtWildCard);
                      checkStartWildCard(baseHost, baseHostWildCard);
                      checkExactMatch(basePath, basePathWildCard);
                      checkExactMatch(destProt, destProtWildCard);
                      checkStartWildCard(destHost, destHostWildCard);
                      checkExactMatch(destPath, destPathWildCard);
                      checkEqual(destProt, destProtEqual);
                      checkEqual(destHost, destHostEqual);
                   }

     bool baseMatch(const KURL &url, const QString &protClass)
     {
        if (baseProtWildCard)
        {
           if ( !baseProt.isEmpty() && !url.protocol().startsWith(baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != baseProt) &&
                (protClass.isEmpty() || (protClass != baseProt)) )
              return false;
        }
        if (baseHostWildCard)
        {
           if (!baseHost.isEmpty() && !url.host().endsWith(baseHost))
              return false;
        }
        else
        {
           if (url.host() != baseHost)
              return false;
        }
        if (basePathWildCard)
        {
           if (!basePath.isEmpty() && !url.path().startsWith(basePath))
              return false;
        }
        else
        {
           if (url.path() != basePath)
              return false;
        }
        return true;
     }

     bool destMatch(const KURL &url, const QString &protClass, const KURL &base, const QString &baseClass)
     {
        if (destProtEqual)
        {
           if ( (url.protocol() != base.protocol()) &&
                (protClass.isEmpty() || baseClass.isEmpty() || protClass != baseClass) )
              return false;
        }
        else if (destProtWildCard)
        {
           if ( !destProt.isEmpty() && !url.protocol().startsWith(destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        else
        {
           if ( (url.protocol() != destProt) &&
                (protClass.isEmpty() || (protClass != destProt)) )
              return false;
        }
        if (destHostWildCard)
        {
           if (!destHost.isEmpty() && !url.host().endsWith(destHost))
              return false;
        }
        else if (destHostEqual)
        {
           if (url.host() != base.host())
              return false;
        }
        else
        {
           if (url.host() != destHost)
              return false;
        }
        if (destPathWildCard)
        {
           if (!destPath.isEmpty() && !url.path().startsWith(destPath))
              return false;
        }
        else
        {
           if (url.path() != destPath)
              return false;
        }
        return true;
     }

     QString action;
     QString baseProt;
     QString baseHost;
     QString basePath;
     QString destProt;
     QString destHost;
     QString destPath;
     bool baseProtWildCard : 1;
     bool baseHostWildCard : 1;
     bool basePathWildCard : 1;
     bool destProtWildCard : 1;
     bool destHostWildCard : 1;
     bool destPathWildCard : 1;
     bool destProtEqual    : 1;
     bool destHostEqual    : 1;
     bool permission;
  };
  QPtrList<URLActionRule> urlActionRestrictions;

    QString sessionKey;
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
    removeX11EventFilter( static_cast< const QWidget* >( sender()));
}

void KApplication::removeX11EventFilter( const QWidget* filter )
{
    if ( !x11Filter || !filter )
        return;
    x11Filter->removeRef( filter );
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
       if (t == QEvent::AccelOverride)
       {
          static_cast<QKeyEvent *>(event)->accept();
          return true;
       }
       else
          kdWarning(125) << "kde_g_bKillAccelOverride set, but received an event other than AccelOverride." << endl;
    }

    if ((t == QEvent::AccelOverride) || (t == QEvent::KeyPress))
    {
       static const KShortcut& _selectAll = KStdAccel::selectAll();
       QLineEdit *edit = ::qt_cast<QLineEdit *>(receiver);
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
       QTextEdit *medit = ::qt_cast<QTextEdit *>(receiver);
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
        if( w->isTopLevel() && !w->testWFlags( WX11BypassWM ) && !w->isPopup() && !event->spontaneous())
        {
            if( d->app_started_timer == NULL )
            {
                d->app_started_timer = new QTimer( this );
                connect( d->app_started_timer, SIGNAL( timeout()), SLOT( checkAppStartedSlot()));
            }
            if( !d->app_started_timer->isActive())
                d->app_started_timer->start( 0, true );
        }
        if( w->isTopLevel() && ( w->icon() == NULL || w->icon()->isNull()))
        {
            // icon() cannot be null pixmap, it'll be the "unknown" icon - so check if there is this application icon
            static QPixmap* ic = NULL;
            if( ic == NULL )
                ic = new QPixmap( KGlobal::iconLoader()->loadIcon( iconName(),
                    KIcon::NoGroup, 0, KIcon::DefaultState, NULL, true ));
            if( !ic->isNull())
            {
                w->setIcon( *ic );
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

KApplication::KApplication( int& argc, char** argv, const QCString& rAppName,
                            bool allowStyles, bool GUIenabled ) :
  QApplication( argc, argv, GUIenabled ), KInstance(rAppName),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate())
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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

KApplication::KApplication( bool allowStyles, bool GUIenabled ) :
  QApplication( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv(),
                GUIenabled ),
  KInstance( KCmdLineArgs::about),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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
  KInstance( KCmdLineArgs::about), display(0L), d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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
  KInstance( _instance ), display(0L), d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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
  KInstance( _instance ),
#ifdef Q_WS_X11
  display(0L),
#endif
  d (new KApplicationPrivate)
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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
KApplication::KApplication(Display *display, int& argc, char** argv, const QCString& rAppName,
                           bool allowStyles, bool GUIenabled ) :
  QApplication( display ), KInstance(rAppName),
  display(0L),
  d (new KApplicationPrivate())
{
    aIconPixmap.pm.icon = 0L;
    aIconPixmap.pm.miniIcon = 0L;
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

      XInternAtoms( qt_xdisplay(), names, n, false, atoms_return );

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
  d->actionRestrictions = config->hasGroup("KDE Action Restrictions" ) && !kde_kiosk_exception;
  // For brain-dead configurations where the user's local config file is not writable.
  // * We use kdialog to warn the user, so we better not generate warnings from
  //   kdialog itself.
  // * Don't warn if we run with a read-only $HOME
  QCString readOnly = getenv("KDE_HOME_READONLY");
  if (readOnly.isEmpty() && (qstrcmp(name(), "kdialog") != 0))
  {
    KConfigGroupSaver saver(config, "KDE Action Restrictions");
    if (config->readBoolEntry("warn_unwritable_config",true))
       config->checkConfigFilesWritable(true);
  }

  if (GUIenabled)
  {
#ifdef Q_WS_X11
    // this is important since we fork() to launch the help (Matthias)
    fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, FD_CLOEXEC);
    // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
    d->oldXErrorHandler = XSetErrorHandler( kde_x_errhandler );
    d->oldXIOErrorHandler = XSetIOErrorHandler( kde_xio_errhandler );
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
    // XXX: This is a hack. Make our factory the default factory, but add the
    // previous default factory to the list of factories. Why? When the default
    // factory can't resolve something, it iterates in the list of factories.
    // But it QWhatsThis only uses the default factory. So if there was already
    // a default factory (which happens when using an image library using uic),
    // we prefer KDE's factory and so we put that old default factory in the
    // list and use KDE as the default. This may speed up things as well.
    QMimeSourceFactory* oldDefaultFactory = QMimeSourceFactory::takeDefaultFactory();
    QMimeSourceFactory::setDefaultFactory( mimeSourceFactory() );
    if ( oldDefaultFactory ) {
        QMimeSourceFactory::addFactory( oldDefaultFactory );
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
    XChangeProperty(qt_xdisplay(), smw->winId(),
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

   QApplication::flushX();
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

  s_DCOPClient = new DCOPClient();
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
        return true;
    }

    // open a temporary connection, if possible

    propagateSessionManager();
    QCString smEnv = ::getenv("SESSION_MANAGER");
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
        ::setenv( "SESSION_MANAGER", s.latin1(), true  );
    }
#endif
}

void KApplication::commitData( QSessionManager& sm )
{
    d->session_save = true;
    bool canceled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !canceled;
         it = sessionClients()->next() ) {
        canceled = !it->commitData( sm );
    }
    if ( canceled )
        sm.cancel();

    if ( sm.allowsInteraction() ) {
        QWidgetList done;
        QWidgetList *list = QApplication::topLevelWidgets();
        bool canceled = false;
        QWidget* w = list->first();
        while ( !canceled && w ) {
            if ( !w->testWState( WState_ForceHide ) && !w->inherits("KMainWindow") ) {
                QCloseEvent e;
                sendEvent( w, &e );
                canceled = !e.isAccepted();
                if ( !canceled )
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
    emit saveYourself(); // for compatibility
    bool canceled = false;
    for (KSessionManaged* it = sessionClients()->first();
         it && !canceled;
         it = sessionClients()->next() ) {
        canceled = !it->saveState( sm );
    }

    // if we created a new session config object, register a proper discard command
    if ( pSessionConfig ) {
        pSessionConfig->sync();
        QStringList discard;
        discard  << "rm" << locateLocal("config", sessionConfigName());
        sm.setDiscardCommand( discard );
    } else {
	sm.setDiscardCommand( "" );
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
    setOverrideCursor( Qt::waitCursor );
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

static const KCmdLineOptions qt_options[] =
{
  //FIXME: Check if other options are specific to Qt/X11
#ifdef Q_WS_X11
   { "display <displayname>", I18N_NOOP("Use the X-server display 'displayname'"), 0},
#else
   { "display <displayname>", I18N_NOOP("Use the QWS display 'displayname'"), 0},
#endif
   { "session <sessionId>", I18N_NOOP("Restore the application for the given 'sessionId'"), 0},
   { "cmap", I18N_NOOP("Causes the application to install a private color\nmap on an 8-bit display"), 0},
   { "ncols <count>", I18N_NOOP("Limits the number of colors allocated in the color\ncube on an 8-bit display, if the application is\nusing the QApplication::ManyColor color\nspecification"), 0},
   { "nograb", I18N_NOOP("tells Qt to never grab the mouse or the keyboard"), 0},
   { "dograb", I18N_NOOP("running under a debugger can cause an implicit\n-nograb, use -dograb to override"), 0},
   { "sync", I18N_NOOP("switches to synchronous mode for debugging"), 0},
   { "fn", 0, 0},
   { "font <fontname>", I18N_NOOP("defines the application font"), 0},
   { "bg", 0, 0},
   { "background <color>", I18N_NOOP("sets the default background color and an\napplication palette (light and dark shades are\ncalculated)"), 0},
   { "fg", 0, 0},
   { "foreground <color>", I18N_NOOP("sets the default foreground color"), 0},
   { "btn", 0, 0},
   { "button <color>", I18N_NOOP("sets the default button color"), 0},
   { "name <name>", I18N_NOOP("sets the application name"), 0},
   { "title <title>", I18N_NOOP("sets the application title (caption)"), 0},
#ifdef Q_WS_X11
   { "visual TrueColor", I18N_NOOP("forces the application to use a TrueColor visual on\nan 8-bit display"), 0},
   { "inputstyle <inputstyle>", I18N_NOOP("sets XIM (X Input Method) input style. Possible\nvalues are onthespot, overthespot, offthespot and\nroot"), 0 },
   { "im <XIM server>", I18N_NOOP("set XIM server"),0},
   { "noxim", I18N_NOOP("disable XIM"), 0 },
#endif
#ifdef Q_WS_QWS
   { "qws", I18N_NOOP("forces the application to run as QWS Server"), 0},
#endif
   { "reverse", I18N_NOOP("mirrors the whole layout of widgets"), 0},
   KCmdLineLastOption
};

static const KCmdLineOptions kde_options[] =
{
   { "caption <caption>",       I18N_NOOP("Use 'caption' as name in the titlebar"), 0},
   { "icon <icon>",             I18N_NOOP("Use 'icon' as the application icon"), 0},
   { "miniicon <icon>",         I18N_NOOP("Use 'icon' as the icon in the titlebar"), 0},
   { "config <filename>",       I18N_NOOP("Use alternative configuration file"), 0},
   { "dcopserver <server>",     I18N_NOOP("Use the DCOP Server specified by 'server'"), 0},
   { "nocrashhandler",          I18N_NOOP("Disable crash handler, to get core dumps"), 0},
   { "waitforwm",          I18N_NOOP("Waits for a WM_NET compatible windowmanager"), 0},
   { "style <style>", I18N_NOOP("sets the application GUI style"), 0},
   { "geometry <geometry>", I18N_NOOP("sets the client geometry of the main widget - see man X for the argument format"), 0},
   { "smkey <sessionKey>", 0, 0}, // this option is obsolete and exists only to allow smooth upgrades from sessions
                                  // saved under Qt 3.0.x -- Qt 3.1.x includes the session key now automatically in
				  // the session id (Simon)
   KCmdLineLastOption
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

    if ( !args ) return;

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
       if (!aIconPixmap.pm.miniIcon) {
         aIconPixmap.pm.miniIcon = new QPixmap;
       }
       *aIconPixmap.pm.miniIcon = SmallIcon(tmp);
       aMiniIconName = tmp;
    }

    if (args->isSet("icon"))
    {
       const char *tmp = args->getOption("icon");
       if (!aIconPixmap.pm.icon) {
          aIconPixmap.pm.icon = new QPixmap;
       }
       *aIconPixmap.pm.icon = DesktopIcon( tmp );
       aIconName = tmp;
       if (!aIconPixmap.pm.miniIcon) {
         aIconPixmap.pm.miniIcon = new QPixmap;
       }
       if (aIconPixmap.pm.miniIcon->isNull())
       {
          *aIconPixmap.pm.miniIcon = SmallIcon( tmp );
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
				    0, 1, false, AnyPropertyType, &type, &format,
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
  if( !aIconPixmap.pm.icon) {
      aIconPixmap.pm.icon = new QPixmap;
  }
  if( aIconPixmap.pm.icon->isNull()) {
      *aIconPixmap.pm.icon = DesktopIcon( instanceName() );
  }
  return *aIconPixmap.pm.icon;
}

QString KApplication::iconName() const
{
  return aIconName.isNull() ? (QString)instanceName() : aIconName;
}

QPixmap KApplication::miniIcon() const
{
  if (!aIconPixmap.pm.miniIcon) {
      aIconPixmap.pm.miniIcon = new QPixmap;
  }
  if (aIconPixmap.pm.miniIcon->isNull()) {
      *aIconPixmap.pm.miniIcon = SmallIcon( instanceName() );
  }
  return *aIconPixmap.pm.miniIcon;
}

QString KApplication::miniIconName() const
{
  return aMiniIconName.isNull() ? (QString)instanceName() : aMiniIconName;
}

extern void kDebugCleanup();

KApplication::~KApplication()
{
  delete aIconPixmap.pm.miniIcon;
  aIconPixmap.pm.miniIcon = 0L;
  delete aIconPixmap.pm.icon;
  aIconPixmap.pm.icon = 0L;
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
                    if( qt_x_user_time == 0
                        || ( _event->xclient.data.l[ 3 ] - qt_x_user_time ) < 100000U )
                        { // and the timestamp looks reasonable
                        qt_x_user_time = _event->xclient.data.l[ 3 ]; // update our qt_x_user_time from it
                        }
                    }
                else // normal DND, only needed until Qt updates qt_x_user_time from XdndDrop
                    {
                    if( qt_x_user_time == 0
                        || ( _event->xclient.data.l[ 2 ] - qt_x_user_time ) < 100000U )
                        { // the timestamp looks reasonable
                        qt_x_user_time = _event->xclient.data.l[ 2 ]; // update our qt_x_user_time from it
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

void KApplication::updateUserTimestamp( unsigned long time )
{
#if defined Q_WS_X11
    if( time == 0 )
    { // get current X timestamp
        Window w = XCreateSimpleWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0, 0, 0 );
        XSelectInput( qt_xdisplay(), w, PropertyChangeMask );
        unsigned char data[ 1 ];
        XChangeProperty( qt_xdisplay(), w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
        XEvent ev;
        XWindowEvent( qt_xdisplay(), w, PropertyChangeMask, &ev );
        time = ev.xproperty.time;
        XDestroyWindow( qt_xdisplay(), w );
    }
    if( qt_x_user_time == 0
        || time - qt_x_user_time < 1000000000U ) // check time > qt_x_user_time, handle wrapping
        qt_x_user_time = time;
#endif
}

unsigned long KApplication::userTimestamp() const
{
#if defined Q_WS_X11
    return qt_x_user_time;
#else
    return 0;
#endif
}

void KApplication::updateRemoteUserTimestamp( const QCString& dcopId, unsigned long time )
{
#if defined Q_WS_X11
    if( time == 0 )
        time = qt_x_user_time;
    DCOPRef( dcopId, "MainApplication-Interface" ).call( "updateUserTimestamp", time );
#endif
}

void KApplication::invokeEditSlot( const char *slot )
{
  QObject *object = focusWidget();
  if( !object )
    return;

  QMetaObject *meta = object->metaObject();

  int idx = meta->findSlot( slot + 1, true );
  if( idx < 0 )
    return;

  object->qt_invoke( idx, 0 );
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
    QString defaultStyle = KStyle::defaultStyle();
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
    QColor foreground = config->readColorEntry( "foreground", &black );
    QColor button = config->readColorEntry( "buttonBackground", &kde34Button );
    QColor buttonText = config->readColorEntry( "buttonForeground", &black );
    QColor highlight = config->readColorEntry( "selectBackground", &kde34Blue );
    QColor highlightedText = config->readColorEntry( "selectForeground", &white );
    QColor base = config->readColorEntry( "windowBackground", &white );
    QColor baseText = config->readColorEntry( "windowForeground", &black );
    QColor link = config->readColorEntry( "linkColor", &kde34Link );
    QColor visitedLink = config->readColorEntry( "visitedLinkColor", &kde34VisitedLink );

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
    else if (disbtntext != black)
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
        bool do_not_set_palette = FALSE;
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
    b = !config->readBoolEntry("EffectNoTooltip", false);
    QToolTip::setGloballyEnabled( b );

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
    QSqlPropertyMap *kdeMap = new QSqlPropertyMap;
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
    QSqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

void KApplication::invokeHelp( const QString& anchor,
                               const QString& _appname) const
{
    return invokeHelp( anchor, _appname, "" );
}

#ifndef Q_WS_WIN
// for win32 we're using simple help tools like Qt Assistant,
// see kapplication_win.cpp
void KApplication::invokeHelp( const QString& anchor,
                               const QString& _appname,
                               const QCString& startup_id ) const
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
   if ( !dcopClient()->isApplicationRegistered("khelpcenter") )
   {
       if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false))
       {
           if (Tty != kapp->type())
               QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Help Center"),
               i18n("Could not launch the KDE Help Center:\n\n%1").arg(error), i18n("&OK"));
           else
               kdWarning() << "Could not launch help:\n" << error << endl;
	   return;
       }
   }
   else
       DCOPRef( "khelpcenter", "KHelpCenterIface" ).send( "openUrl", url, startup_id );
}
#endif

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
   if ( !dcopClient()->isApplicationRegistered("khelpcenter") )
   {
       if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, "", false))
       {
           if (Tty != kapp->type())
               QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Help Center"),
               i18n("Could not launch the KDE Help Center:\n\n%1").arg(error), i18n("&OK"));
           else
               kdWarning() << "Could not launch help:\n" << error << endl;
           return;
       }
   }
   else
       DCOPRef( "khelpcenter", "KHelpCenterIface" ).send( "openUrl", url );
}


void KApplication::invokeMailer(const QString &address, const QString &subject)
{
    return invokeMailer(address,subject,"");
}

void KApplication::invokeMailer(const QString &address, const QString &subject, const QCString& startup_id)
{
   invokeMailer(address, QString::null, QString::null, subject, QString::null, QString::null,
       QStringList(), startup_id );
}

void KApplication::invokeMailer(const KURL &mailtoURL)
{
    return invokeMailer( mailtoURL, "" );
}

void KApplication::invokeMailer(const KURL &mailtoURL, const QCString& startup_id )
{
    return invokeMailer( mailtoURL, startup_id, false);
}

void KApplication::invokeMailer(const KURL &mailtoURL, const QCString& startup_id, bool allowAttachments )
{
   QString address = KURL::decode_string(mailtoURL.path()), subject, cc, bcc, body;
   QStringList queries = QStringList::split('&', mailtoURL.query().mid(1));
   QStringList attachURLs;
   for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
   {
     QString q = (*it).lower();
     if (q.startsWith("subject="))
       subject = KURL::decode_string((*it).mid(8));
     else
     if (q.startsWith("cc="))
       cc = cc.isEmpty()? KURL::decode_string((*it).mid(3)): cc + ',' + KURL::decode_string((*it).mid(3));
     else
     if (q.startsWith("bcc="))
       bcc = bcc.isEmpty()? KURL::decode_string((*it).mid(4)): bcc + ',' + KURL::decode_string((*it).mid(4));
     else
     if (q.startsWith("body="))
       body = KURL::decode_string((*it).mid(5));
     else
     if (allowAttachments && q.startsWith("attach="))
       attachURLs.push_back(KURL::decode_string((*it).mid(7)));
     else
     if (allowAttachments && q.startsWith("attachment="))
       attachURLs.push_back(KURL::decode_string((*it).mid(11)));
     else
     if (q.startsWith("to="))
       address = address.isEmpty()? KURL::decode_string((*it).mid(3)): address + ',' + KURL::decode_string((*it).mid(3));
   }

   invokeMailer( address, cc, bcc, subject, body, QString::null, attachURLs, startup_id );
}

void KApplication::invokeMailer(const QString &to, const QString &cc, const QString &bcc,
                                const QString &subject, const QString &body,
                                const QString & messageFile, const QStringList &attachURLs)
{
    return invokeMailer(to,cc,bcc,subject,body,messageFile,attachURLs,"");
}

#ifndef Q_WS_WIN
// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp

static QStringList splitEmailAddressList( const QString & aStr )
{
  // This is a copy of KPIM::splitEmailAddrList().
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  QStringList list;

  if (aStr.isEmpty())
    return list;

  QString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for (uint index=0; index<aStr.length(); index++) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch (aStr[index].latin1()) {
    case '"' : // start or end of quoted string
      if (commentlevel == 0)
        insidequote = !insidequote;
      break;
    case '(' : // start of comment
      if (!insidequote)
        commentlevel++;
      break;
    case ')' : // end of comment
      if (!insidequote) {
        if (commentlevel > 0)
          commentlevel--;
        else {
          //kdDebug() << "Error in address splitting: Unmatched ')'"
          //          << endl;
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
      if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, index-addrstart);
        if (!addr.isEmpty())
          list += addr.simplifyWhiteSpace();
        addrstart = index+1;
      }
      break;
    }
  }
  // append the last address to the list
  if (!insidequote && (commentlevel == 0)) {
    addr = aStr.mid(addrstart, aStr.length()-addrstart);
    if (!addr.isEmpty())
      list += addr.simplifyWhiteSpace();
  }
  //else
  //  kdDebug() << "Error in address splitting: "
  //            << "Unexpected end of address list"
  //            << endl;

  return list;
}

void KApplication::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                const QString &subject, const QString &body,
                                const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                const QCString& startup_id )
{
   KConfig config("emaildefaults");

   config.setGroup("Defaults");
   QString group = config.readEntry("Profile","Default");

   config.setGroup( QString("PROFILE_%1").arg(group) );
   QString command = config.readPathEntry("EmailClient");

   QString to, cc, bcc;
   if (command.isEmpty() || command == QString::fromLatin1("kmail")
       || command.endsWith("/kmail"))
   {
     command = QString::fromLatin1("kmail --composer -s %s -c %c -b %b --body %B --attach %A -- %t");
     if ( !_to.isEmpty() )
     {
       // put the whole address lists into RFC2047 encoded blobs; technically
       // this isn't correct, but KMail understands it nonetheless
       to = QString( "=?utf8?b?%1?=" )
            .arg( KCodecs::base64Encode( _to.utf8(), false ) );
     }
     if ( !_cc.isEmpty() )
       cc = QString( "=?utf8?b?%1?=" )
            .arg( KCodecs::base64Encode( _cc.utf8(), false ) );
     if ( !_bcc.isEmpty() )
       bcc = QString( "=?utf8?b?%1?=" )
             .arg( KCodecs::base64Encode( _bcc.utf8(), false ) );
   } else {
     to = _to;
     cc = _cc;
     bcc = _bcc;
   }

   if (config.readBoolEntry("TerminalClient", false))
   {
     KConfigGroup confGroup( KGlobal::config(), "General" );
     QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", "konsole");
     command = preferredTerminal + " -e " + command;
   }

   QStringList cmdTokens = KShell::splitArgs(command);
   QString cmd = cmdTokens[0];
   cmdTokens.remove(cmdTokens.begin());

   KURL url;
   QStringList qry;
   if (!to.isEmpty())
   {
     QStringList tos = splitEmailAddressList( to );
     url.setPath( tos.first() );
     tos.remove( tos.begin() );
     for (QStringList::ConstIterator it = tos.begin(); it != tos.end(); ++it)
       qry.append( "to=" + KURL::encode_string( *it ) );
   }
   const QStringList ccs = splitEmailAddressList( cc );
   for (QStringList::ConstIterator it = ccs.begin(); it != ccs.end(); ++it)
      qry.append( "cc=" + KURL::encode_string( *it ) );
   const QStringList bccs = splitEmailAddressList( bcc );
   for (QStringList::ConstIterator it = bccs.begin(); it != bccs.end(); ++it)
      qry.append( "bcc=" + KURL::encode_string( *it ) );
   for (QStringList::ConstIterator it = attachURLs.begin(); it != attachURLs.end(); ++it)
      qry.append( "attach=" + KURL::encode_string( *it ) );
   if (!subject.isEmpty())
      qry.append( "subject=" + KURL::encode_string( subject ) );
   if (!body.isEmpty())
      qry.append( "body=" + KURL::encode_string( body ) );
   url.setQuery( qry.join( "&" ) );
   if ( ! (to.isEmpty() && qry.isEmpty()) )
      url.setProtocol("mailto");

   QMap<QChar, QString> keyMap;
   keyMap.insert('t', to);
   keyMap.insert('s', subject);
   keyMap.insert('c', cc);
   keyMap.insert('b', bcc);
   keyMap.insert('B', body);
   keyMap.insert('u', url.url());

   for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
   {
     if (*it == "%A")
     {
         if (it == cmdTokens.begin()) // better safe than sorry ...
             continue;
         QStringList::ConstIterator urlit = attachURLs.begin();
         QStringList::ConstIterator urlend = attachURLs.end();
         if ( urlit != urlend )
         {
             QStringList::Iterator previt = it;
             --previt;
             *it = *urlit;
             ++it;
             while ( ++urlit != urlend )
             {
                 cmdTokens.insert( it, *previt );
                 cmdTokens.insert( it, *urlit );
             }
         } else {
             --it;
             it = cmdTokens.remove( cmdTokens.remove( it ) );
         }
     } else {
         *it = KMacroExpander::expandMacros(*it, keyMap);
         ++it;
     }
   }

   QString error;
   // TODO this should check if cmd has a .desktop file, and use data from it, together
   // with sending more ASN data
   if (kdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
     if (Tty != kapp->type())
       QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Mail Client"),
             i18n("Could not launch the mail client:\n\n%1").arg(error), i18n("&OK"));
     else
       kdWarning() << "Could not launch mail client:\n" << error << endl;
}
#endif

void KApplication::invokeBrowser( const QString &url )
{
    return invokeBrowser( url, "" );
}

#ifndef Q_WS_WIN
// on win32, for invoking browser we're using win32 API
// see kapplication_win.cpp
void KApplication::invokeBrowser( const QString &url, const QCString& startup_id )
{
   QString error;

   if (startServiceByDesktopName("kfmclient", url, &error, 0, 0, startup_id, false))
   {
      if (Tty != kapp->type())
          QMessageBox::critical(kapp->mainWidget(), i18n("Could not Launch Browser"),
               i18n("Could not launch the browser:\n\n%1").arg(error), i18n("&OK"));
      else
          kdWarning() << "Could not launch browser:\n" << error << endl;
      return;
   }
}
#endif

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
   } else if( getenv( "DISPLAY" )) {
       QCString dpystring( getenv( "DISPLAY" ));
       envs.append( QCString("DISPLAY=") + dpystring );
   }
#endif
   stream << envs;
#if defined Q_WS_X11
   // make sure there is id, so that user timestamp exists
   stream << ( startup_id.isEmpty() ? KStartupInfo::createNewStartupId() : startup_id );
#endif
   if( function.left( 12 ) != "kdeinit_exec" )
       stream << noWait;

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
    return kdeinitExec( name, args, error, pid, "" );
}

int
KApplication::kdeinitExec( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QCString& startup_id )
{
   return startServiceInternal("kdeinit_exec(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}

int
KApplication::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid )
{
    return kdeinitExecWait( name, args, error, pid, "" );
}

int
KApplication::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid, const QCString& startup_id )
{
   return startServiceInternal("kdeinit_exec_wait(QString,QStringList,QValueList<QCString>,QCString)",
        name, args, error, 0, pid, startup_id, false);
}

QString KApplication::tempSaveName( const QString& pFilename ) const
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
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
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

  return aFilename;
}


QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover ) const
{
  QString aFilename;

  if( QDir::isRelativePath(pFilename) )
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
          // Last chance: use temp dir
          aAutosaveDir.setPath( KGlobal::dirs()->saveLocation("tmp") );
        }
    }

  aFilename.replace( "/", "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

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

QCString KApplication::startupId() const
{
    return d->startup_id;
}

void KApplication::setStartupId( const QCString& startup_id )
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
   static bool init = false;
   if (!init)
   {
      unsigned int seed;
      init = true;
      int fd = open("/dev/urandom", O_RDONLY);
      if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
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

   QString str; str.setLength( length );
   int i = 0;
   while (length--)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
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

bool KApplication::authorizeControlModule(const QString &menuId)
{
   if (menuId.isEmpty() || kde_kiosk_exception)
      return true;
   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "KDE Control Module Restrictions" );
   return config->readBoolEntry(menuId, true);
}

QStringList KApplication::authorizeControlModules(const QStringList &menuIds)
{
   KConfig *config = KGlobal::config();
   KConfigGroupSaver saver( config, "KDE Control Module Restrictions" );
   QStringList result;
   for(QStringList::ConstIterator it = menuIds.begin();
       it != menuIds.end(); ++it)
   {
      if (config->readBoolEntry(*it, true))
         result.append(*it);
   }
   return result;
}

void KApplication::initUrlActionRestrictions()
{
  d->urlActionRestrictions.setAutoDelete(true);
  d->urlActionRestrictions.clear();
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("open", QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("list", QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, true));
// TEST:
//  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
//  ("list", QString::null, QString::null, QString::null, QString::null, QString::null, QString::null, false));
//  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
//  ("list", QString::null, QString::null, QString::null, "file", QString::null, QDir::homeDirPath(), true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("link", QString::null, QString::null, QString::null, ":internet", QString::null, QString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", QString::null, QString::null, QString::null, ":internet", QString::null, QString::null, true));

  // We allow redirections to file: but not from internet protocols, redirecting to file:
  // is very popular among io-slaves and we don't want to break them
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", QString::null, QString::null, QString::null, "file", QString::null, QString::null, true));
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", ":internet", QString::null, QString::null, "file", QString::null, QString::null, false));

  // local protocols may redirect everywhere
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", ":local", QString::null, QString::null, QString::null, QString::null, QString::null, true));

  // Anyone may redirect to about:
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", QString::null, QString::null, QString::null, "about", QString::null, QString::null, true));

  // Anyone may redirect to itself, cq. within it's own group
  d->urlActionRestrictions.append( new KApplicationPrivate::URLActionRule
  ("redirect", QString::null, QString::null, QString::null, "=", QString::null, QString::null, true));

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver( config, "KDE URL Restrictions" );
  int count = config->readNumEntry("rule_count");
  QString keyFormat = QString("rule_%1");
  for(int i = 1; i <= count; i++)
  {
    QString key = keyFormat.arg(i);
    QStringList rule = config->readListEntry(key);
    if (rule.count() != 8)
      continue;
    QString action = rule[0];
    QString refProt = rule[1];
    QString refHost = rule[2];
    QString refPath = rule[3];
    QString urlProt = rule[4];
    QString urlHost = rule[5];
    QString urlPath = rule[6];
    QString strEnabled = rule[7].lower();

    bool bEnabled = (strEnabled == "true");

    if (refPath.startsWith("$HOME"))
       refPath.replace(0, 5, QDir::homeDirPath());
    else if (refPath.startsWith("~"))
       refPath.replace(0, 1, QDir::homeDirPath());
    if (urlPath.startsWith("$HOME"))
       urlPath.replace(0, 5, QDir::homeDirPath());
    else if (urlPath.startsWith("~"))
       urlPath.replace(0, 1, QDir::homeDirPath());

    if (refPath.startsWith("$TMP"))
       refPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));
    if (urlPath.startsWith("$TMP"))
       urlPath.replace(0, 4, KGlobal::dirs()->saveLocation("tmp"));

    d->urlActionRestrictions.append(new KApplicationPrivate::URLActionRule
    	( action, refProt, refHost, refPath, urlProt, urlHost, urlPath, bEnabled));
  }
}

void KApplication::allowURLAction(const QString &action, const KURL &_baseURL, const KURL &_destURL)
{
  if (authorizeURLAction(action, _baseURL, _destURL))
     return;

  d->urlActionRestrictions.append(new KApplicationPrivate::URLActionRule
        ( action, _baseURL.protocol(), _baseURL.host(), _baseURL.path(-1),
                  _destURL.protocol(), _destURL.host(), _destURL.path(-1), true));
}

bool KApplication::authorizeURLAction(const QString &action, const KURL &_baseURL, const KURL &_destURL)
{
  if (_destURL.isEmpty())
     return true;

  bool result = false;
  if (d->urlActionRestrictions.isEmpty())
     initUrlActionRestrictions();

  KURL baseURL(_baseURL);
  baseURL.setPath(QDir::cleanDirPath(baseURL.path()));
  QString baseClass = KProtocolInfo::protocolClass(baseURL.protocol());
  KURL destURL(_destURL);
  destURL.setPath(QDir::cleanDirPath(destURL.path()));
  QString destClass = KProtocolInfo::protocolClass(destURL.protocol());

  for(KApplicationPrivate::URLActionRule *rule = d->urlActionRestrictions.first();
      rule; rule = d->urlActionRestrictions.next())
  {
     if ((result != rule->permission) && // No need to check if it doesn't make a difference
         (action == rule->action) &&
         rule->baseMatch(baseURL, baseClass) &&
         rule->destMatch(destURL, destClass, baseURL, baseClass))
     {
        result = rule->permission;
     }
  }
  return result;
}


uint KApplication::keyboardModifiers()
{
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &keybstate );
    return keybstate & 0x00ff;
#elif defined W_WS_MACX
    return GetCurrentEventKeyModifiers() & 0x00ff;
#else
    //TODO for win32
    return 0;
#endif
}

uint KApplication::mouseState()
{
    uint mousestate;
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &mousestate );
#elif defined(Q_WS_WIN)
    const bool mousebtn_swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(VK_LBUTTON))
        mousestate |= (mousebtn_swapped ? Button3Mask : Button1Mask);
    if (GetAsyncKeyState(VK_MBUTTON))
        mousestate |= Button2Mask;
    if (GetAsyncKeyState(VK_RBUTTON))
        mousestate |= (mousebtn_swapped ? Button1Mask : Button3Mask);
#elif defined(Q_WS_MACX)
    mousestate = GetCurrentEventButtonState();
#else
    //TODO: other platforms
#endif
    return mousestate & 0xff00;
}

Qt::ButtonState KApplication::keyboardMouseState()
{
    int ret = 0;
#ifdef Q_WS_X11
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint state;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
                   &root_x, &root_y, &win_x, &win_y, &state );
    // transform the same way like Qt's qt_x11_translateButtonState()
    if( state & Button1Mask )
        ret |= LeftButton;
    if( state & Button2Mask )
        ret |= MidButton;
    if( state & Button3Mask )
        ret |= RightButton;
    if( state & ShiftMask )
        ret |= ShiftButton;
    if( state & ControlMask )
        ret |= ControlButton;
    if( state & KKeyNative::modX( KKey::ALT ))
        ret |= AltButton;
    if( state & KKeyNative::modX( KKey::WIN ))
        ret |= MetaButton;
#elif defined(Q_WS_WIN)
    const bool mousebtn_swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(VK_LBUTTON))
        ret |= (mousebtn_swapped ? RightButton : LeftButton);
    if (GetAsyncKeyState(VK_MBUTTON))
        ret |= MidButton;
    if (GetAsyncKeyState(VK_RBUTTON))
        ret |= (mousebtn_swapped ? LeftButton : RightButton);
    if (GetAsyncKeyState(VK_SHIFT))
        ret |= ShiftButton;
    if (GetAsyncKeyState(VK_CONTROL))
        ret |= ControlButton;
    if (GetAsyncKeyState(VK_MENU))
        ret |= AltButton;
    if (GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN))
        ret |= MetaButton;
#else
    //TODO: other platforms
#endif
    return static_cast< ButtonState >( ret );
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

void KApplication::virtual_hook( int id, void* data )
{ KInstance::virtual_hook( id, data ); }

void KSessionManaged::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kapplication.moc"
