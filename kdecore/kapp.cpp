/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdir.h> // must be at the front
#include <qobjcoll.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qwidcoll.h>
#include <qpopupmenu.h>
#include <qsessionmanager.h>

#include <kapp.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kdebugdialog.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <dcopclient.h>
#include <qlist.h>
#include <qsessionmanager.h>
#include <qtranslator.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#include "ltdl.h"

#include "kwm.h"

#include <fcntl.h>
#include <stdlib.h> // getenv()
#include <signal.h>
#include <unistd.h>

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>

// defined by X11 headers
#ifdef KeyPress
#undef KeyPress
#endif

KApplication* KApplication::KApp = 0L;
//extern bool bAreaCalculated;

static int kde_xio_errhandler( Display * )
{
  return kapp->xioErrhandler();
}

static int kde_x_errhandler( Display *dpy, XErrorEvent *err )
{
    char errstr[256];
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    qWarning( "KDE detected X Error: %s %d\n  Major opcode:  %d", errstr, err->error_code, err->request_code );
    return 0;
}

/*
  Private data to make keeping binary compatibility easier
 */
class KApplicationPrivate
{
public:
    KApplicationPrivate()
    {
    }
    ~KApplicationPrivate()
    {
    }
};

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
  Syntax:  "<appname>:<level>:<sessionId>"
 */
static int session_restore_level = 0;
static QString sessionConfigName()
{
  QString aSessionConfigName;
  QTextOStream ts( &aSessionConfigName );
  ts << qApp->name() << ":" << session_restore_level << ":" << qApp->sessionId();
  return aSessionConfigName;
}

KApplication::KApplication( int& argc, char** argv, const QString& rAppName ) :
    QApplication( argc, argv )
{
    if (!rAppName.isEmpty())
	QApplication::setName(rAppName.ascii());

    pAppData = new KApplicationPrivate;

    init();
    parseCommandLine( argc, argv );

}

KConfig* KApplication::config() const { return KGlobal::config(); }

int KApplication::xioErrhandler()
{
  emit shutDown();
  exit( 1 );
  return 0;
}

class KDETranslator : public QTranslator
{
public:
  KDETranslator(QObject *parent) : QTranslator(parent, "kdetranslator") {}
  virtual QString find(const char*, const char* message) const {
    return i18n(message);
  }
};

void KApplication::init()
{
  QApplication::setDesktopSettingsAware( false );
  // this is important since we fork() to launch the help (Matthias)
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  // set up the fancy (=robust and error ignoring ) KDE xio error handlers (Matthias)
  XSetErrorHandler( kde_x_errhandler );
  XSetIOErrorHandler( kde_xio_errhandler );

  connect( this, SIGNAL( aboutToQuit() ), this, SIGNAL( shutDown() ) );

  // CC: install KProcess' signal handler
  // by creating the KProcController instance (if its not already existing)
  // This is handled be KProcess (stefh)
  /*
  if ( theKProcessController == 0L)
    theKProcessController = new KProcessController();
  */
  KApp = this;

  styleHandle = 0;
  pKStyle = 0;

  display = desktop()->x11Display();

  KDEChangePalette = XInternAtom( display, "KDEChangePalette", False );
  KDEChangeGeneral = XInternAtom( display, "KDEChangeGeneral", False );
  KDEChangeStyle = XInternAtom( display, "KDEChangeStyle", False);

  readSettings(false);
  kdisplaySetPalette();
  kdisplaySetStyleAndFont();

  installTranslator(new KDETranslator(this));

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + name() + "/");

  // install an event filter for KDebug
  installEventFilter( this );

  pSessionConfig = 0L;
  pDCOPClient = new DCOPClient(name());
  pDCOPClient->attach(); // attach to DCOP server.
  bSessionManagement = true;

  // register a communication window for desktop changes (Matthias)
  {
    Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
    smw = new QWidget(0,0);
    long data = 1;
    XChangeProperty(qt_xdisplay(), smw->winId(), a, a, 32,
					PropModeReplace, (unsigned char *)&data, 1);
  }

  captionLayout = CaptionAppLast;
}

