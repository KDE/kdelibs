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

#define INCLUDE_MENUITEM_DEF

#include "config.h"

#undef QT_NO_TRANSLATION
#include <qtranslator.h>
#define QT_NO_TRANSLATION
#undef Unsorted
#include <qdir.h>
#include <qcollection.h>
#include <qwidgetlist.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qsessionmanager.h>
#include <qlist.h>
#include <qstylesheet.h>
#include <qpixmapcache.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qstylefactory.h>

#undef QT_NO_TRANSLATION
#include <kapp.h>
#define QT_NO_TRANSLATION
#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
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
#include <qobjectlist.h>
#include <qmetaobject.h>
#include <qptrdict.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qtabbar.h>

#include <kstyle.h>
#include <kdestyle.h>
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

#ifdef Q_WS_X11
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

bool kde_have_kipc = true; // magic hook to disable kipc in kdm

KApplication* KApplication::KApp = 0L;
bool KApplication::loadedByKdeinit = false;

template class QList<KSessionManaged>;

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

class KCheckAccelerators : public QObject
{
public:
    KCheckAccelerators( QObject* parent, int k )
	: QObject( parent, "kapp_accel_filter" ), key( k ), block( false ) {
	 strictMenuCheck = TRUE;
	parent->installEventFilter( this );
    }
    bool eventFilter( QObject * , QEvent * e) {
	if ( block )
	    return false;
	if ( e->type() == QEvent::Accel ) {
	    if ( ( static_cast<QKeyEvent *>(e) )->key() == key ) {
		block = true;
		checkAccelerators();
		block = false;
		( static_cast<QKeyEvent *>(e) )->accept();
		return true;
	    }
	}
	return false;
    }

    struct AccelInfo {
	QString item;
	QString string;
    };

    typedef QValueList<AccelInfo> AccelInfoList;

    void findAccel( const QString& item, const QString &txt, QMap<QChar, AccelInfoList > &accels ) {
	int i = txt.find( "&" );
	if ( i == -1 )
	    return;
	QChar c = txt[ i + 1 ];
	if ( c.isNull() || c == '&' )
	    return;
	c = c.lower();
	QMap<QChar, AccelInfoList>::Iterator it = accels.find( c );
	AccelInfo info;
	info.item  = item;
	info.string = txt;
	if ( it == accels.end() ) {
	    AccelInfoList list;
	    list.append( info );
	    accels.insert( c, list );
	} else {
	    AccelInfoList &list = it.data();
	    list.append( info );
	}
    }


    void checkMenuData( const QString& prefix, QMenuData* m, QMap<QChar, AccelInfoList > accels  ) {
	QMenuItem* mi;
	int i;
	QString s;
	for ( i = 0; i < (int) m->count(); i++ ) {
	    mi = m->findItem( m->idAt( i ) );
	    s = mi->text();
	    if ( s.contains( '\t' ) )
		s = s.left( s.find( '\t' ) );
	    findAccel( prefix + s, s, accels );
	}

	for ( QMap<QChar,AccelInfoList>::Iterator it = accels.begin(); it != accels.end(); ++it  ) {
	    QChar c = it.key();
	    AccelInfoList list = it.data();
  	    if ( list.count() <= 1 )
  		continue;
	    QMap<QChar, AccelInfoList>::Iterator menuIt = menuAccels.find( c );
	    if ( menuIt == menuAccels.end() ) {
		menuAccels.insert( c, list );
	    } else {
		AccelInfoList &olist = menuIt.data();
		for ( AccelInfoList::Iterator ait = list.begin(); ait != list.end(); ++ait )
		    olist.append( *ait );
	    }
	}

	for ( i = 0; i < (int) m->count(); i++ ) {
	    mi = m->findItem( m->idAt( i ) );
	    if ( mi->popup() ) {
		s = mi->text();
		if ( s.contains( '\t' ) )
		    s = s.left( s.find( '\t' ) );
		if ( !strictMenuCheck )
		    accels.clear();
		checkMenuData( prefix + s + "/", mi->popup(), accels );
	    }
	}
    }

    void checkMenuData( QMenuData* m ) {
	QMap<QChar, AccelInfoList > accels;
	checkMenuData( "", m, accels );
    }

