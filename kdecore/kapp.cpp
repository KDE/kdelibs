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
//
// $Log$
// Revision 1.122  1999/01/18 10:56:12  kulow
// .moc files are back in kdelibs. Built fine here using automake 1.3
//
// Revision 1.121  1999/01/15 09:30:30  kulow
// it's official - kdelibs builds with srcdir != builddir. For this I
// automocifized it, the generated rules are easier to maintain than
// selfwritten rules. I have to fight with some bugs of this tool, but
// generally it's better than keeping them updated by hand.
//
// Revision 1.120  1999/01/10 14:48:47  ettrich
// fixed default widget style
//
// Revision 1.119  1998/12/13 20:32:57  ettrich
// reverted some of the previous changes
//
// Revision 1.118  1998/12/13 20:04:16  ettrich
// yet another fix for user defined SM
//
// Revision 1.117  1998/12/13 18:03:54  ettrich
// bugfix for user defined session management
//
// Revision 1.116  1998/12/05 01:32:59  ettrich
// support for icon text
//
// Revision 1.115  1998/11/08 19:06:43  esken
// Several security fixes by adding checkAccess() tests before the creation
// of files and directorys. This is neccesary for SUID programs. Added
// checkAccess(), which checks if the user may write a file.
//
// checkAccess() is a global function, perhaps it should be moved to some
// KTools class as static member funtion later.
//
// Revision 1.114  1998/10/12 13:22:54  ettrich
// Matthias: small fix
//
// Revision 1.113  1998/10/12 00:17:09  ettrich
// Matthias: automatic removal of session management temp-files
//
// Revision 1.112  1998/09/28 07:54:23  garbanzo
// Remove some unneeded/unused kdebug gunk.
//
// Revision 1.111  1998/09/21 18:33:30  bieker
// Use paths.h if we can.
//
// Revision 1.110  1998/09/10 23:47:29  markus
// //$markus: replaced NULL with 0L in aboutKDE(). This caused trouble on
// the netwinder.
//
// Revision 1.109  1998/09/01 20:21:14  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.108  1998/08/23 15:58:31  kulow
// fixed some more advanced warnings
//
// Revision 1.107  1998/08/11 00:24:57  steffen
// KProcess stuff removed
//
// Revision 1.106  1998/07/23 20:55:11  ettrich
// Matthias: new configurable keybindings
//
// Revision 1.105  1998/07/02 15:42:42  ettrich
// Matthias: Added a new signal
//
//                  void shutDown();
//
//           which is emitted when the client is killed (for example via
//           kwm's logout)
//
// Revision 1.104  1998/06/21 15:03:37  konold
//
// Martin K.: Fixed typo            Coolo does this affect the translations?? Please check.
//
// Revision 1.103  1998/06/15 12:49:29  kulow
// applied patch to replace .kde with localkdedir()
//
// Revision 1.102  1998/05/28 21:43:52  kulow
// removed TESTDIR. Since kdedir() is protected, it shouldn't happen anymore :)
//
// Revision 1.101  1998/04/12 08:52:36  jacek
//
// * Updtated KCharset class documentation.
//
// * Chnged font entry in config files to store charset as string, not numerical
//   id (which worked only for iso-8859-* charsets)
//
// * added KCharset::xCharset method for getting charset name to use for X font
//   names.
//
// Revision 1.100  1998/04/03 14:25:59  kulow
// fixed a memory leak
//
// Revision 1.99  1998/03/27 17:27:31  mark
// MD:
// - Added call to setWinStyleHighlightColor()
// - changed disabled text color from darkGray to colorGroup().mid()
// - changed config keys and groups to read color scheme and fonts
// - used readFontEntry for reading generalFont and fixedFont
//
// Revision 1.98  1998/03/21 23:05:12  mark
// MD: Addition of a new font, a fixed font, for general use in applications.
// Unfortunately, I think this is a BINARY INCOMPATIBLE change. (Cleared with Kalle.)
//
// Revision 1.97  1998/03/12 21:17:01  wuebben
// Bernd: updated 'about KDE' text. It is now more in line with KDE goals
//
// Revision 1.96  1998/03/11 11:22:24  kulow
// use readColorEntry instead of the old way. This is more consistent
//
// Revision 1.95  1998/03/09 20:20:11  kulow
// - set KApp to 0, after the destructor has run. After that, nothing is for sure
// - remove widgetList given by QApplication::topLevelWidgets. You can see this
//   in the documentation
//
// Revision 1.94  1998/02/24 21:36:04  kulow
// fix for invokeHTMLHelp: missing / behind the path
//
// Revision 1.93  1998/02/24 21:16:22  kulow
// remove kde_minidir, since this is somehow stupid (I hard coded it in the
// Makefiles as icondir/mini now) and added kde_sounddir
// This is even source incompatible, but it hasn't been used, so it should
// work after some little changes I will make
//
// Revision 1.92  1998/02/24 20:07:45  kalle
// Added F1 accelerator to standard help menu
//
// Revision 1.91  1998/02/24 15:54:29  kulow
// replaced some hard coded paths with the kapp->kde_ methodes.
// I'm not sure, if kde_datadir() is optimal for /share/apps ;)
//
// Revision 1.90  1998/02/20 06:21:18  kalle
// Don't connect the Help/About slot when there is no text for an about box
//
// Revision 1.89  1998/02/19 05:58:22  rwilliam
// Modified getHelpMenu()
//    Now displays Content instead of Help...
//
// Revision 1.88  1998/02/07 20:44:05  kulow
// good news: KDE is relocatable again. kdelibs's configure is now inserting KDEDIR in the directories name and the kde_dir functions check for this.
// This way we can have the advantages of both sides. It's relocatable with $KDEDIR
// and you can hardcode specific directories, if you don't use relative paths.
// All, that is needed now is to patch the applications to use the kapp::kde_ functions
//
// Revision 1.87  1998/01/27 20:17:01  kulow
// applied patch by Kalle to make invokeHTMLHelp use the locale setting.
//
// Revision 1.86  1998/01/25 20:35:48  kulow
// corrected order of signal and action (a party for the four-eyes-strategy ;)
//
// Revision 1.85  1998/01/25 20:09:43  kulow
// patch created by me after Mario told me, what to do ;)
// applyGUIStyle does not work exactly like QApplication::setStyle
//
// Revision 1.84  1998/01/22 16:29:45  jacek
// Default locale's charset support added
//
// Revision 1.83  1998/01/17 07:35:16  kalle
// Uh-oh...
//
// Revision 1.82  1998/01/16 21:29:30  kalle
// mention the new bug site in the about dialog
//
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
// readded my yesterday bugfixes. I hope, I have not forgotten one.
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
//
// Revision 1.51  1997/10/10 22:36:21  ettrich
// Matthias: fixed an async reply problem with invokeHTMLHelp
//
// Revision 1.50  1997/10/10 22:09:17  ettrich
// Matthias: BINARY INCOMPATIBLE CHANGES: extended session management support