DCOPClient *KApplication::dcopClient() const
{
  return pDCOPClient;
}

KConfig* KApplication::sessionConfig() {
  if (pSessionConfig)
    return pSessionConfig;

  // create an instance specific config object
  pSessionConfig = new KConfig( sessionConfigName(), false, false);

  return pSessionConfig;
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
    return TRUE;
}

bool KSessionManaged::commitData(QSessionManager&)
{
    return TRUE;
}


void KApplication::disableSessionManagement() {
  bSessionManagement = True;
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

    if ( !bSessionManagement ) {
	sm.setRestartHint( QSessionManager::RestartNever );
	return;
    }
}

void KApplication::saveState( QSessionManager& sm )
{
    static bool firstTime = FALSE;

    if ( !bSessionManagement ) {
	sm.setRestartHint( QSessionManager::RestartNever );
	return;
    }

    if ( firstTime ) {
	firstTime = FALSE;
	return; // no need to save the state.
    }


    // remove former session config, we need a new one
    if ( pSessionConfig ) {
	delete pSessionConfig;
	pSessionConfig = 0;
    }

    // increase restoration level, we are in a new lifecycle now
    QFile file;
    do {
	session_restore_level++;
	QString aLocalFileName = KGlobal::dirs()->saveLocation("config") +
				 sessionConfigName();
	file.setName( aLocalFileName );
    } while ( file.exists() );

    // tell the session manager about our new lifecycle
    QStringList restartCommand = sm.restartCommand();
    QString restoreLevel;
    restoreLevel.setNum( session_restore_level );
    restartCommand << "-restore" << restoreLevel;
    sm.setRestartCommand( restartCommand );

    // finally: do session management
    emit saveYourself();
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
	discard  << ( QString("rm ")+file.name()); // only one argument  due to broken xsm
	sm.setDiscardCommand( discard );
    }

    if ( cancelled )
	sm.cancel();
}

QPopupMenu* KApplication::helpMenu( bool /*bAboutQtMenu*/,
	   const QString& aboutAppText )
{
  int id = 0;
  QPopupMenu* pMenu = new QPopupMenu();

  id = pMenu->insertItem( i18n( "&Contents" ) );
  pMenu->connectItem( id, this, SLOT( appHelpActivated() ) );
  pMenu->setAccel( Key_F1, id );

  pMenu->insertSeparator();

  id = pMenu->insertItem( i18n( "&About" ) + " " + name() + "...");
  if( !aboutAppText.isNull() )
	{
	  pMenu->connectItem( id, this, SLOT( aboutApp() ) );
	  aAppAboutString = aboutAppText;
	}

  id = pMenu->insertItem( i18n( "About &KDE..." ) );
  pMenu->connectItem( id, this, SLOT( aboutKDE() ) );
  /*
	if( bAboutQtMenu )
	{
	id = pMenu->insertItem( i18n( "About Qt" ) );
	pMenu->connectItem( id, this, SLOT( aboutQt() ) );
	}
  */
  return pMenu;
}


void KApplication::appHelpActivated()
{
  invokeHTMLHelp( QString(name()) + "/" + "index.html", "" );
}


void KApplication::aboutKDE()
{
  QMessageBox about(i18n( "About KDE" ),
		    i18n(
			 "\nThe KDE Desktop Environment was written by the KDE Team,\n"
			 "a world-wide network of software engineers committed to\n"
			 "free software development.\n\n"
			 "Visit http://www.kde.org for more information on the KDE\n"
			 "Project. Please consider joining and supporting KDE.\n\n"
			 "Please report bugs at http://bugs.kde.org.\n"),
		    QMessageBox::Information,
		    QMessageBox::Ok + QMessageBox::Default, 0, 0,
		    0, "aboutkde");
  about.setButtonText(0, i18n("&OK"));
  about.exec();
}

