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

#include <kapp.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kdebugdialog.h>
#include <klocale.h>
#include <kiconloader.h>

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

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

// defined by X11 headers
#ifdef KeyPress
#undef KeyPress
#endif

#include <kstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>
#include <kconfig.h>
#include <kstddirs.h>

KApplication* KApplication::KApp = 0L;
//extern bool bAreaCalculated;

static int kde_xio_errhandler( Display * )
{
  return kapp->xioErrhandler();
}

KApplication::KApplication( int& argc, char** argv, const QString& rAppName ) :
    QApplication( argc, argv )
{
    if (!rAppName.isEmpty())
	QApplication::setName(rAppName.ascii());
    init();
    parseCommandLine( argc, argv );

}

KConfig* KApplication::getConfig() const { return KGlobal::config(); }

int KApplication::xioErrhandler()
{
  emit shutDown();
  exit( 1 );
  return 0;
}

void KApplication::init()
{
  QApplication::setDesktopSettingsAware( false );
  // this is important since we fork() to launch the help (Matthias)
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  // set up the fance KDE xio error handler (Matthias)
  XSetIOErrorHandler( kde_xio_errhandler );

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

  WM_SAVE_YOURSELF = XInternAtom( display, "WM_SAVE_YOURSELF", False );
  WM_PROTOCOLS = XInternAtom( display, "WM_PROTOCOLS", False );
  KDEChangePalette = XInternAtom( display, "KDEChangePalette", False );
  KDEChangeGeneral = XInternAtom( display, "KDEChangeGeneral", False );
  KDEChangeStyle = XInternAtom( display, "KDEChangeStyle", False);

  readSettings(false);
  kdisplaySetPalette();
  kdisplaySetStyleAndFont();

  // install appdata resource type
  KGlobal::dirs()->addResourceType("appdata", KStandardDirs::kde_default("data")
                                   + name() + "/");

  // install an event filter for KDebug
  installEventFilter( this );

  pSessionConfig = 0L;
  bIsRestored = False;
  bSessionManagement = False;
  bSessionManagementUserDefined = False;
  pTopWidget = 0L;

  // register a communication window for desktop changes (Matthias)
  {
    Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
    smw = new QWidget(0,0);
    long data = 1;
    XChangeProperty(qt_xdisplay(), smw->winId(), a, a, 32,
					PropModeReplace, (unsigned char *)&data, 1);
  }
  aWmCommand = argv()[0];
}

KConfig* KApplication::getSessionConfig() {
  if (pSessionConfig)
    return pSessionConfig;

  // create a instance specific config object
  QString aConfigName = KGlobal::dirs()->getSaveLocation("config");
  aConfigName += name();
  aConfigName += "rc";

  QString aSessionConfigName;
  QString num;
  int i = 0;
  do {
    i++;
    num.setNum(i);
    aSessionConfigName = aConfigName + "." + num;
  } while (QFile::exists(aSessionConfigName));
  QFile aConfigFile(aSessionConfigName);

  bool bSuccess;
  if ( ! checkAccess( aConfigFile.name(), W_OK ) )
    bSuccess = false;
  else {
    bSuccess = aConfigFile.open( IO_ReadWrite );
  }
  if( bSuccess ){
    chown(aConfigFile.name().ascii(), getuid(), getgid());
    aConfigFile.close();
    pSessionConfig = new KConfig(aSessionConfigName);
    aSessionName = name();
    aSessionName += "rc.";
    aSessionName += num;
  }
  return pSessionConfig;
}

void KApplication::enableSessionManagement(bool userdefined){
  bSessionManagement = True;
  bSessionManagementUserDefined = userdefined;
  if (topWidget()){
    KWM::enableSessionManagement(topWidget()->winId());
  }
}

void KApplication::setWmCommand(const QString& s){
  aWmCommand = s;
  if (topWidget() && !bSessionManagement)
    KWM::setWmCommand( topWidget()->winId(), aWmCommand);
}

