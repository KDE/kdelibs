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
// Revision 1.87  1998/01/27 20:17:01  kulow
// $Log$
// Revision 1.81  1998/01/03 15:24:33  kulow
// added accelerators and ... to the returned Help - Menu
//
// Revision 1.80  1997/12/31 14:12:25  kulow
// corrected grammar and added one more .copy()
//
// Revision 1.79  1997/12/29 14:36:46  kulow
// kdedir() does no longer return a deep copy, but a shallow copy.
// I inserted a test against a deep copy to take care of changes. But this
// is just to find problems and can be removed by undefining TEST_KDEDIR in
// Makefile.am
//
// Revision 1.78  1997/12/28 21:32:14  kulow
// last time I forgot two functions kde_mimedir() and kde_confdir()
// I will move this functions very soon to static functions of a new
// class KPaths. Kapplication is not the optimal class to contain this
// functions, since I need a DISPLAY to find out the paths. But I think,
// we can create some inline methods in kapp then
//
// Revision 1.77  1997/12/13 15:08:56  jacek
// KCharsets support added
// WARNING: kderc file format has changed. Since now charset name
// instead of id is stored.
//
//
// Revision 1.76  1997/11/27 20:21:20  kulow
// safer deletion of theKProcessController by Johannes Sixt.
// Read the comments for details
//
// Revision 1.75  1997/11/21 11:22:29  kulow
// make a deep copy of kdedir before returning it. I still don't get it,
// why this is necessary.
//
// Revision 1.74  1997/11/20 22:41:32  kalle
// Small bug fix in kapp.cpp (I hope this does not break anything...)
// First element of search was $HOME./kde and is now $HOME/.kde
// New version looks way more correct to me...
//
// Revision 1.73  1997/11/20 22:24:30  kalle
// new static methods for the various directories
//
// Revision 1.72  1997/11/18 21:40:41  kalle
// KApplication::localconfigdir()
// KApplication::localkdedir()
// KConfig searches in $KDEDIR/share/config/kderc
//
// Revision 1.71  1997/11/11 05:41:53  wuebben
// Bernd: implemented iso charset support
//
// Revision 1.70  1997/11/09 04:08:54  wuebben
// Bernd: iso charset changes (minimal and harmless )
//
// Revision 1.69  1997/11/03 14:41:28  ettrich
// Matthias: tiny fix for command line arguments: "-icon" will also be used
//       for the miniicon if "-miniicon" is not defined.
//
// Revision 1.68  1997/11/03 13:50:15  kulow
// delete theKProcessController in the destructor
//
// Revision 1.67  1997/10/30 13:30:15  ettrich
// Matthias: fix for setWmCommand: now setWmCommand can also be used for
//   PseudoSessionManagement (this is the default when session management
//   was not enabled with enableSessionManagement).
//   Now KApplication should work as promised in kapp.h :-)
//
// Revision 1.66  1997/10/25 22:27:40  kalle
// Fixed bug with default help menu (Thanks, Bernd! This one was just in time!)
//
// Revision 1.65  1997/10/24 16:58:55  ettrich
// Matthias: bugfixes for session management.
//
// Revision 1.64  1997/10/22 20:42:52  kalle
// Help menu works as advertised
//
// Revision 1.63  1997/10/21 20:44:41  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.62  1997/10/17 15:46:22  stefan
// Moved the include of stdlib.h -- otherwise gcc-2.7.2.1 has an internal
// compiler bug. I know - I should upgrade gcc, but for all the
// poor folks that still have the old one ;-)
//
// Revision 1.61  1997/10/17 13:30:16  ettrich
// Matthias: registerTopWidget/unregisterTopWidget are obsolete and empty now.
//           Introduced new registration model
//
// Revision 1.60  1997/10/16 11:35:24  kulow
// I'm not sure, why this have been removed, but I'm sure, they are
// needed.
//
// Revision 1.59  1997/10/16 11:14:27  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.57  1997/10/14 13:31:57  kulow
// removed one more default value from the implementation
//
// Revision 1.56  1997/10/13 14:31:03  ettrich
// Matthias: fixed the caption bug (AGAIN THESE WEIRD SHALLOW QSTRING COPIES...)
//
// Revision 1.55  1997/10/13 11:00:04  ettrich
// Matthias: fix to generation of SM command
//
// Revision 1.54  1997/10/12 14:36:34  kalle
// KApplication::tempSaveName() and KApplication::checkRecoverFile()
// now implemented as documented.
// The change in kapp.h is only in the documentation and thus *-compatible.
//
// Revision 1.53  1997/10/11 22:39:27  ettrich
// Matthias: BINARY INCOMPATIBLE CHANGES
//     - removed senseless method setUnsavedData
//
// Revision 1.52  1997/10/11 19:25:32  ettrich
// Matthias: mainWidget -> topWidget for SM
// Matthias: fixed an async reply problem with invokeHTMLHelp
//
// Revision 1.50  1997/10/10 22:09:17  ettrich
// Matthias: BINARY INCOMPATIBLE CHANGES: extended session management support