void KApplication::aboutApp()
{
  QWidget* w = activeWindow();
  QString caption = i18n("About %1").arg(kapp->caption());
  QMessageBox about(caption, aAppAboutString, QMessageBox::Information,
		   QMessageBox::Ok + QMessageBox::Default, 0, 0, w, "aboutapp");
  about.setButtonText(0, i18n("&OK"));
  about.setIconPixmap(icon());
  about.exec();
}


void KApplication::aboutQt(){
   //  QWidget* w = activeWindow();
  //  QMessageBox::aboutQt( w, caption() );
}


bool KApplication::eventFilter ( QObject*, QEvent* e )
{
  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent *k = (QKeyEvent*)e;
    if( ( k->key() == Key_F12 ) &&
	( k->state() & ControlButton ) &&
	( k->state() & ShiftButton ) )
    {
      QWidget *w = activeWindow();
      KDebugDialog* pDialog = new KDebugDialog(w);
      /* Fill dialog fields with values from config data */
      KConfig* pConfig = KGlobal::config();
      QString aOldGroup = pConfig->group();
      pConfig->setGroup( "KDebug" );
      pDialog->setInfoOutput( pConfig->readNumEntry( "InfoOutput", 4 ) );
      pDialog->setInfoFile( pConfig->readEntry( "InfoFilename","kdebug.dbg" ));
      pDialog->setInfoShow( pConfig->readEntry( "InfoShow", "" ) );
      pDialog->setWarnOutput( pConfig->readNumEntry( "WarnOutput", 4 ) );
      pDialog->setWarnFile( pConfig->readEntry( "WarnFilename","kdebug.dbg" ));
      pDialog->setWarnShow( pConfig->readEntry( "WarnShow", "" ) );
      pDialog->setErrorOutput( pConfig->readNumEntry( "ErrorOutput", 4 ) );
      pDialog->setErrorFile(pConfig->readEntry( "ErrorFilename","kdebug.dbg"));
      pDialog->setErrorShow( pConfig->readEntry( "ErrorShow", "" ) );
      pDialog->setFatalOutput( pConfig->readNumEntry( "FatalOutput", 4 ) );
      pDialog->setFatalFile( pConfig->readEntry("FatalFilename","kdebug.dbg"));
      pDialog->setFatalShow( pConfig->readEntry( "FatalShow", "" ) );
      pDialog->setAbortFatal( pConfig->readNumEntry( "AbortFatal", 0 ) );
								
      /* Show dialog */
      int nRet = pDialog->exec();
      if( nRet == QDialog::Accepted )
      {
	/* User pressed OK, retrieve values */
	pConfig->writeEntry( "InfoOutput", pDialog->infoOutput() );
	pConfig->writeEntry( "InfoFilename", pDialog->infoFile() );
	pConfig->writeEntry( "InfoShow", pDialog->infoShow() );
	pConfig->writeEntry( "WarnOutput", pDialog->warnOutput() );
	pConfig->writeEntry( "WarnFilename", pDialog->warnFile() );
	pConfig->writeEntry( "WarnShow", pDialog->warnShow() );
	pConfig->writeEntry( "ErrorOutput", pDialog->errorOutput() );
	pConfig->writeEntry( "ErrorFilename", pDialog->errorFile() );
	pConfig->writeEntry( "ErrorShow", pDialog->errorShow() );
	pConfig->writeEntry( "FatalOutput", pDialog->fatalOutput() );
	pConfig->writeEntry( "FatalFilename", pDialog->fatalFile() );
	pConfig->writeEntry( "FatalShow", pDialog->fatalShow() );
	pConfig->writeEntry( "AbortFatal", pDialog->abortFatal() );
	
	//bAreaCalculated = false;
      }
      else
      {
	/* User pressed Cancel, do nothing */
      }
      
      /* restore old group */
      pConfig->setGroup( aOldGroup );
      
      return true; // do not process event further
    }
  }
  return false; // process event further
}