    void checkAccelerators() {
	QWidget* actWin = qApp->activeWindow();
	if ( !actWin )
	    return;
	QMap<QChar, AccelInfoList > accels;
	QObjectList *l = actWin->queryList( "QWidget" );
	if ( !l )
	    return;
	QMenuBar* mbar = 0;
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    if ( ( static_cast<QWidget *>(o) )->isVisibleTo( actWin ) ) {
		QWidget *w = static_cast<QWidget *>(o);
		if ( w->inherits( "QMenuBar" ) )
		    mbar = static_cast<QMenuBar *>(w);
#if QT_VERSION < 300
		const QMetaProperty* text = w->metaObject()->property( "text", TRUE );
		const QMetaProperty* title = w->metaObject()->property( "title", TRUE );
#else
		QMetaObject *mo = w->metaObject();
		const QMetaProperty* text = mo->property( mo->findProperty( "text", TRUE ), TRUE );
		const QMetaProperty* title = mo->property( mo->findProperty( "title", TRUE ), TRUE );
#endif
		if ( text )
		    findAccel( w->className(), w->property( "text" ).toString(), accels );
		if ( title )
		    findAccel( w->className(), w->property( "title" ).toString(), accels );

		if ( w->inherits( "QTabBar" ) ) {
		    QTabBar *tbar = static_cast<QTabBar *>(w);
		    for ( int i = 0; i < tbar->count(); i++ )
			findAccel( tbar->className(), tbar->tab( i )->text(), accels );
		}
	    }
	}
	delete l;

	QString s;

	int num_clashes = 0;
	for ( QMap<QChar,AccelInfoList>::Iterator it = accels.begin(); it != accels.end(); ++it  ) {
	    AccelInfoList list = it.data();
  	    if ( list.count() <= 1 )
  		continue;

	    if ( ++num_clashes == 1 ) {
		s += "<table border>";
		s += "<tr><th>Accel</th><th>String</th><th>Widget</th></tr>";
	    }
	    AccelInfoList::Iterator ait = list.begin();
	    s += "<tr><td rowspan=" + QString::number( list.count() ) + "><large><b>" + it.key() + "</b></large></td>";
	    s += "<td>";
	    s += (*ait).string;
	    s += "</td><td>";
	    s += (*ait).item;
	    s += "</td></tr>";

	    for ( ait++; ait != list.end(); ++ait ) {
		s += "<tr><td>";
		s += (*ait).string;
		s += "</td><td>";
		s += (*ait).item;
		s += "</td></tr>";
	    }
	}
	if ( num_clashes  ) {
	    s += "</table>";
	    if ( num_clashes == 1 )
		s.prepend( "<h3>One clash detected</h3>" );
	    else
		s.prepend(  QString("<h3>" ) + QString::number( num_clashes ) + " clashes detected</h3>" );
	} else {
	    s += "<h3>No clashes detected</h3>";
	}

	if ( mbar ) {
	    QString m;
	    num_clashes = 0;
	    checkMenuData( mbar );
	    for ( QMap<QChar,AccelInfoList>::Iterator it = menuAccels.begin(); it != menuAccels.end(); ++it  ) {
		AccelInfoList list = it.data();
		AccelInfoList::Iterator ait;
		QStringList unique;
		for ( ait = list.begin(); ait != list.end(); ++ait ) {
		    if ( !unique.contains( (*ait).item ) )
			unique += (*ait).item;
		}
		if ( unique.count() <= 1 )
		    continue;

		if ( ++num_clashes == 1 ) {
		    m += "<table border>";
		    m += "<tr><th>Accel</th><th>Menu Item</th></tr>";
		}
		ait = list.begin();
		m += "<tr><td rowspan=" + QString::number( unique.count() ) + "><large><b>" + it.key() + "</b></large></td>";
		unique.clear();
		m += "<td>";
		m += (*ait).item;
		unique += (*ait).item;
		m += "</td></tr>";

		for ( ait++; ait != list.end(); ++ait ) {
		    if ( unique.contains( (*ait).item ) )
			continue;
		    m += "<tr><td>";
		    m += (*ait).item;
		    unique += (*ait).item;
		    m += "</td></tr>";
		}
	    }
	    if ( num_clashes  ) {
		m += "</table>";
		if ( num_clashes == 1 )
		    m.prepend( "<h3>One clash detected</h3>" );
		else
		    m.prepend(  QString("<h3>" ) + QString::number( num_clashes ) + " clashes detected</h3>" );
	    } else {
		m += "<h3>No clashes detected</h3>";
	    }

	    m.prepend( "<h2>Menubar</h2>" );
	    m += "<h2>Other control elements</h2>";
	    s.prepend( m );
	}