QPopupMenu* KApplication::getHelpMenu( bool /*bAboutQtMenu*/,
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
  QMessageBox::about( 0L, i18n( "About KDE" ),
					  i18n(
"\nThe KDE Desktop Environment was written by the KDE Team,\n"
"a world-wide network of software engineers committed to\n"
"free software development.\n\n"
"Visit http://www.kde.org for more information on the KDE\n"
"Project. Please consider joining and supporting KDE.\n\n"
"Please report bugs at http://bugs.kde.org.\n"
));
}

void KApplication::aboutApp()
{
  QMessageBox::about( 0L, getCaption(), aAppAboutString );
}


void KApplication::aboutQt()
{
  //  QMessageBox::aboutQt( 0, getCaption() );
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
		  KDebugDialog* pDialog = new KDebugDialog();
		  /* Fill dialog fields with values from config data */
		  KConfig* pConfig = KGlobal::config();
		  QString aOldGroup = pConfig->group();
		  pConfig->setGroup( "KDebug" );
		  pDialog->setInfoOutput( pConfig->readNumEntry( "InfoOutput", 4 ) );
		  pDialog->setInfoFile( pConfig->readEntry( "InfoFilename",
													"kdebug.dbg" ) );
		  pDialog->setInfoShow( pConfig->readEntry( "InfoShow", "" ) );
		  pDialog->setWarnOutput( pConfig->readNumEntry( "WarnOutput", 4 ) );
		  pDialog->setWarnFile( pConfig->readEntry( "WarnFilename",
													"kdebug.dbg" ) );
		  pDialog->setWarnShow( pConfig->readEntry( "WarnShow", "" ) );
		  pDialog->setErrorOutput( pConfig->readNumEntry( "ErrorOutput", 4 ) );
		  pDialog->setErrorFile( pConfig->readEntry( "ErrorFilename",
													 "kdebug.dbg" ) );
		  pDialog->setErrorShow( pConfig->readEntry( "ErrorShow", "" ) );
		  pDialog->setFatalOutput( pConfig->readNumEntry( "FatalOutput", 4 ) );
		  pDialog->setFatalFile( pConfig->readEntry( "FatalFilename",
													 "kdebug.dbg" ) );
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
	case restore: {
	    aSessionName = argv[i+1];
	    QString aSessionConfigName = locate("config", argv[i+1]);
	    if (QFile::exists(aSessionConfigName)){
		QFile aConfigFile(aSessionConfigName);
		bool bSuccess;
		if ( ! checkAccess( aConfigFile.name(), W_OK ) )
		    bSuccess = false;
		else
		    bSuccess = aConfigFile.open( IO_ReadWrite );
		if( bSuccess ){
		    // Set uid/gid (neccesary for SUID programs)
		    chown(aConfigFile.name().ascii(), getuid(), getgid());
		    
		    aConfigFile.close();
		    pSessionConfig = new KConfig(QString::null, !aSessionConfigName.isNull());
		    
		    // do not write back. the application will get
		    // a new one if demanded.
		    pSessionConfig->rollback();
		    
		    if (pSessionConfig){
			bIsRestored = True;
		    }
		    aConfigFile.remove();
		}
	    }
	}
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
}

QPixmap KApplication::getIcon() const
{
  if( aIconPixmap.isNull()) {
      KApplication *that = const_cast<KApplication*>(this);
      that->aIconPixmap = KGlobal::iconLoader()->loadApplicationIcon( QString(name()) + ".xpm");
  }
  return aIconPixmap; 
}

QPixmap KApplication::getMiniIcon() const
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

  // Carefully shut down the process controller: It is very likely
  // that we receive a SIGCHLD while the destructor is running
  // (since we are in the process of shutting down, an opportunity
  // at which child process are being killed). So we first mark
  // the controller deleted (so that the SIGCHLD handler thinks it
  // is already gone) before we actually delete it.
  KProcessController* ctrl = theKProcessController;
  theKProcessController = 0;
  delete ctrl; // Stephan: "there can be only one" ;)

  KApp = 0;
}