void KApplication::parseCommandLine( int& argc, char** argv )
{
    enum parameter_code { unknown = 0, caption, icon, miniicon, restore };
    const char* parameter_strings[] = { "-caption", "-icon", "-miniicon", "-restore" , 0 };

    aDummyString2 = " ";
    int i = 1;
    parameter_code parameter;
    while( i < argc ) {
	parameter = unknown;

	for ( int p = 0 ; parameter_strings[p]; p++)
	    if ( !strcmp( argv[i], parameter_strings[p]) ) {
		parameter = static_cast<parameter_code>(p + 1);
		break;
	    }

	if ( parameter != unknown && argc < i +2 ) { // last argument without parameters
	    argc -= 1;
	    break; // jump out of the while loop
	}

	switch (parameter) {
	case caption:
	    aCaption = argv[i+1];
	    aDummyString2 += parameter_strings[caption-1];
	    aDummyString2 += " \"";
	    aDummyString2 += argv[i+1];
	    aDummyString2 += "\" ";
	    break;
	case icon:
	    if (argv[i+1][0] == '/')
		aIconPixmap = QPixmap(argv[i+1]);
	    else
		aIconPixmap = KGlobal::iconLoader()->loadApplicationIcon( argv[i+1] );
	    if (aMiniIconPixmap.isNull())
		aMiniIconPixmap = KGlobal::iconLoader()->loadApplicationMiniIcon( argv[i+1] );
	    aDummyString2 += parameter_strings[icon-1];
	    aDummyString2 += " ";
	    aDummyString2 += argv[i+1];
	    aDummyString2 += " ";
	    break;
	case miniicon:
	    aMiniIconPixmap = KGlobal::iconLoader()->loadApplicationMiniIcon( argv[i+1] );
	    aDummyString2 += parameter_strings[miniicon-1];
	    aDummyString2 += " ";
	    aDummyString2 += argv[i+1];
	    aDummyString2 += " ";
	    break;
	case restore:
	    session_restore_level = QString( argv[i+1] ).toInt();
	    delete pSessionConfig; // should be 0 anyway
	    // create a new read-only session config for the restortation
	    pSessionConfig = new KConfig( sessionConfigName(), true, false);
	    break;
	case unknown:
	    i++;
	}

	if ( parameter != unknown ) { // remove arguments

	    for( int j = i;  j < argc-2; j++ )
		argv[j] = argv[j+2];

	    argc -=2 ;
	}

    }
    pArgc = argc;
}

QPixmap KApplication::icon() const
{
  if( aIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication*>(this);
      that->aIconPixmap = KGlobal::iconLoader()->loadApplicationIcon( QString(name()) + ".xpm");
  }
  return aIconPixmap;
}

QPixmap KApplication::miniIcon() const
{
  if (aMiniIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication*>(this);
      that->aMiniIconPixmap = KGlobal::iconLoader()->loadApplicationMiniIcon( QString(name()) + ".xpm" );
  }
  return aMiniIconPixmap;
}
KApplication::~KApplication()
{
  removeEventFilter( this );

  delete smw;

  KGlobal::freeAll();

  // close down IPC
  delete pDCOPClient;

  // Carefully shut down the process controller: It is very likely
  // that we receive a SIGCHLD while the destructor is running
  // (since we are in the process of shutting down, an opportunity
  // at which child process are being killed). So we first mark
  // the controller deleted (so that the SIGCHLD handler thinks it
  // is already gone) before we actually delete it.
  KProcessController* ctrl = theKProcessController;
  theKProcessController = 0;
  delete ctrl; // Stephan: "there can be only one" ;)

  delete pAppData;
  KApp = 0;
}

