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
#include <drag.h>

#include <kapp.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kdebugdialog.h>
#include <klocale.h>
#include <kiconloader.h>

#include "ltdl.h"

#include "kwm.h"

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

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

KCharsets* KApplication::pCharsets = 0L;

KApplication* KApplication::KApp = 0L;
QStrList* KApplication::pSearchPaths;
//extern bool bAreaCalculated;

static int kde_xio_errhandler( Display * )
{
  return kapp->xioErrhandler();
}

KApplication::KApplication( int& argc, char** argv ) :
  QApplication( argc, argv )
{
  QString aArgv0 = argv[0];
  int nSlashPos = aArgv0.findRev( '/' );
  if( nSlashPos != -1 )
	aAppName = aArgv0.remove( 0, nSlashPos+1 );
  else
	aAppName = aArgv0;

  init();

  parseCommandLine( argc, argv );

}


KApplication::KApplication( int& argc, char** argv, const QString& rAppName ) :
  QApplication( argc, argv )
{
  aAppName = rAppName;

  init();

  parseCommandLine( argc, argv );

}

int KApplication::xioErrhandler()
{
  emit shutDown();
  exit( 1 );
  return 0;
}

void KApplication::init()
{
  QApplication::setDesktopSettingsAware( FALSE );
  // this is important since we fork() to launch the help (Matthias)
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  // set up the fance KDE xio error handler (Matthias)
  XSetIOErrorHandler( kde_xio_errhandler );

  rootDropZone = 0L;

  // CC: install KProcess' signal handler
  // by creating the KProcController instance (if its not already existing)
  // This is handled be KProcess (stefh)
  /*
  if ( theKProcessController == 0L)
    theKProcessController = new KProcessController();
  */
  KApp = this;
  bLocaleConstructed = false; // no work around mutual dependencies

  pIconLoader = 0L;
  styleHandle = 0;

  // create the config directory ~/.kde/share/config
  QString configPath = KApplication::localkdedir();
  // We should check if  mkdir() succeeds, but since we cannot do much anyway...
  // But we'll check at least for access permissions (for SUID case)
  if ( checkAccess(configPath, W_OK) ) {
    if ( mkdir (configPath.data(), 0755) == 0) {  // make it public(?)
      chown(configPath.data(), getuid(), getgid());
      configPath += "/share";
      if ( checkAccess(configPath, W_OK) ) {
        if ( mkdir (configPath.data(), 0755) == 0 ) { // make it public
          chown(configPath.data(), getuid(), getgid());
          configPath += "/config";
          if ( checkAccess(configPath, W_OK) ) {
            if ( mkdir (configPath.data(), 0700) == 0 ) // make it private
              chown(configPath.data(), getuid(), getgid());
          }
        }
      }
    }
  }

  // try to read a global application file
  QString aGlobalAppConfigName = kde_configdir() + "/" + aAppName + "rc";
  QFile aGlobalAppConfigFile( aGlobalAppConfigName );
  // try to open read-only
  bool bSuccess = aGlobalAppConfigFile.open( IO_ReadOnly );
  if( !bSuccess )
	// there is no global app config file
	aGlobalAppConfigName = "";
  aGlobalAppConfigFile.close();


  // now for the local app config file
  QString aConfigName = KApplication::localkdedir();
  aConfigName += "/share/config/";
  aConfigName += aAppName;
  aConfigName += "rc";

  QFile aConfigFile( aConfigName );

  // We may write to the file
  if ( ! checkAccess(aConfigName.data(), W_OK ) )
    bSuccess = false;
  else {
  // Open the application-specific config file. It will be created if
  // it does not exist yet.
    bSuccess = aConfigFile.open( IO_ReadWrite );
    // Set uid/gid (neccesary for SUID programs)
    if ( bSuccess )
      chown(aConfigFile.name().ascii(), getuid(), getgid());
  }
  if( !bSuccess )
	{
	  // try to open at least read-only
	  bSuccess = aConfigFile.open( IO_ReadOnly );
	  if( !bSuccess )
		{
		  // we didn't succeed to open an app-config file
		  pConfig = new KConfig( aGlobalAppConfigName );
		  eConfigState = APPCONFIG_NONE;
		}
	  else
		{
		  // we succeeded to open an app-config file read-only
		  pConfig = new KConfig( aGlobalAppConfigName, aConfigName );
		  eConfigState = APPCONFIG_READONLY;
		}
	}
  else
	{
	  // we succeeded to open an app-config file read-write
	  pConfig = new KConfig( aGlobalAppConfigName, aConfigName );
	  eConfigState = APPCONFIG_READWRITE;
	}

  pCharsets = new KCharsets();

  pLocale = new KLocale(aAppName);
  bLocaleConstructed = true;

  // Drag 'n drop stuff taken from kfm
  display = desktop()->x11Display();
  DndSelection = XInternAtom( display, "DndSelection", False );
  DndProtocol = XInternAtom( display, "DndProtocol", False );
  DndEnterProtocol = XInternAtom( display, "DndEnterProtocol", False );
  DndLeaveProtocol = XInternAtom( display, "DndLeaveProtocol", False );
  DndRootProtocol = XInternAtom( display, "DndRootProtocol", False );
  lastEnteredDropZone = 0L;
  dropZones.setAutoDelete( FALSE );

  // initialize file search paths
  pSearchPaths = new QStrList();
  buildSearchPaths();

  WM_SAVE_YOURSELF = XInternAtom( display, "WM_SAVE_YOURSELF", False );
  WM_PROTOCOLS = XInternAtom( display, "WM_PROTOCOLS", False );
  KDEChangePalette = XInternAtom( display, "KDEChangePalette", False );
  KDEChangeGeneral = XInternAtom( display, "KDEChangeGeneral", False );
  KDEChangeStyle = XInternAtom( display, "KDEChangeStyle", False);

  readSettings();
  kdisplaySetPalette();
  kdisplaySetStyleAndFont();

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
    QWidget* w = new QWidget(0,0);
    long data = 1;
    XChangeProperty(qt_xdisplay(), w->winId(), a, a, 32,
					PropModeReplace, (unsigned char *)&data, 1);
  }
  aWmCommand = argv()[0];
}

