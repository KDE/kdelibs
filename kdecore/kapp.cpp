// $Id$
// Revision 1.87  1998/01/27 20:17:01  kulow
// $Log$
// Revision 1.19  1997/08/29 23:52:02  kdecvs
// Kalle: Do not crash when $HOME/.kde/config does not exist
//
// Revision 1.18  1997/08/29 15:56:55  kdecvs
// Torben: added some needed functions and bugfixes for kfm-0.9
//
// Revision 1.17  1997/08/07 08:50:35  ettrich
// Matthias: replaced $KDEDIR with kdedir() in the searchpath
//
// Revision 1.16  1997/07/27 13:43:57  kalle
// Even more SGI and SCC patches, security patch for kapp, various fixes for ksock
//
// Revision 1.15  1997/07/25 19:46:41  kalle
// SGI changes
//
// Revision 1.14  1997/07/24 21:04:38  kalle
// Kalle: Patches for SGI
//
// Revision 1.13  1997/06/29 18:26:35  kalle
// 29.06.97:	KConfig reads and writes string lists
// 			Torben's patches to ktoolbar.*, kurl.h
//
// 22.06.97:	KApplications save and restore position and size of their top
// (unstable)	level widget.
//
// Revision 1.12  1997/05/30 20:04:37  kalle
// Kalle:
// 30.05.97:	signal handler for reaping zombie help processes reinstalls itself
// 		patch to KIconLoader by Christian Esken
// 		slightly better look for KTabCtl
// 		kdecore Makefile does not expect current dir to be in path
// 		Better Alpha support
//
// Revision 1.11  1997/05/17 20:38:21  kalle
// Kalle:
// - Bugfix for KPanner (from Paul Kendall)
// - Better colors with kdisplay schemes (from Bernd Wuebben)
// - new behavior in KApplication::invokeHTMLHelp(): if the first
//   argument (the filename) is empty, the filename is defaulted to
// 	$KDEDIR/doc/HTML/<appname>/<appname>.html
// - KApplication::getCaption for Matthias added (breaks binary compatibility!)
//
// Revision 1.10  1997/05/15 20:33:19  wuebben
// Bernd: Added signals:
// kdisplayFontChanged()
// kdisplayPaletteChanged()
// kdisplayStyleChaned()
// Some applications such as kedit need to know when the Font, Palette,
// or Style ngeduy.
//
// Revision 1.9  1997/05/13 05:49:00  kalle
// Kalle: Default arguments for KConfig::read*Entry()
// app-specific config files don't start with a dot
// Bufgix for the bugfix in htmlobj.cpp (FontManager)
//
// Revision 1.8  1997/05/09 19:52:54  kalle
// Kalle: Application-specific config files without leading .
//
// Revision 1.7  1997/05/08 22:53:16  kalle
// Kalle:
// KPixmap gone for good
// Eliminated static objects from KApplication and HTML-Widget
//
// Revision 1.6  1997/05/02 16:46:34  kalle
// Kalle: You may now override how KApplication reacts to external changes
// KButton uses the widget default palette
// new kfontdialog version 0,5
// new kpanner by Paul Kendall
// new: KIconLoader
//
// Revision 1.5  1997/04/28 06:57:44  kalle
// Various widgets moved from apps to libs
// Added KSeparator
// Several bugs fixed
// Patches from Matthias Ettrich
// Made ksock.cpp more alpha-friendly
// Removed XPM-Loading from KPixmap
// Reaping zombie KDEHelp childs
// WidgetStyle of KApplication objects configurable via kdisplay
//
// Revision 1.4  1997/04/23 16:45:14  kulow
// fixed a bug in acinclude.m4 (Thanks to Paul)
// solved some little problems with some gcc versions (no new code)
//
// Revision 1.3  1997/04/21 22:37:23  kalle
// Bug in Kconfig gefixed (schrieb sein app-spezifisches File nicht mehr)
// kcolordlg und kapp abgedated (von Martin Jones)
//
// Revision 1.2  1997/04/15 20:01:55  kalle
// Kalles changes for 0.8
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:06  cvsuser
// Sources imported
//
// Revision 1.10  1997/03/16 22:29:56  kalle
// look for config variables in ~/.kde/config/.appnamerc if the
// environment variable USE_NEW_CONFIG_LOCATION is set
//
// Revision 1.9  1997/03/16 22:21:26  kalle
// patches from Nicolas Hadacek
//
// Revision 1.8  1997/03/16 21:59:58  kalle
// initialize KKeyConfig
//
// Revision 1.7  1997/03/10 20:04:39  kalle
// Merged changes from 0.6.3
//
// Revision 1.6  1997/03/09 17:28:46  kalle
// KTextStream -> QTextStream
//
// Revision 1.5  1997/01/15 20:14:28  kalle
// merged changes from 0.52
//
// Revision 1.4  1996/12/14 13:50:08  kalle
// finding out the state the application config object is in
//
// Revision 1.3  1996/12/07 22:23:26  kalle
// Drag and Drop stuff from Torben added
//
// Revision 1.2  1996/12/07 17:50:49  kalle
// Patches from Nicolas
//
// Revision 1.1  1996/12/01 11:19:55  kalle
// Initial revision
//
//
// KApplication implementation
//
// (C) 1996 Matthias Kalle Dalheimer <mda@stardivision.de>
//
// DND stuff by Torben Weis <weis@stud.uni-frankfurt.de>
// 09.12.96
//
//
// Revision 1.69  1997/11/03 14:41:28  ettrich
#ifndef _KAPP_H
// Matthias: tiny fix for command line arguments: "-icon" will also be used
#endif