bool KApplication::x11EventFilter( XEvent *_event )
{
  if ( _event->type == ClientMessage ) {
    XClientMessageEvent *cme = ( XClientMessageEvent * ) _event;

    // stuff for reconfiguring
    if ( cme->message_type == KDEChangeStyle ) {
	applyGUIStyle(WindowsStyle); // arg doesn't matter
	return true;
    }

    if ( cme->message_type == KDEChangePalette )
      {
	readSettings(true);
	kdisplaySetPalette();
	
	return True;
      }
    if ( cme->message_type == KDEChangeGeneral )
      {
	readSettings(true);
	kdisplaySetPalette();
	kdisplaySetStyleAndFont();
	
	return True;
      }
  }

  return false;
}

void KApplication::applyGUIStyle(GUIStyle /* pointless */) {
    /* Hey, we actually do stuff here now :)
     * The widgetStyle key is used as a style string. If it matches a
     * Qt internal style that is used, otherwise it is checked to see
     * if it matches a lib name in either $(KDEDIR)/lib or
     * ~/.kde/share/apps/kstyle/modules. If it does we assume it's a style
     * plugin and try to dlopen and allocate a KStyle. If libtool dlopen
     * isn't supported that's no problem, plugins just won't work and you'll
     * be restricted to the internal styles.
     *
     * mosfet@jorsm.com
     */

    static bool dlregistered = false;

    KSimpleConfig pConfig( "kstylerc", true );
    QString oldGroup = pConfig.group();
    pConfig.setGroup("KDE");
    QString styleStr = pConfig.readEntry("widgetStyle", "Platinum");

    if(styleHandle){
      // warning("KApp: Unloading previous style plugin.");
      lt_dlclose((lt_dlhandle*)styleHandle);
      styleHandle = 0;
    }

    if(styleStr == "Platinum"){
        pKStyle=0;
        styleHandle=0;
        setStyle(new QPlatinumStyle);
    }
    else if(styleStr == "Windows 95"){
        pKStyle=0;
        styleHandle=0;
        setStyle(new QWindowsStyle);
    }
    else if(styleStr == "CDE"){
        pKStyle=0;
        styleHandle=0;
        setStyle(new QCDEStyle);
    }
    else if(styleStr == "Motif"){
        pKStyle=0;
        styleHandle=0;
        setStyle(new QMotifStyle);
    }
    else{
        if(!dlregistered){
            dlregistered = true;
            lt_dlinit();
        }

        if(!locate("lib", styleStr).isNull()) {
          styleStr = locate("lib", styleStr);
          styleHandle = lt_dlopen(styleStr.ascii());
        }
        else {
          warning("KApp: Unable to find style plugin %s.", styleStr.ascii());
          pKStyle = 0;
          styleHandle=0;
          setStyle(new QPlatinumStyle);
          return;
        }

        if(!styleHandle){
            warning("KApp: Unable to open style plugin %s (%s).",
		    styleStr.ascii(), lt_dlerror());

            pKStyle = 0;
            setStyle(new QPlatinumStyle);
        }
        else{
            lt_ptr_t alloc_func = lt_dlsym(styleHandle,
                                           "allocate");
            if(!alloc_func){
                warning("KApp: Unable to init style plugin %s (%s).",
			styleStr.ascii(), lt_dlerror());
                pKStyle = 0;
                lt_dlclose(styleHandle);
                styleHandle = 0;
                setStyle(new QPlatinumStyle);
            }
            else{
                KStyle* (*alloc_ptr)();
                alloc_ptr = (KStyle* (*)())alloc_func;
                pKStyle = alloc_ptr();
                if(pKStyle){
                    setStyle(pKStyle);
                }
                else{
                    warning("KApp: Style plugin unable to allocate style.");
                    pKStyle = 0;
                    setStyle(new QPlatinumStyle);
                    lt_dlclose(styleHandle);
                    styleHandle = 0;
                }
            }
        }
    }
}