#include <qdir.h> // must be at the front

#include "kapp.moc"

#include <kapp.h>
#include "kkeyconf.h"
#include <kiconloader.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kdebug.h>
#include "kwm.h"
#include <kdebugdialog.h>

#include <unistd.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <qobjcoll.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qmsgbox.h>
#include <qtstream.h>
#include <qregexp.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h> // getenv()
#include <signal.h>


#include <qwidcoll.h>

#include "kprocctrl.h"

KCharsets* KApplication::pCharsets = 0L;

KApplication* KApplication::KApp = 0L;
QStrList* KApplication::pSearchPaths;
extern bool bAreaCalculated;

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

void KApplication::init()
{
  // this is important since we fork() to launch the help (Matthias)
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);

  rootDropZone = 0L;

  // CC: install KProcess' signal handler
  // by creating the KProcController instance (if its not already existing)
  if ( theKProcessController == 0L) 
    theKProcessController = new KProcessController();

  KApp = this;
  bLocaleConstructed = false; // no work around mutual dependencies
  
  pIconLoader = 0L;

  // create the config directory
  QString configPath = QDir::homeDirPath () + "/.kde";
  // We should check if  mkdir() succeeds, but since we cannot do much anyway...
  mkdir (configPath.data(), 0755); // make it public(?)
  configPath += "/share";
  mkdir (configPath.data(), 0755); // make it public
  configPath += "/config";
  mkdir (configPath.data(), 0700); // make it private    

  // try to read a global application file
  QString aGlobalAppConfigName = kdedir() + "/share/config/" + aAppName + "rc";
  QFile aGlobalAppConfigFile( aGlobalAppConfigName );
  // try to open read-only
  bool bSuccess = aGlobalAppConfigFile.open( IO_ReadOnly );
  if( !bSuccess )
	// there is no global app config file
	aGlobalAppConfigName = "";
  aGlobalAppConfigFile.close();

  QString aConfigName;
  char* pHome;
  if( (pHome = getenv( "HOME" )) )
	aConfigName = pHome;
  else
	aConfigName = "."; // use current dir if $HOME is not set
  aConfigName += "/.kde/share/config/";
  aConfigName += aAppName;
  aConfigName += "rc";

  QFile aConfigFile( aConfigName );

  // Open the application-specific config file. It will be created if
  // it does not exist yet.
  
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
		  pConfig = new KConfig( aGlobalAppConfigName, aConfigName );
		    PropModeReplace, (unsigned char *)&data, 1);
  
  pCharsets = new KCharsets();
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

  // initialize KKeyConfig
  initKKeyConfig( getConfig() );  

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
  QString aConfigName;
  char* pHome;
  if( (pHome = getenv( "HOME" )) )
    aConfigName = pHome;
  else
    aConfigName = "."; // use current dir if $HOME is not set
  aConfigName += "/.kde/share/config/";
  aConfigName += aAppName;
  aConfigName += "rc";

  QString aSessionConfigName;
  invokeHTMLHelp( aAppName, "" );
  int i = 0;
  do {
  if( bAboutQtMenu )
    num.setNum(i);
	  id = pMenu->insertItem( klocale->translate( "About Qt" ) );
	  pMenu->connectItem( id, this, SLOT( aboutQt() ) );
  QFile aConfigFile(aSessionConfigName);
	*/
  if( bSuccess ){
    aConfigFile.close();
    pSessionConfig = new KConfig(0L, aSessionConfigName);
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
					  "The KDE Desktop Environment was written by "
					  "the KDE team.\n\n"
					  "Please send bug reports to kde-bugs@kde.org.\n\n\n"
					  "KDE was developed with Qt, a cross-platform GUI library.\n\n"
					  "Qt is a product of Troll Tech (http://www.troll.no, info@troll.no).\n" 
					  "Qt may be used freely to develop free software on the X Window System.\n"
					  )
    KWM::setWmCommand( topWidget()->winId(), aWmCommand);
}

  id = pMenu->insertItem( klocale->translate( "About " ) + aAppName );