#ifndef _KKEYCONF_H
//       for the miniicon if "-miniicon" is not defined.
#endif

//   PseudoSessionManagement (this is the default when session management
#include <stdlib.h> // getenv()
//   Now KApplication should work as promised in kapp.h :-)
// Revision 1.66  1997/10/25 22:27:40  kalle
// Fixed bug with default help menu (Thanks, Bernd! This one was just in time!)
//
// Matthias: bugfixes for session management.
// Revision 1.63  1997/10/21 20:44:41  kulow
// removed all NULLs and replaced it with 0L or "".
//
// Revision 1.62  1997/10/17 15:46:22  stefan
// compiler bug. I know - I should upgrade gcc, but for all the
// poor folks that still have the old one ;-)
// Matthias: registerTopWidget/unregisterTopWidget are obsolete and empty now.
//           Introduced new registration model

// prevents children from going zombie
void reaper(int) 
{
  
  wait(0);

  /* The signal handler may not be reinstalled before the call to wait
   * because brain-dead System V systems then would call this signal
   * handler continuously. (See Stevens: Advanced Programming in the
   * Unix Environment, section 10.7. for details.)
   */
  (void)signal(SIGCHLD, reaper); 
}
// Revision 1.60  1997/10/16 11:35:24  kulow
// I'm not sure, why this have been removed, but I'm sure, they are
// needed.
//
  rootDropZone = 0L;
    
// Revision 1.59  1997/10/16 11:14:27  torben
// Kalle: Copyright headers
// kdoctoolbar removed
	aAppName = aArgv0.remove( 0, nSlashPos );
// Revision 1.57  1997/10/14 13:31:57  kulow
// removed one more default value from the implementation
//
//
// Revision 1.55  1997/10/13 11:00:04  ettrich
  init();
// Matthias: fix to generation of SM command
//
// Revision 1.54  1997/10/12 14:36:34  kalle
// KApplication::tempSaveName() and KApplication::checkRecoverFile()
// now implemented as documented.
// The change in kapp.h is only in the documentation and thus *-compatible.
  rootDropZone = 0L;

//
// Revision 1.53  1997/10/11 22:39:27  ettrich
//
// Revision 1.52  1997/10/11 19:25:32  ettrich
  init();
// Matthias: mainWidget -> topWidget for SM
// Matthias: fixed an async reply problem with invokeHTMLHelp
//
// Revision 1.50  1997/10/10 22:09:17  ettrich
  (void)signal(SIGCHLD, reaper);
#include "kwm.h"
  KApp = this;
#include <qtstream.h>
  pConfigStream = NULL;

#include "kprocctrl.h"

KCharsets* KApplication::pCharsets = 0L;

KApplication* KApplication::KApp = 0L;
QStrList* KApplication::pSearchPaths;
  if( getenv( "USE_OLD_CONFIG_LOCATION" ) )
       aConfigName += "/.";
  else  
       aConfigName += "/.kde/config/";