QString KApplication::caption() const
{
  if( !aCaption.isNull() )
	return aCaption;
  else
	return name();
}


//
// 1999-09-20: Espen Sand
// An attempt to simplify consistent captions. 
//
QString KApplication::makeStdCaption( const QString &userCaption, 
	                              bool withAppName, bool modified ) const
{
  if( userCaption.isNull() == true )
  {
    return( caption() );
  }

  //
  // This string should be collected from a global object.
  //
  QString modString = i18n("**");
  if( modified == true ) 
  {
    modified = modString.isNull() == true ? false : true;
  }

  if( withAppName == true )
  {
    if( captionLayout == CaptionAppLast )
    {
      if( modified == true )
      {
	return( QString("%1 %2 - %3").arg(modString).arg(userCaption).
		arg(caption()));
      }
      else
      {
	return( QString("%1 - %2").arg(userCaption).arg(caption()));
      }
    }
    else if( captionLayout == CaptionAppFirst )
    {
      if( modified == true )
      {
	return( QString("%1: %2 %3").arg(caption()).arg(modString).
		arg(userCaption) );
      }
      else
      {
	return( QString("%1: %2").arg(caption()).arg(userCaption) );
      }
    }
  }
  
  if( modified == true )
  {
    return( QString("%1 %2").arg(modString).arg(userCaption) );
  }
  else
  {
    return( userCaption );
  }
}






void KApplication::readSettings(bool reparse)
{
  // use the global config files
  KConfig* config = KGlobal::config();
  if (reparse)
      config->reparseConfiguration();

  config->setGroup( "WM");
  // this default is Qt lightGray
  inactiveTitleColor_ = config->readColorEntry( "inactiveBackground", &lightGray );

  // this default is Qt darkGrey
  inactiveTextColor_ = config->readColorEntry( "inactiveForeground", &darkGray );

  // this default is Qt darkBlue
  activeTitleColor_ = config->readColorEntry( "activeBackground", &darkBlue );

  // this default is Qt white
  activeTextColor_ = config->readColorEntry( "activeForeground", &white );

  config->setGroup( "KDE");
  contrast_ = config->readNumEntry( "contrast", 7 );

  //  Read the font specification from config.
  //  Initialize fonts to default first or it won't work !!
	
  // cursor blink rate
  //
  int num = config->readNumEntry( "cursorBlinkRate", cursorFlashTime() );
  // filter out bogus numbers
  if ( num < 200 ) num = 200;
  if ( num > 2000 ) num = 2000;
  setCursorFlashTime(num);

	
}



void KApplication::kdisplaySetPalette()
{
    // the following is temporary and will soon dissappear (Matthias, 3.August 1999 )

  KConfigBase* config;
  config  = kapp->config();
  config->setGroup( "General" );

  QColor buttonFallback =
    config->readColorEntry( "background", &lightGray );
  QColor button =
    config->readColorEntry( "buttonBackground", &buttonFallback );

  QColor buttonTextFallback =
    config->readColorEntry( "foreground", &black );
  QColor buttonText =
    config->readColorEntry( "buttonForeground", &buttonTextFallback );

  QColor background =
    config->readColorEntry( "background", &lightGray );

  QColor highlight =
    config->readColorEntry( "selectBackground", &darkBlue);

  QColor highlightedText =
    config->readColorEntry( "selectForeground", &white );

  QColor base =
    config->readColorEntry( "windowBackground", &white );

  QColor foreground =
    config->readColorEntry( "windowForeground", &black );


  int contrast =
    config->readNumEntry( "contrast", 7 );

  int highlightVal, lowlightVal;

  highlightVal=100+(2*contrast+4)*16/10;
  lowlightVal=100+(2*contrast+4)*10;


  QColorGroup disabledgrp( foreground, background,
			   background.light(150),
			   background.dark(),
			   background.dark(120),
			   background.dark(120), base );

  QColorGroup colgrp( foreground, background,
		      background.light(150),
		      background.dark(),
		      background.dark(120),
		      foreground, base );

  colgrp.setColor( QColorGroup::Highlight, highlight);
  colgrp.setColor( QColorGroup::HighlightedText, highlightedText);
  colgrp.setColor( QColorGroup::Button, button);
  colgrp.setColor( QColorGroup::ButtonText, buttonText);
  disabledgrp.setColor( QColorGroup::Button, button);
  disabledgrp.setColor( QColorGroup::ButtonText, buttonText);

  setPalette( QPalette( colgrp, disabledgrp, colgrp), true );

  applyGUIStyle( WindowsStyle ); // to fix the palette again
  emit kdisplayPaletteChanged();
  emit appearanceChanged();
}