KIconLoader* KApplication::getIconLoader()
  id = pMenu->insertItem( klocale->translate( "About KDE" ) );
  if( !pIconLoader )
    pIconLoader = new KIconLoader();

  return pIconLoader;
}


QPopupMenu* KApplication::getHelpMenu( bool /*bAboutQtMenu*/, 
									   const char* aboutAppText )
{
  QPopupMenu* pMenu = new QPopupMenu();

  int id = pMenu->insertItem( klocale->translate( "&Help..." ) );
  pMenu->connectItem( id, this, SLOT( appHelpActivated() ) );

  pMenu->insertSeparator();

  id = pMenu->insertItem( QString(klocale->translate( "&About" )) + " " + aAppName + "...");
  pMenu->connectItem( id, this, SLOT( aboutApp() ) );
  aAppAboutString = aboutAppText;

  id = pMenu->insertItem( klocale->translate( "About &KDE..." ) );
  pMenu->connectItem( id, this, SLOT( aboutKDE() ) );
  /*
	if( bAboutQtMenu )
	{
	id = pMenu->insertItem( klocale->translate( "About Qt" ) );
										 "Please send bug reports to kde-bugs@kde.org.\n\n\n"
	}
  */
										 "Please report bug at http://buglist.kde.org.\n\n\n"
}


void KApplication::appHelpActivated()
{
  invokeHTMLHelp( aAppName + "/" + "index.html", "" );
}


void KApplication::aboutKDE()
{
  QMessageBox::about( NULL, klocale->translate( "About KDE" ),
					  klocale->translate( 
										 "The KDE Desktop Environment was written by "
										 "the KDE team.\n\n"
										 "Please report bugs at http://buglist.kde.org.\n\n\n"
										 "KDE was developed with Qt, a cross-platform GUI library.\n\n"
										 "Qt is a product of Troll Tech (http://www.troll.no, info@troll.no).\n" 
										 "Qt may be used freely to develop free software on the X Window System.\n"
										 )
					  );
}