KApplication::KApplication( int& argc, char** argv ) :
  QApplication( argc, argv )
  pConfigFile = new QFile( aConfigName );
  int nSlashPos = aArgv0.findRev( '/' );
  if( nSlashPos != -1 )
  bool bSuccess = pConfigFile->open( IO_ReadWrite ); 
  else
	aAppName = aArgv0;

	  bSuccess = pConfigFile->open( IO_ReadOnly );

  parseCommandLine( argc, argv );

		  pConfig = new KConfig();


KApplication::KApplication( int& argc, char** argv, const QString& rAppName ) :
  QApplication( argc, argv )
{
		  pConfigStream = new QTextStream( pConfigFile );
		  pConfig = new KConfig( pConfigStream );

  init();

  parseCommandLine( argc, argv );

}
	  pConfigStream = new QTextStream( pConfigFile );
	  pConfig = new KConfig( pConfigStream );
void KApplication::init()
{
  // this is important since we fork() to launch the help (Matthias)
  
  // try to read a global application file
  {
    QString aConfigName;
    aConfigName = kdedir();
    aConfigName += "/config/";
    aConfigName += aAppName;
    aConfigName += "rc";
    QFile pConfigFile( aConfigName );
    // try to open read-only
    bSuccess = pConfigFile.open( IO_ReadOnly );
    if( bSuccess )
      {
	// we succeeded to open an app-config file read-only
	{
	  QTextStream  pConfigStream( &pConfigFile );
	  pConfig->parseOneConfigFile( &pConfigStream );
	}
	// parse the application file again to preserve priority
	if( eConfigState != APPCONFIG_NONE )
	  {
		pConfig->parseOneConfigFile(pConfigStream);
	  }
      }
    pConfigFile.close();
  }
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

  // try to read a global application file
  QString aGlobalAppConfigName = kdedir() + "/share/config/" + aAppName + "rc";
  QFile aGlobalAppConfigFile( aGlobalAppConfigName );
  // try to open read-only
  bool bSuccess = aGlobalAppConfigFile.open( IO_ReadOnly );
  if( !bSuccess )
  KDEChangePalette = XInternAtom( display, "KDEChangePalette", False );
  KDEChangeGeneral = XInternAtom( display, "KDEChangeGeneral", False );
  KDEChangeStyle = XInternAtom( display, "KDEChangeStyle", False);
void KApplication::restoreTopLevelGeometry() const
{
  QWidget* mw = kapp->mainWidget();
  if( !mw )
	return;
  
  pConfig->setGroup( "Geometry" );

  int x = pConfig->readNumEntry( "TopLevelGeometry-x", mw->x() );
  int y = pConfig->readNumEntry( "TopLevelGeometry-y", mw->y() );
  int w = pConfig->readNumEntry( "TopLevelGeometry-w", mw->width() );
  int h = pConfig->readNumEntry( "TopLevelGeometry-h", mw->height() );
  mw->setGeometry( x, y, mw->width(), mw->height() );
  mw->resize( w, h );
}
 
    pLocale = new KLocale();

  return pLocale;
  for( int i = 0; i < argc; i++ )
	if( !strcmp( argv[i], "-caption" ) )
	  aCaption = argv[i+1];
}  

        aIconPixmap = QPixmap(argv[i+1]);
      else
        aIconPixmap = getIconLoader()->loadApplicationIcon( argv[i+1] );
  /* If there is a main level widget, save its position and size */
  QWidget* w = kapp->mainWidget();
  if( !w )
	debug( "No main widget when running destructor\n" );
  if( w )
	{
	  pConfig->setGroup( "Geometry" );
	  pConfig->writeEntry( "TopLevelGeometry-x", w->frameGeometry().x() );
	  pConfig->writeEntry( "TopLevelGeometry-y", w->frameGeometry().y() );
	  pConfig->writeEntry( "TopLevelGeometry-w", w->frameGeometry().width() );
	  pConfig->writeEntry( "TopLevelGeometry-h", w->frameGeometry().height() );
	}

		  aMiniIconPixmap = aIconPixmap;
      aDummyString2 += argv[i+1];
      aDummyString2 += " ";
      break;
    case miniicon:
  if( pConfigStream )
	delete pConfigStream;
  pConfigFile->close();
  delete pConfigFile;
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
	KConfig config;
	
	return aAppName;
}
	// This is the default light gray for KDE
	QColor col;
	col.setRgb(214,214,214);


		  return;