void KApplication::kdisplaySetFont()
{
    QApplication::setFont( KGlobal::generalFont(), true );

    emit kdisplayFontChanged();
    emit appearanceChanged();

    resizeAll();
}	


void KApplication::kdisplaySetStyle()
{
  applyGUIStyle( WindowsStyle );

  emit kdisplayStyleChanged();
  emit appearanceChanged();
  resizeAll();
}	


void KApplication::kdisplaySetStyleAndFont()
{
    QApplication::setFont( KGlobal::generalFont(), true );
    kdisplaySetPalette(); // does both palette and style
    emit kdisplayFontChanged();

    resizeAll();
}	


void KApplication::resizeAll()
{
    return;
  // send a resize event to all windows so that they can resize children
  QWidgetList *widgetList = QApplication::topLevelWidgets();
  QWidgetListIt it( *widgetList );

  while ( it.current() )
	{
	  it.current()->resize( it.current()->size() );
	  ++it;
	}
  delete widgetList;
}




void KApplication::invokeHTMLHelp( QString filename, QString topic ) const
{
  if ( fork() == 0 )	
    {		
	  if( filename.isEmpty() )
	    filename = QString(name()) + "/index.html";

         // first try the locale setting
         QString file = locate("html", KGlobal::locale()->language() + '/' + filename);
	 if (file.isNull())
	     file = locate("html", "default/" + filename);

	 if (file.isNull()) {
	     warning("no help file %s found\n", filename.ascii());
	     return;
	 }

	 if( !topic.isEmpty() ) {
	     file.append( "#" );
	     file.append(topic);
	 }
	
	  /* Since this is a library, we must conside the possibilty that
	   * we are being used by a suid root program. These next two
	   * lines drop all privileges.
	   */
	  setuid( getuid() );
	  setgid( getgid() );
	  const char* shell = "/bin/sh";
	  if (getenv("SHELL"))
		shell = getenv("SHELL");
         file.prepend("khcclient ");
         execl(shell, shell, "-c", file.ascii(), 0L);
	  exit( 1 );
    }
}


void KApplication::invokeMailer(const QString &address,const QString &subject )
{
  if( fork() == 0 )
  {	
    QString mailClient( "kmail");
    QString exec = QString("%1 %2 -s %3").arg(mailClient).arg(address).
      arg(subject);

    setuid( getuid() ); // Make sure a set-user-id prog. is not root anymore
    setgid( getgid() );

    const char* shell = "/bin/sh";
    if( getenv("SHELL") ) 
    {
      shell = getenv("SHELL");
    }
    execl( shell, shell, "-c", exec.ascii(), 0L );
    exit( 1 );
  }
}