bool KApplication::x11EventFilter( XEvent *_event )
{
  if ( _event->type == ClientMessage ) {
    XClientMessageEvent *cme = ( XClientMessageEvent * ) _event;

    // session management
    if( cme->message_type == WM_PROTOCOLS ) {
      if( (Atom)(cme->data.l[0]) == WM_SAVE_YOURSELF ) {
	//we want a new session config!
	if (bIsRestored && pSessionConfig) {
	  delete pSessionConfig;
	  pSessionConfig = 0;
	  bIsRestored = false;
	}
	
	
	if (!topWidget() ||
	    cme->window != topWidget()->winId()) {
	  KWM::setWmCommand(cme->window, "");
	  return true;
	}
	
	emit saveYourself(); // give applications a chance to
	// save their data
	if (bSessionManagementUserDefined)
	  KWM::setWmCommand( topWidget()->winId(), aWmCommand);
	else {
	  
	  if (pSessionConfig && !aSessionName.isEmpty()){
	    QString aCommand = name();
	    if (aCommand != argv()[0]){
	      if (argv()[0][0]=='/')
		aCommand = argv()[0];
	      else {
		char* s = new char[1024];
		aCommand=(getcwd(s, 1024));
		aCommand+="/";
		delete [] s;
		aCommand+=name();
	      }
	    }
	    aCommand+=" -restore ";
	    aCommand+=aSessionName;
	    aCommand+=aDummyString2;
	    KWM::setWmCommand( topWidget()->winId(),
			       aCommand);
	    pSessionConfig->sync();
	  } else {
	    QString aCommand = argv()[0];
	    aCommand+=aDummyString2;
	    KWM::setWmCommand( topWidget()->winId(),
			       aCommand);
	  }
	}
	
	return true;
      }
    }

    // stuff for reconfiguring
    if ( cme->message_type == KDEChangeStyle ) {
      QString str;
      
      KGlobal::config()->setGroup("KDE");
      str = KGlobal::config()->readEntry("widgetStyle");
      if(!str.isNull())
	if(str == "Motif")
	  applyGUIStyle(MotifStyle);
	else
	  if(str == "Windows 95")
	    applyGUIStyle(WindowsStyle);
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
	kdisplaySetPalette(); // This has to be first (mosfet)
	kdisplaySetStyleAndFont();
	
	return True;
      }
  }

  return false;
}

void KApplication::applyGUIStyle(GUIStyle /* newstyle */) {
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

    KConfig *pConfig = KGlobal::config();
    QString oldGroup = pConfig->group();
    pConfig->setGroup("KDE");
    QString styleStr = pConfig->readEntry("widgetStyle", "Platinum");

    if(styleHandle){
        warning("KApp: Unloading previous style plugin.");
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
                alloc_ptr = (KStyle* (*)(void))alloc_func;
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
    pConfig->setGroup(oldGroup);
}


QString KApplication::getCaption() const
{
  if( !aCaption.isNull() )
	return aCaption;
  else
	return name();
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


  // Finally, read GUI style from config.
  //	
  config->setGroup( "KDE" );
  if ( config->readEntry( "widgetStyle", "Windows 95" ) == "Windows 95" )
    applicationStyle_=WindowsStyle;
  else
    applicationStyle_=MotifStyle;
	
}



void KApplication::kdisplaySetPalette()
{
  emit kdisplayPaletteChanged();
  emit appearanceChanged();
}

void KApplication::kdisplaySetFont()
{
//     QApplication::setFont( generalFont_, true );
  // setFont() works every time for me !

  emit kdisplayFontChanged();
  emit appearanceChanged();

  resizeAll();
}	


void KApplication::kdisplaySetStyle()
{
  // QApplication::setStyle( applicationStyle );
  applyGUIStyle( applicationStyle_ );

  emit kdisplayStyleChanged();
  emit appearanceChanged();
  resizeAll();
}	


void KApplication::kdisplaySetStyleAndFont()
{
  //  QApplication::setStyle( applicationStyle );
  // 	setStyle() works pretty well but may not change the style of combo
  //	boxes.
    if ( font() != KGlobal::generalFont())
	QApplication::setFont( KGlobal::generalFont(), true );
  applyGUIStyle(applicationStyle_);

  emit kdisplayStyleChanged();
  emit kdisplayFontChanged();
  emit appearanceChanged();

  resizeAll();
}	


void KApplication::resizeAll()
{
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
         file.prepend("khelpcenterclient ");
         execl(shell, shell, "-c", file.ascii(), 0L);
	  exit( 1 );
    }
}