void KApplication::aboutApp()
{
  QMessageBox::about( NULL, getCaption(), aAppAboutString );
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
  int i = 0;
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
		  pDialog->setErrorShow( pConfig->readEntry( "ErrorShow", "" ) );
		  pDialog->setFatalOutput( pConfig->readNumEntry( "FatalOutput", 4 ) );
		  pDialog->setFatalFile( pConfig->readEntry( "FatalFilename", 
													 "kdebug.dbg" ) );
		  pDialog->setFatalShow( pConfig->readEntry( "FatalShow", "" ) );
		  pDialog->setAbortFatal( pConfig->readNumEntry( "AbortFatal", 0 ) );
			  pConfig->writeEntry( "WarnOutput", pDialog->warnOutput() );
			  pConfig->writeEntry( "WarnFilename", pDialog->warnFile() );
			  pConfig->writeEntry( "WarnShow", pDialog->warnShow() );
			  pConfig->writeEntry( "ErrorOutput", pDialog->errorOutput() );
			  pConfig->writeEntry( "ErrorFilename", pDialog->errorFile() );
			  pConfig->writeEntry( "ErrorShow", pDialog->errorShow() );
  int parameter_count = 4;

			  bAreaCalculated = false;
			}
		  else
			{
			  /* User pressed Cancel, do nothing */

    for ( int p = 0 ; p < parameter_count; p++)
		  /* restore old group */
        parameter = (parameter_code)(p + 1);

		  return TRUE; // do not process event further
		}
	}
  return FALSE; // process event further
}

	aSessionName = argv[i+1];
	QString aSessionConfigName;
	if (argv[i+1][0] == '/')
	  aSessionConfigName = argv[i+1];
	else {
	  char* pHome;
	  if( (pHome = getenv( "HOME" )) )
	    aSessionConfigName = pHome;
	  else
	    aSessionConfigName = "."; // use current dir if $HOME is not set
	  aSessionConfigName += argv[i+1];
	}
	if (QFile::exists(aSessionConfigName)){
	  QFile aConfigFile(aSessionConfigName);
	  bool bSuccess = aConfigFile.open( IO_ReadWrite ); 
	  if( bSuccess ){
	    aConfigFile.close();
	    pSessionConfig = new KConfig(0L, aSessionConfigName);
	    if (pSessionConfig){
	      bIsRestored = True;
	    }
	  }
	}
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
		  char* pHome;
		  if( (pHome = getenv( "HOME" )) )
			aSessionConfigName = pHome;
		  else
			aSessionConfigName = "."; // use current dir if $HOME is not set
		  aSessionConfigName += "/.kde/share/config/";
		  aSessionConfigName += argv[i+1];
		}
		if (QFile::exists(aSessionConfigName)){
		  bool bSuccess = aConfigFile.open( IO_ReadWrite ); 
		  if( bSuccess ){
			aConfigFile.close();
  delete theKProcessController; // Stephan: "the can only be one" ;)
  theKProcessController = 0L;

    if ( parameter != unknown ) { // remove arguments
      
      for( int j = i;  j < argc-2; j++ )
        argv[j] = argv[j+2];
      
      argc -=2 ;
    }
				  aCommand+=(getcwd(s, 1024));
  }

			                     // save their data 
    aIconPixmap = getIconLoader()->loadApplicationIcon( aAppName + ".xpm");
  }
  if (aMiniIconPixmap.isNull()){
    aMiniIconPixmap = getIconLoader()->loadApplicationMiniIcon( aAppName + ".xpm");

}
				if (argv()[0][0]=='/')
				  aCommand = argv()[0];
				  aCommand=(getcwd(s, 1024));
						 argv()[0]);
				  delete [] s;
				  aCommand+=aAppName;
				}
    
  if( pLocale )
    delete pLocale;

  delete pCharsets;
						 aCommand);
  delete pSearchPaths;

  delete pConfig;

  // Carefully shut down the process controller: It is very likely
						 aCommand);
  // (since we are in the process of shutting down, an opportunity
  // at which child process are being killed). So we first mark
  // the controller deleted (so that the SIGCHLD handler thinks it
  // is already gone) before we actually delete it.
  KProcessController* ctrl = theKProcessController;
  theKProcessController = 0;
  delete ctrl; // Stephan: "there can be only one" ;)
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
			if(str == "Motif")
			  applyGUIStyle(MotifStyle);
			else
			  if(str == "Windows 95")
				applyGUIStyle(WindowsStyle);
		  QString str;
		  
		  getConfig()->setGroup("GUI Style");
		  str = getConfig()->readEntry("Style");
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
			  
		  kdisplaySetPalette();
		  
		  return True;
		}
	  if ( cme->message_type == KDEChangeGeneral )
		{
		  readSettings();
		  kdisplaySetStyleAndFont();
		  kdisplaySetPalette();
		  
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
		  return fullPath;
		  
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

	  return TRUE;
    }
    
  return FALSE;
}

void KApplication::applyGUIStyle(GUIStyle newstyle) {
  QApplication::setStyle( applicationStyle );
    // Torben
    // We want to search the local files with highest priority
    QString tmp( getenv( "HOME" ) );
    tmp += "./kde";
    appendSearchPath( tmp );
  // foreach toplevel ...
  tmp += "./kde";
    QWidget *w = wl_it.current();

    // set new style
    w->setStyle(newstyle);
    QObjectList *ol = w->queryList("QWidget", 0, 0, TRUE);
    QObjectListIt ol_it( *ol );

    // set style to child widgets
    while ( ol_it.current() ) {
      QWidget *child = (QWidget *)(ol_it.current());
      child->setStyle(newstyle);
      ++ol_it;
    }
    delete ol;
    ++wl_it;
  }

  delete wl;
}