void KApplication::invokeBrowser( const QString &url )
{
  if( fork() == 0 )
  { 
    setuid( getuid() ); // Make sure a set-user-id prog. is not root anymore
    setgid( getgid() );
    QString browser, exec;
    
    //
    // 1999-10-05 Espen Sand 
    //
    // The code below is inspired by the work of Markus Goetz <guruz@gmx.de>
    // It should not be here at all. We should rather just use a method
    // that returns the correct browser exec string:
    // exec = KSomeGlobalObject::browser( url ):
    // and it must be done before the fork() above I'll guess. Is that OK
    // with Qt/QString?
    //
    #if 0
    if( 0 )
    {
      browser = "lynx";
      exec = QString("konsole -e %1 %2").arg(browser).arg(url);
    }
    else if(0)
    {
      browser = "netscape";
      QString lockFile = QString("%1/.netscape/lock").arg(getenv("HOME"));

      struct stat statInfo;
      if( lstat(lockFile.ascii(), &statInfo) != -1 )
      {
	exec = QString("%1 -remote 'openURL(%2,new-window)'").arg(browser).
	  arg(url);
      }
      else
      {
	exec = QString("%1 %2").arg(browser).arg(url);
      }
    }
    else
    {
      browser = "kfmclient";
      exec = QString("%1 openURL %2").arg(browser).arg(url);
    }
    #endif


    //
    // This is what we use now.
    //
    browser = "kfmclient";
    exec = QString("%1 openURL %2").arg(browser).arg(url);

    const char* shell = "/bin/sh";
    if( getenv("SHELL") ) 
    {
      shell = getenv("SHELL");
    }
    execl( shell, shell, "-c", exec.ascii(), 0L );
    exit( 1 );
  }

}




bool KApplication::kdeFonts(QStringList &fontlist) const
{
  QString fontfilename = KGlobal::dirs()->saveLocation("config") + "kdefonts";
  QFile fontfile(fontfilename);

  if (!fontfile.exists())
    return false;

  if(!fontfile.open(IO_ReadOnly)){
    return false;
  }

  QTextStream t(&fontfile);


  while ( !t.eof() ) {
    QString s = t.readLine();
    if(!s.isEmpty())
      fontlist.append( s );
  }

  fontfile.close();

  return true;
}


QString KApplication::tempSaveName( const QString& pFilename ) const
{
  QString aFilename;

  if( pFilename[0] != '/' )
    {
      kdebug( KDEBUG_WARN, 101, "Relative filename passed to KApplication::tempSaveName" );
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
      kdebug( KDEBUG_WARN, 101, "Relative filename passed to KApplication::tempSaveName" );
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
  int accessOK = access( pathname.ascii(), mode );
  if ( accessOK == 0 )
    return true;  // OK, I can really access the file

  // else
  // if we want to write the file would be created. Check, if the
  // user may write to the directory to create the file.
  if ( mode & W_OK == 0 )
    return false;   // Check for write access is not part of mode => bail out


  if (!access( pathname.ascii(), F_OK)) // if it already exists
      return false;

  //strip the filename (everything until '/' from the end
  QString dirName(pathname);
  int pos = dirName.findRev('/');
  if ( pos == -1 )
    return false;   // No path in argument. This is evil, we won't allow this

  dirName.truncate(pos); // strip everything starting from the last '/'

  accessOK = access( dirName.ascii(), W_OK );
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
    // set the specified caption
    topWidget->setCaption( caption() );
    // set the specified icons
    KWM::setIcon(topWidget->winId(), icon());
    KWM::setMiniIcon(topWidget->winId(), miniIcon());
    // set a short icon text
    // TODO: perhaps using .ascii() isn't right here as this may be seen by
    // a user?
    XSetIconName( qt_xdisplay(), topWidget->winId(), caption().ascii() );
  }
}

QColor KApplication::inactiveTitleColor() const
{
    return inactiveTitleColor_;
}


QColor KApplication::inactiveTextColor() const
{
    return inactiveTextColor_;
}


QColor KApplication::activeTitleColor() const
{
    return activeTitleColor_;
}


QColor KApplication::activeTextColor() const
{
    return activeTextColor_;
}

int KApplication::contrast() const
{
    return contrast_;
}


// pointless, to be removed########
Qt::GUIStyle KApplication::applicationStyle() const
{
    return style();
}


#include "kapp.moc"