QString KApplication::kdedir()
{
  static QString kdedir;

  if (kdedir.isEmpty()) {
	kdedir = getenv("KDEDIR");
	if (kdedir.isEmpty()) {

#ifdef KDEDIR
	  kdedir = KDEDIR;
#else
	  kdedir = "/usr/local/kde";
#endif
	}
	if (kdedir.at(kdedir.length() -1 ) != '/')
	  kdedir += '/';
  }

  return kdedir;
}


/* maybe we could read it out of a config file, but
   this can be added later */
QString KApplication::kde_htmldir()
{
    warning("kde_htmldir() is obsolete. Try to use KStandardDirs instead");
    static QString dir;
    if (dir.isNull()) 
      dir = kdedir() + KStandardDirs::kde_default("html");
    return dir;
}

QString KApplication::kde_appsdir()
{
    warning("kde_appsdir() is obsolete. Try to use KStandardDirs instead");
    static QString dir;
    if (dir.isNull()) 
      dir = kdedir() + KStandardDirs::kde_default("apps");
    return dir;
}
                                        

QString KApplication::kde_datadir()
{
    warning("kde_datadir() is obsolete. Try to use KStandardDirs instead");
    static QString dir;
    if (dir.isNull())
      dir = kdedir() + KStandardDirs::kde_default("data");
    return dir;
}

QString KApplication::kde_bindir()
{
    warning("kde_bindir() is obsolete. Try to use KStandardDirs instead");
    static QString dir;
    if (dir.isNull())
      dir = kdedir() + KStandardDirs::kde_default("exe");
    return dir;
}

QString KApplication::kde_mimedir()
{
    warning("kde_mimedir() is obsolete. Try to use KStandardDirs instead");
    static QString dir;
     if (dir.isNull()) 
       dir = kdedir() + KStandardDirs::kde_default("mime");
    return dir;
}

QString KApplication::localkdedir()
{
  warning("localkdedir is obsolete. Try to use KStandardDirs instead");
  return ( QDir::homeDirPath() + "/.kde" );
}

bool KApplication::getKDEFonts(QStringList &fontlist) const
{
  QString fontfilename = KGlobal::dirs()->getSaveLocation("config") + "kdefonts";
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
  pTopWidget = topWidget;
  if (topWidget){
    // set the specified icons
    KWM::setIcon(topWidget->winId(), getIcon());
    KWM::setMiniIcon(topWidget->winId(), getMiniIcon());
    // set a short icon text
    // TODO: perhaps using .ascii() isn't right here as this may be seen by 
    // a user?
    XSetIconName( qt_xdisplay(), topWidget->winId(), getCaption().ascii() );
    if (bSessionManagement)
      enableSessionManagement(bSessionManagementUserDefined);

    if (!bSessionManagement)
	KWM::setWmCommand( topWidget->winId(), aWmCommand);
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


Qt::GUIStyle KApplication::applicationStyle() const
{
    return applicationStyle_;
}


#include "kapp.moc"