QString KApplication::findFile( const char *file )
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

  fullPath.resize( 0 );

 appendSearchPath( kdedir().data() );
}


const char* KApplication::getCaption() const
{
  if( !aCaption.isNull() )
	return aCaption;
  else
	return aAppName;
}
		  return;

void KApplication::buildSearchPaths()
  // Torben
  // We want to search the local files with highest priority
        int num;
  QString tmp( getenv( "HOME" ) );
  tmp += "/.kde";
  appendSearchPath( tmp );
	// use the global config files
        KConfig* config = getConfig();
        config->reparseConfiguration();
  QString kdePathRc = getConfig()->readEntry( "Path" );
	QString str;
  if ( !kdePathRc.isNull() )
    {
      char *start, *end, *workPath = new char [ kdePathRc.length() + 1 ];
	  strcpy( workPath, kdePathRc );
	config->setGroup( "Color Scheme");
	// this default is the KDE standard grey
	str = config->readEntry( "InactiveTitleBarColor", "#C0C0C0" );
	inactiveTitleColor.setNamedColor( str );
		  if ( end )
		    *end = '\0';
	str = config->readEntry( "InactiveTitleTextColor", "#808080" );
	inactiveTextColor.setNamedColor( str );
		}
	  delete [] workPath;
	str = config->readEntry( "ActiveTitleBarColor", "#000080" );
	activeTitleColor.setNamedColor( str );
  // add paths in the KDEPATH environment variable
  const char *kdePathEnv = getenv( "KDEPATH" );
	str = config->readEntry( "ActiveTitleTextColor", "#FFFFFF" );
	activeTextColor.setNamedColor( str );
	  char *start, *end, *workPath = new char [ strlen( kdePathEnv ) + 1 ];
	  strcpy( workPath, kdePathEnv );
	str = config->readEntry( "TextColor", "#000000" );
	textColor.setNamedColor( str );
		{
	  	  end = strchr( start, ':' );
	str = config->readEntry( "BackgroundColor", "#C0C0C0" );
	backgroundColor.setNamedColor( str );
		  appendSearchPath( start );
		  start = end ? end + 1 : end;
	str = config->readEntry( "SelectColor", "#000080" );
	selectColor.setNamedColor( str );
    }

	str = config->readEntry( "SelectTextColor", "#FFFFFF" );
	selectTextColor.setNamedColor( str );

void KApplication::appendSearchPath( const char *path )
	str = config->readEntry( "WindowColor", "#FFFFFF" );
	windowColor.setNamedColor( str );

  // return if this path has already been added
	str = config->readEntry( "WindowTextColor", "#000000" ); 

  pSearchPaths->append( path );
}
	generalFont = QFont("helvetica", 12, QFont::Normal);