KConfig* KApplication::getSessionConfig() {
  if (pSessionConfig)
    return pSessionConfig;
  // create a instance specific config object
  QString aConfigName = KApplication::localkdedir();
  aConfigName += "/share/config/";
  aConfigName += aAppName;
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
    chown(aConfigFile.name(), getuid(), getgid());
    aConfigFile.close();
    pSessionConfig = new KConfig(QString::null, aSessionConfigName);
    aSessionName = aAppName.copy();
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




KIconLoader* KApplication::getIconLoader()
{
  if( !pIconLoader )
    pIconLoader = new KIconLoader();

  return pIconLoader;
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

  id = pMenu->insertItem( QString(i18n( "&About" )) + " " + aAppName + "...");
  if( aboutAppText )
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
  invokeHTMLHelp( aAppName + "/" + "index.html", "" );
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
  //  QMessageBox::aboutQt( NULL, getCaption() );
}


KLocale* KApplication::getLocale()
{
  if( !pLocale )
    pLocale = new KLocale();

  return pLocale;
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
		  KConfig* pConfig = getConfig();
		  QString aOldGroup = pConfig->getGroup();
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

		  return TRUE; // do not process event further
		}
	}
  return FALSE; // process event further
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
        aIconPixmap = getIconLoader()->loadApplicationIcon( argv[i+1] );
      if (aMiniIconPixmap.isNull()){
		if (argv[i+1][0] == '/')
		  aMiniIconPixmap = aIconPixmap;
		else
		  aMiniIconPixmap = getIconLoader()->loadApplicationMiniIcon( argv[i+1] );
      }
      aDummyString2 += parameter_strings[icon-1];
      aDummyString2 += " ";
      aDummyString2 += argv[i+1];
      aDummyString2 += " ";
      break;
    case miniicon:
      if (argv[i+1][0] == '/')
        aMiniIconPixmap = QPixmap(argv[i+1]);
      else
        aMiniIconPixmap = getIconLoader()->loadApplicationMiniIcon( argv[i+1] );
      aDummyString2 += parameter_strings[miniicon-1];
      aDummyString2 += " ";
      aDummyString2 += argv[i+1];
      aDummyString2 += " ";
      break;
    case restore:
      {
		aSessionName = argv[i+1];
		QString aSessionConfigName;
		if (argv[i+1][0] == '/')
		  aSessionConfigName = argv[i+1];
		else {
		  aSessionConfigName = KApplication::localkdedir();
		  aSessionConfigName += "/share/config/";
		  aSessionConfigName += argv[i+1];
		}
		if (QFile::exists(aSessionConfigName)){
		  QFile aConfigFile(aSessionConfigName);
		  bool bSuccess;
		  if ( ! checkAccess( aConfigFile.name(), W_OK ) )
		    bSuccess = false;
		  else
		    bSuccess = aConfigFile.open( IO_ReadWrite );
		  if( bSuccess ){
                        // Set uid/gid (neccesary for SUID programs)
                        chown(aConfigFile.name(), getuid(), getgid());

			aConfigFile.close();
			pSessionConfig = new KConfig(QString::null, aSessionConfigName);
			
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

  if (aIconPixmap.isNull()){
    aIconPixmap = getIconLoader()->loadApplicationIcon( aAppName + ".xpm");
  }
  if (aMiniIconPixmap.isNull()){
    aMiniIconPixmap = getIconLoader()->loadApplicationMiniIcon( aAppName + ".xpm");
  }

}

KApplication::~KApplication()
{
  removeEventFilter( this );

  if( pIconLoader )
    delete pIconLoader;

  if( pLocale )
    delete pLocale;

  delete pCharsets;

  delete pSearchPaths;

  delete pConfig;

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
  // You can get root drop events twice.
  // This is to avoid this.
  static int rootDropEventID = -1;

  if ( _event->type == ClientMessage )
    {
	  XClientMessageEvent *cme = ( XClientMessageEvent * ) _event;
	  // session management
	  if( cme->message_type == WM_PROTOCOLS )
		{
		  if( (Atom)(cme->data.l[0]) == WM_SAVE_YOURSELF )
			{
			    //we want a new session config!
			    if (bIsRestored && pSessionConfig) {
				delete pSessionConfig;
				pSessionConfig = 0;
				bIsRestored = false;
			    }
				
			
			  if (!topWidget() ||
			      cme->window != topWidget()->winId()){
			    KWM::setWmCommand(cme->window, "");
			    return true;
			  }
			
			  emit saveYourself(); // give applications a chance to
			  // save their data
			  if (bSessionManagementUserDefined)
			    KWM::setWmCommand( topWidget()->winId(), aWmCommand);
			  else {
			
			    if (pSessionConfig && !aSessionName.isEmpty()){
			      QString aCommand = aAppName.copy();
			      if (aAppName != argv()[0]){
					if (argv()[0][0]=='/')
					  aCommand = argv()[0];
					else {
					  char* s = new char[1024];
					  aCommand=(getcwd(s, 1024));
					  aCommand+="/";
					  delete [] s;
					  aCommand+=aAppName;
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
	  if ( cme->message_type == KDEChangeStyle )
		{
		  QString str;
		
		  getConfig()->setGroup("KDE");
		  str = getConfig()->readEntry("widgetStyle");
		  if(!str.isNull())
		    if(str == "Motif")
		      applyGUIStyle(MotifStyle);
		    else
		      if(str == "Windows 95")
			applyGUIStyle(WindowsStyle);
		  return TRUE;
		}

	  if ( cme->message_type == KDEChangePalette )
		{
		  readSettings();
		  kdisplaySetPalette();
		
		  return True;
		}
	  if ( cme->message_type == KDEChangeGeneral )
		{
		  readSettings();
		  kdisplaySetPalette(); // This has to be first (mosfet)
		  kdisplaySetStyleAndFont();
		
		  return True;
		}
	
	  if ( cme->message_type == DndLeaveProtocol )
		{
		  if ( lastEnteredDropZone != 0L )
			lastEnteredDropZone->leave();
	
		  lastEnteredDropZone = 0L;

		  return TRUE;
		}
	  else if ( cme->message_type != DndProtocol && cme->message_type != DndEnterProtocol &&
				cme->message_type != DndRootProtocol )
	    return FALSE;
	
	  Window root = DefaultRootWindow(display);
	
	  unsigned char *Data;
	  unsigned long Size;
	  Atom    ActualType;
	  int     ActualFormat;
	  unsigned long RemainingBytes;

	  XGetWindowProperty(display,root,DndSelection,
						 0L,1000000L,
						 FALSE,AnyPropertyType,
						 &ActualType,&ActualFormat,
						 &Size,&RemainingBytes,
						 &Data);

	  QPoint p( (int)cme->data.l[3], (int)cme->data.l[4] );

	  if ( cme->message_type == DndRootProtocol )
		{
		  if ( rootDropEventID == (int)cme->data.l[1] )
			return FALSE;
	
		  rootDropEventID = (int)cme->data.l[1];

		  if ( rootDropZone != 0L )
			rootDropZone->drop( (char*)Data, Size, (int)cme->data.l[0], p.x(), p.y() );
		  return TRUE;
		}
	
	  KDNDDropZone *dz;
	  KDNDDropZone *result = 0L;
	
	  /*
		for ( dz = dropZones.first(); dz != 0L; dz = dropZones.next() )
		{
		QPoint p2 = dz->getWidget()->mapFromGlobal( p );
		if ( dz->getWidget()->rect().contains( p2 ) )
		result = dz;
		}
	  */

	  QWidget *w = widgetAt( p.x(), p.y(), TRUE );

	  while ( result == 0L && w != 0L )
		{
	      for ( dz = dropZones.first(); dz != 0L; dz = dropZones.next() )
			{
			  if ( dz->getWidget() == w )
				result = dz;
			}
	
	      if ( result == 0L )
			w = w->parentWidget();
		}

	  // KFM hack. Find not decorated windows ( root icons )
	  if ( result == 0L )
		for ( dz = dropZones.first(); dz != 0L; dz = dropZones.next() )
	      {
			QPoint p2 = dz->getWidget()->mapFromGlobal( p );
			if ( dz->getWidget()->rect().contains( p2 ) )
		      result = dz;
	      }
	
	  if ( result != 0L )
		{
	      if ( cme->message_type == DndProtocol )
			{
			  result->drop( (char*)Data, Size, (int)cme->data.l[0], p.x(), p.y() );
			}
	      else if ( cme->message_type == DndEnterProtocol )
			{
			  // If we entered another drop zone, tell the drop zone we left about it
			  if ( lastEnteredDropZone != 0L && lastEnteredDropZone != result )
				lastEnteredDropZone->leave();
		
			  // Notify the drop zone over which the pointer is right now.
			  result->enter( (char*)Data, Size, (int)cme->data.l[0], p.x(), p.y() );
			  lastEnteredDropZone = result;
			}
		}
	  else
		{
		  // Notify the last DropZone that the pointer has left the drop zone.
		  if ( lastEnteredDropZone != 0L )
			lastEnteredDropZone->leave();
		  lastEnteredDropZone = 0L;
		}

	  return TRUE;
    }

  return FALSE;
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

    QString oldGroup = pConfig->group();
    pConfig->setGroup("KDE");
    QString styleStr = pConfig->readEntry("widgetStyle", "Platinum");

    if(styleHandle){
        warning(i18n("KApp: Unloading previous style plugin."));
        lt_dlclose((lt_dlhandle*)styleHandle);
        styleHandle = 0;
    }

    if(styleStr == "Platinum"){
        pKStyle=NULL;
        setStyle(new QPlatinumStyle);
    }
    else if(styleStr == "Windows 95"){
        pKStyle=NULL;
        setStyle(new QWindowsStyle);
    }
    else if(styleStr == "CDE"){
        pKStyle=NULL;
        setStyle(new QCDEStyle);
    }
    else if(styleStr == "Motif"){
        pKStyle=NULL;
        setStyle(new QMotifStyle);
    }
    else{
        if(!dlregistered){
            dlregistered = true;
            lt_dlinit();
            lt_dladdsearchdir(localkdedir() + "/share/apps/kstyle/modules");
            lt_dladdsearchdir(kde_bindir() + "/../lib");
        }

        QDir dir(localkdedir() + "/share/apps/kstyle/modules/", styleStr);
        if(!dir.count()){
            dir.setPath(kde_bindir() + "/../lib/");
            if(!dir.count()){
                warning(i18n("KApp: Unable to find style plugin %s."),
                        (const char *)styleStr);
                pKStyle = NULL;
                setStyle(new QPlatinumStyle);
                return;
            }
        }
        styleStr = dir.path() + "/" + styleStr;
        styleHandle = lt_dlopen(styleStr);

        if(!styleHandle){
            warning(i18n("KApp: Unable to open style plugin %s (%s)."),
                    (const char *)styleStr, lt_dlerror());
            pKStyle = NULL;
            setStyle(new QPlatinumStyle);
        }
        else{
            lt_ptr_t alloc_func = lt_dlsym(styleHandle,
                                           "allocate");
            if(!alloc_func){
                warning(i18n("KApp: Unable to init style plugin %s (%s)."),
                    (const char *)styleStr, lt_dlerror());
                pKStyle = NULL;
                lt_dlclose(styleHandle);
                styleHandle = 0;
                setStyle(new QPlatinumStyle);
            }
            else{
                KStyle* (*alloc_ptr)();
                alloc_ptr = (KStyle* (*)(void))alloc_func;
                pKStyle = alloc_ptr();
                if(pKStyle){
                    warning(i18n("KApp: Style plugin successfully allocated."));
                    setStyle(pKStyle);
                }
                else{
                    warning(i18n("KApp: Style plugin unable to allocate style."));
                    pKStyle = NULL;
                    setStyle(new QPlatinumStyle);
                    lt_dlclose(styleHandle);
                    styleHandle = 0;
                }
            }
        }
    }
    pConfig->setGroup(oldGroup);
}


QString KApplication::findFile( const QString& file )
{
  QString fullPath;
  QStrListIterator it( *pSearchPaths );

  while ( it.current() )
    {
	  fullPath = it.current();
	  fullPath += '/';
	  fullPath += file;
	  if ( !access( fullPath, 0 ) )
		return fullPath;
	  ++it;
    }

  return QString::null;
}


const QString KApplication::getCaption() const
{
  if( !aCaption.isNull() )
	return aCaption;
  else
	return aAppName;
}


void KApplication::buildSearchPaths()
{
  // Torben
  // We want to search the local files with highest priority
  QString tmp = KApplication::localkdedir();
  appendSearchPath( tmp );

  // add paths from "[KDE Setup]:Path=" config file entry
  getConfig()->setGroup( "KDE Setup" );
  QString kdePathRc = getConfig()->readEntry( "Path" );

  if ( !kdePathRc.isNull() )
    {
      char *start, *end, *workPath = new char [ kdePathRc.length() + 1 ];
	  strcpy( workPath, kdePathRc );
	  start = workPath;
	  while ( start )
		{
	  	  end = strchr( start, ':' );
		  if ( end )
		    *end = '\0';
		  appendSearchPath( start );
		  start = end ? end + 1 : end;
		}
	  delete [] workPath;
    }

  // add paths in the KDEPATH environment variable
  QString kdePathEnv = getenv( "KDEPATH" );
  if ( !kdePathEnv.isEmpty() )
    {
	  char *start, *end, *workPath = new char [ strlen( kdePathEnv ) + 1 ];
	  strcpy( workPath, kdePathEnv );
	  start = workPath;
	  while ( start )
		{
	  	  end = strchr( start, ':' );
		  if ( end )
		    *end = '\0';
		  appendSearchPath( start );
		  start = end ? end + 1 : end;
		}
	  delete [] workPath;
    }

  appendSearchPath( kdedir().data() );
}

void KApplication::appendSearchPath( const QString& path )
{
  QStrListIterator it( *pSearchPaths );

  // return if this path has already been added
  while ( it.current() )
    {
	  if ( !strcmp( it.current(), path ) )
		return;
	  ++it;
    }

  pSearchPaths->append( path );
}

void KApplication::readSettings()
{
  // use the global config files
  KConfig* config = getConfig();
  config->reparseConfiguration();

  QString str;
	
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
		
  pCharsets->setDefault(klocale->charset());
  generalFont_ = QFont("helvetica", 12, QFont::Normal);
  pCharsets->setQFont(generalFont_);
  fixedFont_ = QFont("fixed", 12, QFont::Normal);
  pCharsets->setQFont(fixedFont_);

  config->setGroup( "General" );
  generalFont_ = config->readFontEntry( "font", &generalFont_ );
  fixedFont_ = config->readFontEntry( "fixedFont", &fixedFont_ );

  // Finally, read GUI style from config.
	
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

    /*
  // WARNING : QApplication::setPalette() produces inconsistent results.
  // There are 3 problems :-
  // 1) You can't change select colors
  // 2) You need different palettes to apply the same color scheme to
  //		different widgets !!
  // 3) Motif style needs a different palette to Windows style.
	
  int highlightVal, lowlightVal;
	
  highlightVal=100+(2*contrast+4)*16/10;
  lowlightVal=100+(2*contrast+4)*10;
	
  // printf("contrast = %d\n", contrast);
	
  if ( applicationStyle==MotifStyle ) {
	QColorGroup disabledgrp( textColor, backgroundColor,
							 backgroundColor.light(highlightVal),
							 backgroundColor.dark(lowlightVal),
							 backgroundColor.dark(120),
							 backgroundColor.dark(120), windowColor );

	QColorGroup colgrp( textColor, backgroundColor,
						backgroundColor.light(highlightVal),
						backgroundColor.dark(lowlightVal),
						backgroundColor.dark(120),
						textColor, windowColor );

	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );

	emit kdisplayPaletteChanged();
	emit appearanceChanged();

  } else {
	QColorGroup disabledgrp( textColor, backgroundColor,
							 backgroundColor.light(150),
							 backgroundColor.dark(),
							 backgroundColor.dark(120),
							 backgroundColor.dark(120), windowColor );

	QColorGroup colgrp( textColor, backgroundColor,
						backgroundColor.light(150),
						backgroundColor.dark(),
						backgroundColor.dark(120),
						textColor, windowColor );

	QApplication::setWinStyleHighlightColor( selectColor );
	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );

	emit kdisplayPaletteChanged();
	emit appearanceChanged();

  }
    */
}

void KApplication::kdisplaySetFont()
{
//     QApplication::setFont( generalFont_, TRUE );
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
    if ( *font() != generalFont_)
	QApplication::setFont( generalFont_, TRUE );
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
	    filename = aAppName + "/index.html";

	 QString path = KApplication::kde_htmldir().copy() + "/";

         // first try the locale setting
         QString file = path + klocale->language() + '/' + filename;
         if( !QFileInfo( file ).exists() )
               // not found: use the default
               file = path + "default/" + filename;

	  if( !topic.isEmpty() )
		{
                 file.append( "#" );
                 file.append(topic);
		}
	
	  /* Since this is a library, we must conside the possibilty that
	   * we are being used by a suid root program. These next two
	   * lines drop all privileges.
	   */
	  setuid( getuid() );
	  setgid( getgid() );
	  char* shell = "/bin/sh";
	  if (getenv("SHELL"))
		shell = getenv("SHELL");
         file.prepend("kdehelp ");
         execl(shell, shell, "-c", file.data(), 0L);
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
  }

  return kdedir;
}


/* maybe we could read it out of a config file, but
   this can be added later */
const QString KApplication::kde_htmldir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_HTMLDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_appsdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_APPSDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_icondir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_ICONDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_datadir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_DATADIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_localedir()
{
  static QString dir;
  if (dir.isNull()) {
	dir = KDE_LOCALE;
	if (!strncmp(dir.data(), "KDEDIR", 6))
	    dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_cgidir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_CGIDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_sounddir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_SOUNDDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_toolbardir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_TOOLBARDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_wallpaperdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_WALLPAPERDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_bindir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_BINDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_partsdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_PARTSDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_configdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_CONFIGDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString KApplication::kde_mimedir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_MIMEDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}


QString KApplication::localkdedir()
{
  return ( QDir::homeDirPath() + "/.kde" );
}


QString KApplication::localconfigdir()
{
  return ( localkdedir() + "/share/config" );
}


bool KApplication::getKDEFonts(QStrList *fontlist)
{
  QString fontfilename;

  if(fontlist == 0L)
    return false;

  fontfilename = KApplication::localconfigdir();

  if(fontfilename.isEmpty()){
    return false;
  }

  fontfilename = fontfilename + "/kdefonts";

  QFile fontfile(fontfilename);

  if (!fontfile.exists())
    return false;

  if(!fontfile.open(IO_ReadOnly)){
    return false;
  }

  if (!fontfile.isReadable())
    return false;

  QTextStream t(&fontfile);


  while ( !t.eof() ) {
    QString s = t.readLine();
    if(!s.isEmpty())
      fontlist->append( s );
  }

  fontfile.close();

  return true;
}


const QString KApplication::tempSaveName( const QString& pFilename )
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

  return qstrdup( aFilename.data() );
}


const QString KApplication::checkRecoverFile( const QString& pFilename,
        bool& bRecover )
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
	  return qstrdup( aFilename.data() );
	}
  else
	{
	  bRecover = false;
	  return qstrdup( pFilename );
	}
}


bool checkAccess(const QString& pathname, int mode)
{
  int accessOK = access( pathname, mode );
  if ( accessOK == 0 )
    return true;  // OK, I can really access the file

  // else
  // if we want to write the file would be created. Check, if the
  // user may write to the directory to create the file.
  if ( mode & W_OK == 0 )
    return false;   // Check for write access is not part of mode => bail out


  //strip the filename (everything until '/' from the end
  QString dirName(pathname);
  int pos = dirName.findRev('/');
  if ( pos == -1 )
    return false;   // No path in argument. This is evil, we won't allow this

  dirName.truncate(pos); // strip everything starting from the last '/'

  accessOK = access( dirName, W_OK );
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
    XSetIconName( qt_xdisplay(), topWidget->winId(), getCaption() );
    if (bSessionManagement)
      enableSessionManagement(bSessionManagementUserDefined);

    if (!bSessionManagement)
	KWM::setWmCommand( topWidget->winId(), aWmCommand);
  }
}

void KApplication::registerTopWidget()
{
}

void KApplication::unregisterTopWidget()
{
}


QColor KApplication::inactiveTitleColor()
{
    return inactiveTitleColor_;
}


QColor KApplication::inactiveTextColor()
{
    return inactiveTextColor_;
}


QColor KApplication::activeTitleColor()
{
    return activeTitleColor_;
}


QColor KApplication::activeTextColor()
{
    return activeTextColor_;
}

int KApplication::contrast()
{
    return contrast_;
}

QFont KApplication::generalFont()
{
    return generalFont_;
}

QFont KApplication::fixedFont()
{
    return fixedFont_;
}


Qt::GUIStyle KApplication::applicationStyle()
{
    return applicationStyle_;
}



#include "kapp.moc"