	s.prepend( QString("<h2><em>") + actWin->caption() + "<em></h2>" );

	QDialog* dlg = new QDialog( actWin, "kapp_accel_check_dlg",  true, WDestructiveClose );
	dlg->setCaption( "Dr. Klash' Accelerator Diagnosis" );
	dlg->resize( 500, 460 );
	QVBoxLayout* layout = new QVBoxLayout( dlg, 11, 6 );
	layout->setAutoAdd( TRUE );
	QTextView* view = new QTextView( dlg );
	QPushButton* btnClose = new QPushButton( "&Close", dlg );
	connect( btnClose, SIGNAL( clicked() ), dlg, SLOT( close() ) );
	view->setText( s );
	view->setFocus();
	dlg->exec();
    }

    bool strictMenuCheck;

private:
    int key;
    bool block;
    QMap<QChar, AccelInfoList > menuAccels;

};

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
  KApplicationPrivate()
  {
    refCount = 1;
    oldIceIOErrorHandler = 0;
    checkAccelerators = 0;
    styleFile="kstylerc";
    startup_id = "0";
  }

  ~KApplicationPrivate()
  {}

  /**
   * This counter indicates when to exit the application.
   * It starts at 1, is decremented by the "last window close" event, but
   * is incremented by operations that should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  int refCount;
  IceIOErrorHandler oldIceIOErrorHandler;
  KCheckAccelerators* checkAccelerators;
  QString styleFile;
   QString geometry_arg;
  QCString startup_id;
};


static QList<QWidget>*x11Filter = 0;
void KApplication::installX11EventFilter( QWidget* filter )
{
    if ( !filter )
        return;
    if (!x11Filter)
        x11Filter = new QList<QWidget>;
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
       static uint _selectAll = KStdAccel::selectAll();
       if (receiver && receiver->inherits("QLineEdit"))
       {
          QLineEdit *edit = static_cast<QLineEdit *>(receiver);
          // We have a keypress for a lineedit...
          QKeyEvent *kevent = static_cast<QKeyEvent *>(event);
          if (KStdAccel::isEqual(kevent, _selectAll))
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
          if (KStdAccel::isEqual(kevent, Qt::CTRL + Qt::Key_U))
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
       if (receiver && receiver->inherits("QMultiLineEdit"))
       {
          QMultiLineEdit *medit = static_cast<QMultiLineEdit *>(receiver);
          // We have a keypress for a multilineedit...
          QKeyEvent *kevent = static_cast<QKeyEvent *>(event);
          if (KStdAccel::isEqual(kevent, _selectAll))
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
static QList<KSessionManaged>* sessionClients()
{
    static QList<KSessionManaged>* session_clients = 0L;
    if ( !session_clients )
        session_clients = new QList<KSessionManaged>;
    return session_clients;
}

/*
  Auxiliary function to calculate a a session config name used for the
  instance specific config object.
  Syntax:  "<appname>:<sessionId>"
 */
static QString sessionConfigName()
{
  QString aSessionConfigName;
  QTextOStream ts( &aSessionConfigName );
  ts << qApp->name() << ":" << qApp->sessionId();
  return aSessionConfigName;
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
  virtual QString find(const char* context, const char* message) const {
      return KGlobal::locale()->translateQt(context, message);
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

  QApplication::setDesktopSettingsAware( false );

  KApp = this;

  smw = 0;

  // Initial KIPC event mask.
  kipcEventMask = (1 << KIPC::StyleChanged) | (1 << KIPC::PaletteChanged) |
                  (1 << KIPC::FontChanged) | (1 << KIPC::BackgroundChanged) |
                  (1 << KIPC::ToolbarStyleChanged);

  // Trigger creation of locale.
  (void) KGlobal::locale();

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
    kipcCommAtom = XInternAtom(display, "KIPC_COMM_ATOM", false);
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

    KConfig* config = KGlobal::config();
    KConfigGroupSaver saver( config, "Development" );
    QString sKey = config->readEntry("CheckAccelerators" );
    if ( !sKey.isEmpty() && KAccel::stringToKey( sKey ) ) {
	d->checkAccelerators = new KCheckAccelerators( this, KAccel::stringToKey( sKey ) );
	d->checkAccelerators->strictMenuCheck = config->readBoolEntry("StrictMenuCheck", false );
    }
  }

  installTranslator(new KDETranslator(this));

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + QString::fromLatin1(name()) + '/');
  pSessionConfig = 0L;
  pDCOPClient = 0L; // don't instantiate until asked to do so.
  bSessionManagement = true;