void KApplication::buildSearchPaths()
	config.setGroup( "Color Scheme");
	str = config.readEntry( "InactiveTitleBarColor" );
	if ( !str.isNull() )
		inactiveTitleColor.setNamedColor( str );
	else
		inactiveTitleColor = gray;
		
	str = config.readEntry( "InactiveTitleTextColor" );
	if ( !str.isNull() )
		inactiveTextColor.setNamedColor( str );
	else
		inactiveTextColor = col;
		
	str = config.readEntry( "ActiveTitleBarColor" );
	if ( !str.isNull() )
		activeTitleColor.setNamedColor( str );
	else
		activeTitleColor = black;
		
	str = config.readEntry( "ActiveTitleTextColor" );
	if ( !str.isNull() )
		activeTextColor.setNamedColor( str );
	else
		activeTextColor = white;
		
	str = config.readEntry( "TextColor" );
	if ( !str.isNull() )
		textColor.setNamedColor( str );
	else
		textColor = black;
		
	str = config.readEntry( "BackgroundColor" );
	if ( !str.isNull() )
		backgroundColor.setNamedColor( str );
	else
		backgroundColor = col;
		
	str = config.readEntry( "SelectColor" );
	if ( !str.isNull() )
		selectColor.setNamedColor( str );
	else
		selectColor = black;	
	
	str = config.readEntry( "SelectTextColor" );
	if ( !str.isNull() )
		selectTextColor.setNamedColor( str );
	else
		selectTextColor = white;
		
	str = config.readEntry( "WindowColor" );
	if ( !str.isNull() )
		windowColor.setNamedColor( str );
	else
		windowColor = white;
		
	str = config.readEntry( "WindowTextColor" );
	if ( !str.isNull() )
		windowTextColor.setNamedColor( str );
	else
		windowTextColor = black;
	
    }

	str = config->readEntry( "SelectTextColor", "#FFFFFF" );
	selectTextColor.setNamedColor( str );

void KApplication::appendSearchPath( const char *path )
	config.setGroup( "General Font" );
	str = config.readEntry( "Family" );

  // return if this path has already been added
	str = config->readEntry( "WindowTextColor", "#000000" ); 
	str = config.readEntry( "Point Size" );
  pSearchPaths->append( path );
}
	generalFont = QFont("helvetica", 12, QFont::Normal);
	str = config.readEntry( "Weight" );
	config->setGroup( "General Font" );
  // use the global config files
	int num = config->readNumEntry( "Charset",-1 );
	str = config.readEntry( "Italic" );
		generalFont.setCharSet((QFont::CharSet)num);
  QString str;
	str = config->readEntry( "Family" );
	if ( !str.isNull() )
		generalFont.setFamily(str.data());
	
	config.setGroup( "GUI Style" );
	str = config.readEntry( "Style" );
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
  selectTextColor.setNamedColor( str );
	// WARNING : QApplication::setPalette() produces inconsistent results.
    							backgroundColor.light(150),
    							backgroundColor.dark(), 
	// 2) You need different palettes to apply the same color scheme to
	//		different widgets !!
	// 3) Motif style needs a different palette to Windows style.
  str = config->readEntry( "Charset","iso-8859-1" );
    							backgroundColor.light(150),
    							backgroundColor.dark(), 
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
  if ( fork	() == 0 )	
	QWidgetListIt it( *widgetList );
	  if( filename.isEmpty() )
		filename = aAppName + '/' + aAppName + ".html";
      QString path = KApplication::kdedir();
      path.append("/doc/HTML/");
      path.append(filename);
						backgroundColor.dark(), 
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
      execlp( "kdehelp", "kdehelp", path.data(), 0 ); 
      exit( 1 );
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
  if(fontlist == NULL)

	kdedir = KDEDIR;
void KApplication::resizeAll()
    kdedir = "/usr/local/kde";
  // send a resize event to all windows so that they can resize children
  return dir;
}

  fontfilename = fontfilename + "/.kde/config/kdefonts";
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