#include <qdir.h> // must be at the front

#include <kapp.h>
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
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qkeycode.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h> // getenv()
#include <signal.h>


#include <qwidcoll.h>

#include "kprocctrl.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

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

  // create the config directory ~/.kde/share/config
  QString configPath = KApplication::localkdedir();
  // We should check if  mkdir() succeeds, but since we cannot do much anyway...
  // But we'll check at least for access permissions (for SUID case)
  // Don't access if ~/ is unknown (as in kdm).
  if ( (QDir::home() != QDir::root()) && checkAccess(configPath, W_OK) ) {
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
      chown(aConfigFile.name(), getuid(), getgid());
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
    KWM::enableSessionManagement(topWidget()->winId());
  }
}

void KApplication::setWmCommand(const char* s){
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
									   const char* aboutAppText )
{
  QPopupMenu* pMenu = new QPopupMenu();

  int id = pMenu->insertItem( klocale->translate( "&Contents" ) );
  pMenu->connectItem( id, this, SLOT( appHelpActivated() ) );
  pMenu->setAccel( Key_F1, id );

  pMenu->insertSeparator();

  id = pMenu->insertItem( QString(klocale->translate( "&About" )) + " " + aAppName + "...");
  if( aboutAppText )
	{
	  pMenu->connectItem( id, this, SLOT( aboutApp() ) );
	  aAppAboutString = aboutAppText;
	}

  id = pMenu->insertItem( klocale->translate( "About &KDE..." ) );
  pMenu->connectItem( id, this, SLOT( aboutKDE() ) );
  /*
	if( bAboutQtMenu )
	{
	id = pMenu->insertItem( klocale->translate( "About Qt" ) );
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
  QMessageBox::about( 0L, klocale->translate( "About KDE" ),
					  klocale->translate(
"\nThe KDE Desktop Environment was written by the KDE Team,\n"
"a world-wide network of software engineers committed to\n"
"free software development.\n\n"
"Visit http://www.kde.org for more information on the KDE\n"
"Project. Please consider joining and supporting KDE.\n\n"
"Please report bugs at http://buglist.kde.org.\n"
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
  if ( e->type() == Event_KeyPress )
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
  const char *parameter_strings[] = { "-caption", "-icon", "-miniicon", "-restore" , 0 };

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
			pSessionConfig = new KConfig(0L, aSessionConfigName);
			
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

void KApplication::applyGUIStyle(GUIStyle newstyle) {
  QApplication::setStyle( applicationStyle );

  // get list of toplevels
  QWidgetList *wl = QApplication::topLevelWidgets();
  QWidgetListIt wl_it( *wl );

  // foreach toplevel ...
  while(wl_it.current()) {
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

  return fullPath;
}


const char* KApplication::getCaption() const
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
  const char *kdePathEnv = getenv( "KDEPATH" );
  if ( kdePathEnv )
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

void KApplication::appendSearchPath( const char *path )
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
	
  // Read the color scheme group from config file
  // If unavailable set color scheme to KDE default

  config->setGroup( "General");
  // this default is Qt black
  textColor = config->readColorEntry( "foreground", &black );

  // this default is the Qt lightGray
  backgroundColor = config->readColorEntry( "background", &lightGray  );

  // this default is Qt darkBlue
  selectColor = config->readColorEntry( "selectBackground", &darkBlue );

  // this default is Qt white
  selectTextColor = config->readColorEntry( "selectForeground", &white);

  // this default is Qt white
  windowColor = config->readColorEntry( "windowBackground", &white );

  // this default is Qt black
  windowTextColor = config->readColorEntry( "windowForeground", &black );
	
  config->setGroup( "WM");
  // this default is Qt lightGray
  inactiveTitleColor = config->readColorEntry( "inactiveBackground", &lightGray );

  // this default is Qt darkGrey
  inactiveTextColor = config->readColorEntry( "inactiveForeground", &darkGray );

  // this default is Qt darkBlue
  activeTitleColor = config->readColorEntry( "activeBackground", &darkBlue );

  // this default is Qt white
  activeTextColor = config->readColorEntry( "activeForeground", &white );

  config->setGroup( "KDE");
  contrast = config->readNumEntry( "contrast", 7 );

  //  Read the font specification from config.
  //  Initialize fonts to default first or it won't work !!
		
  pCharsets->setDefault(klocale->charset());
  generalFont = QFont("helvetica", 12, QFont::Normal);
  pCharsets->setQFont(generalFont);
  fixedFont = QFont("fixed", 12, QFont::Normal);
  pCharsets->setQFont(fixedFont);

  config->setGroup( "General" );
  generalFont = config->readFontEntry( "font", &generalFont );
  fixedFont = config->readFontEntry( "fixedFont", &fixedFont );

  // Finally, read GUI style from config.
	
  config->setGroup( "KDE" );
  if ( config->readEntry( "widgetStyle", "Windows 95" ) == "Windows 95" )
    applicationStyle=WindowsStyle;
  else
    applicationStyle=MotifStyle;
	
}

void KApplication::kdisplaySetPalette()
{
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
}

void KApplication::kdisplaySetFont()
{
  QApplication::setFont( generalFont, TRUE );
  // setFont() works every time for me !

  emit kdisplayFontChanged();
  emit appearanceChanged();

  resizeAll();
}	


void KApplication::kdisplaySetStyle()
{
  // QApplication::setStyle( applicationStyle );
  applyGUIStyle( applicationStyle );

  emit kdisplayStyleChanged();
  emit appearanceChanged();
  resizeAll();
}	


void KApplication::kdisplaySetStyleAndFont()
{
  //  QApplication::setStyle( applicationStyle );
  // 	setStyle() works pretty well but may not change the style of combo
  //	boxes.
  QApplication::setFont( generalFont, TRUE );
  applyGUIStyle(applicationStyle);

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
const QString& KApplication::kde_htmldir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_HTMLDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_appsdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_APPSDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_icondir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_ICONDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_datadir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_DATADIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_localedir()
{
  static QString dir;
  if (dir.isNull()) {
	dir = KDE_LOCALE;
	if (!strncmp(dir.data(), "KDEDIR", 6))
	    dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_cgidir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_CGIDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_sounddir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_SOUNDDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_toolbardir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_TOOLBARDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_wallpaperdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_WALLPAPERDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_bindir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_BINDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_partsdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_PARTSDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_configdir()
{
  static QString dir;
  if (dir.isNull()) {
      dir = KDE_CONFIGDIR;
      if (!strncmp(dir.data(), "KDEDIR", 6))
	  dir = kdedir() + dir.right(dir.length() - 6);
  }
  return dir;
}

const QString& KApplication::kde_mimedir()
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

  fontfilename = KApplication::localkdedir();

  if(fontfilename.isEmpty()){
    return false;
  }

  fontfilename = fontfilename + "/share/config/kdefonts";

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
		  // Last chance: use _PATH_TMP
		  aAutosaveDir.setPath( _PATH_TMP );
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


bool checkAccess(const char *pathname, int mode)
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

  dirName.resize(pos+1); // strip everything starting from the last '/'

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

#include "kapp.moc"