void KApplication::readSettings()
	config->setGroup( "General Font" );
  // use the global config files
	int num = config->readNumEntry( "Charset",-1 );
	if ( num>=0 )
		generalFont.setCharSet((QFont::CharSet)num);
  QString str;
	str = config->readEntry( "Family" );
	if ( !str.isNull() )
		generalFont.setFamily(str.data());
	
	str = config->readEntry( "Point Size" );
		if ( !str.isNull() )
		generalFont.setPointSize(atoi(str.data()));
  inactiveTitleColor.setNamedColor( str );
				
		if ( !str.isNull() )
  str = config->readEntry( "InactiveTitleTextColor", "#808080" );
  inactiveTextColor.setNamedColor( str );
	str = config->readEntry( "Italic" );
		if ( !str.isNull() )
			if ( atoi(str.data()) != 0 )
				generalFont.setItalic(True);
  str = config->readEntry( "ActiveTitleTextColor", "#FFFFFF" );
  activeTextColor.setNamedColor( str );

	config->setGroup( "GUI Style" );
	str = config->readEntry( "Style" );
	if ( !str.isNull() )

		if( str == "Windows 95" )
			applicationStyle=WindowsStyle;
		else
			applicationStyle=MotifStyle;
	// this default is Qt darkBlue
			applicationStyle=MotifStyle;	
  selectColor.setNamedColor( str );

	// this default is Qt white
  str = config->readEntry( "SelectTextColor", "#FFFFFF" );
  selectTextColor.setNamedColor( str );
	// WARNING : QApplication::setPalette() produces inconsistent results.
	// There are 3 problems :-
	// 1) You can't change select colors
	// 2) You need different palettes to apply the same color scheme to
	//		different widgets !!
	// 3) Motif style needs a different palette to Windows style.
  str = config->readEntry( "Charset","iso-8859-1" );
	int highlightVal, lowlightVal;

	highlightVal=100+(2*contrast+4)*16/10;
	lowlightVal=100+(2*contrast+4)*10;
	//	Read the font specification from config.
	// printf("contrast = %d\n", contrast);
		
	if ( applicationStyle==MotifStyle ) {
		QColorGroup disabledgrp( textColor, backgroundColor, 
    							backgroundColor.light(highlightVal),
    							backgroundColor.dark(lowlightVal), 
    							backgroundColor.dark(120),
                        	darkGray, windowColor );

    	QColorGroup colgrp( textColor, backgroundColor, 
    							backgroundColor.light(highlightVal),
    							backgroundColor.dark(lowlightVal), 
    							backgroundColor.dark(120),
                        	textColor, windowColor );
  if ( !str.isNull() )
    	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );
	
  str = config->readEntry( "Point Size" );
  if ( !str.isNull() )
	generalFont.setPointSize(atoi(str.data()));
    } else {
    	QColorGroup disabledgrp( textColor, backgroundColor, 
    							backgroundColor.light(150),
    							backgroundColor.dark(), 
    							backgroundColor.dark(120),
                        	darkGray, windowColor );

    	QColorGroup colgrp( textColor, backgroundColor, 
    							backgroundColor.light(150),
    							backgroundColor.dark(), 
    							backgroundColor.dark(120),
                        	textColor, windowColor );
  config->setGroup( "GUI Style" );
    	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );
  if ( !str.isNull() )
	{
	  if( str == "Windows 95" )
		applicationStyle=WindowsStyle;
	}
		applicationStyle=MotifStyle;
	} else
	  applicationStyle=MotifStyle;	
	
    QApplication::setFont( generalFont, TRUE );
    // setFont() works every time for me !
void KApplication::kdisplaySetPalette()
    emit kdisplayFontChanged();    
	emit appearanceChanged();
    resizeAll();
  // 1) You can't change select colors
  // 2) You need different palettes to apply the same color scheme to
  //		different widgets !!
  // 3) Motif style needs a different palette to Windows style.
	
    QApplication::setStyle( applicationStyle );
	
    emit kdisplayStyleChanged();
	emit appearanceChanged();
    resizeAll();
  // printf("contrast = %d\n", contrast);
	
  if ( applicationStyle==MotifStyle ) {
	QColorGroup disabledgrp( textColor, backgroundColor, 
							 backgroundColor.light(highlightVal),
    QApplication::setStyle( applicationStyle );
    QApplication::setFont( generalFont, TRUE );
							 darkGray, windowColor );
    emit kdisplayStyleChanged();
    emit kdisplayFontChanged();
	emit appearanceChanged();
    // 	setStyle() works pretty well but may not change the style of combo
    //	boxes.
						textColor, windowColor );
	resizeAll();
	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );

	emit kdisplayPaletteChanged();
	emit appearanceChanged();

	// send a resize event to all windows so that they can resize children
	QWidgetList *widgetList = QApplication::topLevelWidgets();
	QWidgetListIt it( *widgetList );
							 backgroundColor.dark(), 
	while ( it.current() )
							 darkGray, windowColor );
		it.current()->resize( it.current()->size() );
		++it;
						backgroundColor.light(150),
						backgroundColor.dark(), 
						backgroundColor.dark(120),
						textColor, windowColor );

	QApplication::setPalette( QPalette(colgrp,disabledgrp,colgrp), TRUE );

	emit kdisplayPaletteChanged();
    if ( fork() == 0 )	

	if( filename.isEmpty() )