#ifdef Q_WS_X11
  // register a communication window for desktop changes (Matthias)
  if (GUIenabled && kde_have_kipc )
  {
    Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", false);
    smw = new QWidget(0,0);
    long data = 1;
    XChangeProperty(qt_xdisplay(), smw->winId(), a, a, 32,
                                        PropModeReplace, (unsigned char *)&data, 1);
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
  if (pDCOPClient)
    return pDCOPClient;

  pDCOPClient = new DCOPClient();
  connect(pDCOPClient, SIGNAL(attachFailed(const QString &)),
          SLOT(dcopFailure(const QString &)));
  connect(pDCOPClient, SIGNAL(blockUserInput(bool) ),
          SLOT(dcopBlockUserInput(bool)) );

  DCOPClient::setMainClient( pDCOPClient );
  return pDCOPClient;
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


bool KApplication::requestShutDown()
{
    return requestShutDown( false );
}

bool KApplication::requestShutDown( bool bFast )
{
#ifdef Q_WS_X11
    if ( mySmcConnection ) {
        // we already have a connection to the session manager, use it.
        SmcRequestSaveYourself( mySmcConnection, SmSaveBoth, True,
				SmInteractStyleAny, bFast ? True : False, True );

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

    fName += "-"+display;
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

    if ( firstTime ) {
        firstTime = false;
        return; // no need to save the state.
    }

    QString aLocalFileName = KGlobal::dirs()->saveLocation("config") +
        sessionConfigName();

    // remove former session config if still existing, we want a new and fresh one
    if ( pSessionConfig ) {
        delete pSessionConfig;
        pSessionConfig = 0;
        QFile f ( aLocalFileName );
        if ( f.exists() )
            f.remove();
    }

    // tell the session manager about our new lifecycle
    QStringList restartCommand = sm.restartCommand();

    QCString multiHead = getenv("KDE_MULTIHEAD");
    if (multiHead.lower() == "true")
    {
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
    }

    sm.setRestartCommand( restartCommand );

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
//      discard  << ( "rm "+aLocalFileName ); // only one argument  due to broken xsm
        discard  << "rm" << aLocalFileName; // WABA: Screw xsm
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
  if (kapp)
    setOverrideCursor( Qt::waitCursor );
  my_system(QFile::encodeName(srv)+" --suicide");
  if (kapp)
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
                      "communications for KDE.  The message returned\n"
                      "by the system was:\n\n"));
     msgStr += msg;
     msgStr += i18n("\n\nPlease check that the \"dcopserver\" program is running!");

     QMessageBox::critical(kapp->mainWidget(),
                        i18n("DCOP communications error (%1)").arg(kapp->caption()),
                        msgStr, i18n("OK"));
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
   { "dcopserver <server>",     I18N_NOOP("Use the DCOP Server specified by 'server'."), 0},
   { "nocrashhandler",          I18N_NOOP("Disable crash handler, to get core dumps."), 0},
   { "waitforwm",          I18N_NOOP("Waits for a WM_NET compatible windowmanager."), 0},
   { "style <style>", I18N_NOOP("sets the application GUI style."), 0},
   { "geometry <geometry>", I18N_NOOP("sets the client geometry of the main widget."), 0},
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

    if (args->isSet("style"))
    {
       KGlobal::dirs()->addResourceType("kstylefile", KStandardDirs::kde_default("data") + "kstyle/themes");
       QString style(args->getOption("style"));

       QString styleExtension(".themerc");
       if (style.right(8) == styleExtension)
          style.truncate(style.length()-8);

       // Look in local dir first.
       QString styleFile(style+styleExtension);
       if (!KStandardDirs::exists(styleFile))
       {
          styleFile = locate("kstylefile", style+styleExtension);
          if (styleFile.isEmpty())
          {
             styleFile = locate("kstylefile", "qt"+style+styleExtension);
          }
       }
       if (styleFile.isEmpty())
       {
          fprintf(stderr, "%s", i18n("The style %1 was not found\n").arg(style).local8Bit().data());
       }
       else
       {
          d->styleFile = styleFile;
       }
    }

    if (args->isSet("caption"))
    {
       aCaption = QString::fromLocal8Bit(args->getOption("caption"));
    }

    if (args->isSet("miniicon"))
    {
       const char *tmp = args->getOption("miniicon");
       aMiniIconPixmap = SmallIcon(tmp);
    }

    if (args->isSet("icon"))
    {
       const char *tmp = args->getOption("icon");
       aIconPixmap = DesktopIcon( tmp );
       if (aMiniIconPixmap.isNull())
          aMiniIconPixmap = SmallIcon( tmp );
    }

    if (args->isSet("dcopserver"))
    {
       dcopClient()->setServerAddress( args->getOption("dcopserver"));
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
        Atom a = XInternAtom( qt_xdisplay(), "_NET_SUPPORTED", FALSE  );
        Atom type;
        (void) desktop(); // trigger desktop creation, we need PropertyNotify events for the root window
        int format;
        unsigned long length, after;
        unsigned char *data;
        while ( XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), a, 0, 1, FALSE, AnyPropertyType, &type, &format,
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

    delete args; // Throw away
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

QPixmap KApplication::miniIcon() const
{
  if (aMiniIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication *>(this);
      that->aMiniIconPixmap = SmallIcon( instanceName() );
  }
  return aMiniIconPixmap;
}

KApplication::~KApplication()
{
  KLibLoader::cleanUp();
  KGlobal::deleteStaticDeleters();

  delete smw;

  // close down IPC
  delete pDCOPClient;

  // Carefully shut down the process controller: It is very likely
  // that we receive a SIGCHLD while the destructor is running
  // (since we are in the process of shutting down, an opportunity
  // at which child process are being killed). So we first mark
  // the controller deleted (so that the SIGCHLD handler thinks it
  // is already gone) before we actually delete it.
  KProcessController* ctrl = KProcessController::theKProcessController;
  KProcessController::theKProcessController = 0;
  delete ctrl; // Stephan: "there can be only one" ;)

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

#ifndef Q_WS_QWS
bool KApplication::x11EventFilter( XEvent *_event )
{
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

    KSimpleConfig pConfig(d->styleFile , true );
    QString oldGroup = pConfig.group();
    pConfig.setGroup("KDE");
    QString styleStr = pConfig.readEntry("WidgetStyle",
                                          QPixmap::defaultDepth() > 8 ? "HighColor" : "Default");

#if 0
    // default style is not yet ported - currently looking into it
    if(styleStr == "Default"){
        pKStyle = new KDEStyle;
        setStyle(pKStyle);
        styleHandle=0;
    }
    else
#endif
        if(styleStr == "Windows 95"){
        setStyle("Windows");
    }
    else if(styleStr == "Qt SGI"){
        setStyle("SGI");
    }
    else {
        qDebug( "trying to open styleStr..: %s", styleStr.latin1() );

        QStyle* sp = QStyleFactory::create( styleStr );
        qDebug( "got %p", sp );
        if ( sp )
            setStyle(sp);
    }

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
      if ( withAppName && !caption().isNull() && userCaption != caption()  )
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

    disabledgrp.setColor(QColorGroup::Button, button);
    disabledgrp.setColor(QColorGroup::ButtonText, buttonText);
    disabledgrp.setColor(QColorGroup::Midlight, background.light(110));

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
    if(config->readBoolEntry("EffectNoTooltip", false))
		QToolTip::setEnabled(false);

    emit settingsChanged(arg);
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

   if (startServiceByDesktopName("khelpcenter", url, &error))
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

   if (startServiceByDesktopName("khelpcenter", url, &error))
   {
      kdWarning() << "Could not launch help:\n" << error << endl;
      return;
   }
}


void KApplication::invokeMailer(const QString &address, const QString &subject)
{
   KURL mailtoURL;
   mailtoURL.setProtocol("mailto");
   mailtoURL.setPath(address);
   mailtoURL.setQuery("?subject=" + subject);
   invokeMailer(mailtoURL);
}


void KApplication::invokeMailer(const KURL &mailtoURL)
{
   KConfig config("emaildefaults");
   config.setGroup( QString::fromLatin1("PROFILE_Default") );
   QString command = config.readEntry("EmailClient");

   if (command.isEmpty() || command == QString::fromLatin1("kmail"))
     command = QString::fromLatin1("kmail --composer -s %s -c %c -b %b --body %B --attach %A %t");

   // TODO: Take care of the preferred terminal app (instead of hardcoding
   // Konsole), this will probably require a rewrite of the configurable
   // terminal client option because the placeholder for the program which
   // has to be executed by the terminal has to be supplied (e.g. something
   // like '/opt/kde2/bin/konsole -e %p'). - Frerich
   if (config.readBoolEntry("TerminalClient", false))
      command = "konsole -e " + command;

   QString address = KURL::decode_string(mailtoURL.path()), subject, cc, bcc, body, attach;
   QStringList queries = QStringList::split('&', mailtoURL.query().mid(1));
   for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
     if ((*it).startsWith("subject="))
       subject = KURL::decode_string((*it).mid(8));
     else
     if ((*it).startsWith("cc="))
       cc = KURL::decode_string((*it).mid(3));
     else
     if ((*it).startsWith("bcc="))
       bcc = KURL::decode_string((*it).mid(4));
     else
     if ((*it).startsWith("body="))
       body = KURL::decode_string((*it).mid(5));
     else
     if ((*it).startsWith("attach="))
       attach = KURL::decode_string((*it).mid(7));

   // WARNING: This will only work as long as the path of the
   // email client doesn't contain spaces (this is currently
   // impossible due to an evil hack in kcmemail but should
   // be changed after KDE 2.0!). - Frerich
   QStringList cmdTokens = QStringList::split(' ', command.simplifyWhiteSpace());
   QString cmd = cmdTokens[0];
   cmdTokens.remove(cmdTokens.begin());

   for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); ++it)
     if ((*it).find("%t") >= 0)
       (*it).replace(QRegExp("%t"), address);
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
       (*it).replace(QRegExp("%A"), attach);

   QString error;

   if (kdeinitExec(cmd, cmdTokens, &error))
   {
      kdWarning() << "Could not launch mail client:\n" << error << endl;
   }
}


void KApplication::invokeBrowser( const QString &url )
{
   QString error;

   if (startServiceByDesktopName("kfmclient", url, &error))
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
              QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
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
       stream << startup_id;

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
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::startServiceByName( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   return startServiceInternal(
                      "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::startServiceByDesktopPath( const QString& _name, const QString &URL,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::startServiceByDesktopPath( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   return startServiceInternal(
                      "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::startServiceByDesktopName( const QString& _name, const QString &URL,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   QStringList URLs;
   if (!URL.isEmpty())
      URLs.append(URL);
   return startServiceInternal(
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::startServiceByDesktopName( const QString& _name, const QStringList &URLs,
                  QString *error, QCString *dcopService, int *pid, const QCString& startup_id )
{
   return startServiceInternal(
                      "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString)",
                      _name, URLs, error, dcopService, pid, startup_id);
}

int
KApplication::kdeinitExec( const QString& name, const QStringList &args,
                           QString *error, int *pid )
{
   return startServiceInternal("kdeinit_exec(QString,QStringList,QValueList<QCString>)",
        name, args, error, 0, pid, QCString());
}

int
KApplication::kdeinitExecWait( const QString& name, const QStringList &args,
                           QString *error, int *pid )
{
   return startServiceInternal("kdeinit_exec_wait(QString,QStringList,QValueList<QCString>)",
        name, args, error, 0, pid, QCString());
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
    QCString string_buffer = instanceName().data(); // copies it

    char * argv = string_buffer.data();

#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
    XSetCommand(display, leader, &argv, 1);

    XClassHint hint;
    hint.res_name = string_buffer.data();
    hint.res_class = const_cast<char *>("toplevel");
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

#include "kapp.moc"
