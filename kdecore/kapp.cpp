// $Id$
// Revision 1.87  1998/01/27 20:17:01  kulow
// $Log$
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
// Revision 1.69  1997/11/03 14:41:28  ettrich
#ifndef _KAPP_H
// Matthias: tiny fix for command line arguments: "-icon" will also be used
#endif

#ifndef _KKEYCONF_H
//       for the miniicon if "-miniicon" is not defined.
#endif

#include <stdlib.h> // getenv()
// Revision 1.62  1997/10/17 15:46:22  stefan
// Matthias: registerTopWidget/unregisterTopWidget are obsolete and empty now.

// I'm not sure, why this have been removed, but I'm sure, they are
// needed.
//
// Revision 1.59  1997/10/16 11:14:27  torben
// Kalle: Copyright headers
// kdoctoolbar removed
	aAppName = aArgv0.remove( 0, nSlashPos );
// Revision 1.57  1997/10/14 13:31:57  kulow
// removed one more default value from the implementation
//
  init();
// Matthias: fix to generation of SM command
//
// Revision 1.54  1997/10/12 14:36:34  kalle
// KApplication::tempSaveName() and KApplication::checkRecoverFile()
// now implemented as documented.
// The change in kapp.h is only in the documentation and thus *-compatible.
//
// Revision 1.53  1997/10/11 22:39:27  ettrich
  init();
// Matthias: mainWidget -> topWidget for SM
// Matthias: fixed an async reply problem with invokeHTMLHelp
//
// Revision 1.50  1997/10/10 22:09:17  ettrich
  KApp = this;
#include <qtstream.h>
#include "kprocctrl.h"
  if( char* pHome = getenv( "HOME" ) )

KApplication* KApplication::KApp = 0L;
QStrList* KApplication::pSearchPaths;
  if( getenv( "USE_NEW_CONFIG_LOCATION" ) )
       aConfigName += "/.kde/config/.";
  else  
       aConfigName += "/.";

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

  display = XOpenDisplay( NULL );
  // by creating the KProcController instance (if its not already existing)
  if ( theKProcessController == 0L) 
    theKProcessController = new KProcessController();

  KApp = this;
  bLocaleConstructed = false; // no work around mutual dependencies
  
  pIconLoader = 0L;
  mkdir (configPath.data(), 0755); // make it public(?)
  configPath += "/share";
  KDEChangePalette = XInternAtom( display, "KDEChangePalette", False );
  KDEChangeGeneral = XInternAtom( display, "KDEChangeGeneral", False );
  KDEChangeStyle = XInternAtom( display, "KDEChangeStyle", False);
      else
        aIconPixmap = getIconLoader()->loadApplicationIcon( argv[i+1] );
  delete kKeys; // must be done befor "delete pConfig"
    case miniicon:
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
	  for ( dz = dropZones.first(); dz != 0L; dz = dropZones.next() )
		  getConfig()->setGroup("GUI Style");
		  QPoint p2 = dz->getWidget()->mapFromGlobal( p );
		  if ( dz->getWidget()->rect().contains( p2 ) )
			result = dz;
		      applyGUIStyle(MotifStyle);
		*/
		      if(str == "Windows 95")
			applyGUIStyle(WindowsStyle);
		  return TRUE;
		}
	  {
	  if ( cme->message_type == KDEChangePalette )
	      {
		  if ( dz->getWidget() == w )
		      result = dz;
	      }
		  return True;
		}
		  w = w->parentWidget();
	  }
		  readSettings();
		  kdisplaySetStyleAndFont();
		  kdisplaySetPalette();
	      for ( dz = dropZones.first(); dz != 0L; dz = dropZones.next() )
		  return True;
		  QPoint p2 = dz->getWidget()->mapFromGlobal( p );
		  if ( dz->getWidget()->rect().contains( p2 ) )
	  if ( cme->message_type == DndLeaveProtocol )
		{
		  if ( lastEnteredDropZone != 0L )
			lastEnteredDropZone->leave();
	  {
		  lastEnteredDropZone = 0L;
	      {
		  result->drop( (char*)Data, Size, (int)cme->data.l[0], p.x(), p.y() );
	      }
	  else if ( cme->message_type != DndProtocol && cme->message_type != DndEnterProtocol &&
	      {
	    return FALSE;
		  if ( lastEnteredDropZone != 0L && lastEnteredDropZone != result )
		      lastEnteredDropZone->leave();
	  
		  // Notify the drop zone over which the pointer is right now.
		  result->enter( (char*)Data, Size, (int)cme->data.l[0], p.x(), p.y() );
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
	emit kdisplayPaletteChanged();
	emit appearanceChanged();

	// send a resize event to all windows so that they can resize children
  if ( fork	() == 0 )	
	QWidgetListIt it( *widgetList );
	  if( filename.isEmpty() )
		filename = aAppName + ".html";
      QString path = KApplication::kdedir();
      path.append("/doc/HTML/");
      path.append(filename);
						backgroundColor.dark(), 
	  if( topic )
		{
		  path.append( "#" );
		  path.append(topic);
		}

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