void KApplication::kdisplaySetFont()
	QString path = KApplication::kdedir();
	path.append("/share/doc/HTML/default/");
	path.append(filename);
  QApplication::setStyle( applicationStyle );
	if( !topic.isEmpty() )
	{
	    path.append( "#" );
	    path.append(topic);
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
	path.prepend("kdehelp ");
	execl(shell, shell, "-c", path.data(), 0L);
	exit( 1 );
  // 	setStyle() works pretty well but may not change the style of combo
  //	boxes.
  applyGUIStyle(applicationStyle);
  QApplication::setFont( generalFont, TRUE );
  applyGUIStyle(applicationStyle);   
    
  QString kdedir = getenv("KDEDIR");
  emit kdisplayFontChanged();
  if (kdedir.isEmpty())

	kdedir = KDEDIR;
void KApplication::resizeAll()
    kdedir = "/usr/local/kde";
  // send a resize event to all windows so that they can resize children
  return dir;
}

const QString& KApplication::kde_cgidir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_CGIDIR;
  return dir;
}

const QString& KApplication::kde_minidir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_WALLPAPERDIR;
  return dir;
}
 
const QString& KApplication::kde_bindir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_BINDIR;
  return dir;
}

const QString& KApplication::kde_partsdir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_PARTSDIR;
  return dir;
}

const QString& KApplication::kde_configdir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_CONFIGDIR;
  return dir;
}

const QString& KApplication::kde_mimedir()
{
  static QString dir;
  if (dir.isNull()) 
	dir = KDE_MIMEDIR;
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

  fontfilename =  getenv("HOME");

  if(fontfilename.isEmpty()){
    return false;
  }
    
  fontfilename = fontfilename + "/.kde/share/config/kdefonts";

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


const char* KApplication::tempSaveName( const char* pFilename )
{
      KWM::setWmCommand( topWidget->winId(), argv()[0] );

  if( pFilename[0] != '/' )
	{
	  KDEBUG( KDEBUG_WARN, 101, "Relative filename passed to KApplication::tempSaveName" );
	  aFilename = QFileInfo( QDir( "." ), pFilename ).absFilePath();
	}
  else
	aFilename = pFilename;

  QDir aAutosaveDir( QDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
	{
	  if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
		{
		  // Last chance: use /tmp
		  aAutosaveDir.setPath( "/tmp" );
		}
	}

  aFilename.replace( QRegExp( "/" ), "\\!" ).prepend( "#" ).append( "#" ).prepend( "/" ).prepend( aAutosaveDir.absPath() );

  return qstrdup( aFilename.data() );
}



const char* KApplication::checkRecoverFile( const char* pFilename, 
											bool& bRecover )
{
  QString aFilename;

  if( pFilename[0] != '/' )
	{
	  KDEBUG( KDEBUG_WARN, 101, "Relative filename passed to KApplication::tempSaveName" );
	  aFilename = QFileInfo( QDir( "." ), pFilename ).absFilePath();
	}
  else
	aFilename = pFilename;

  QDir aAutosaveDir( QDir::homeDirPath() + "/autosave/" );
  if( !aAutosaveDir.exists() )
	{
	  if( !aAutosaveDir.mkdir( aAutosaveDir.absPath() ) )
		{
		  // Last chance: use /tmp
		  aAutosaveDir.setPath( "/tmp" );
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


void KApplication::setTopWidget( QWidget *topWidget )
{
  pTopWidget = topWidget;
  if (topWidget){
    // set the specified icons
    KWM::setIcon(topWidget->winId(), getIcon());
    KWM::setMiniIcon(topWidget->winId(), getMiniIcon());
    if (bSessionManagement)
      enableSessionManagement(bSessionManagementUserDefined);
    else
      KWM::setWmCommand( topWidget->winId(), aWmCommand);
  }
}

void KApplication::registerTopWidget()
{
}

void KApplication::unregisterTopWidget()
{
}
