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
// Revision 1.48  1998/05/09 15:19:29  kulow
// spanish updates by Boris
//
// Revision 1.47  1998/04/06 10:06:48  kulow
// made kdedir() protected. I have to announce it after lunch ;)
//
// Revision 1.46  1998/03/29 19:07:28  kalle
// Methods for reading and writing bool, unsigned int, long, unsigned long,
// double, QRect, QSize, QPoint
//
// Revision 1.45  1998/03/21 23:05:14  mark
// MD: Addition of a new font, a fixed font, for general use in applications.
// Unfortunately, I think this is a BINARY INCOMPATIBLE change. (Cleared with Kalle.)
//
// Revision 1.44  1998/02/24 21:16:24  kulow
// remove kde_minidir, since this is somehow stupid (I hard coded it in the
// Makefiles as icondir/mini now) and added kde_sounddir
// This is even source incompatible, but it hasn't been used, so it should
// work after some little changes I will make
//
// Revision 1.43  1998/02/24 15:24:57  kulow
// the comment for kde_appsdir was wrong. I corrected it to:
// Returns the directory where KDE applications store their .kdelnk file
//
// Revision 1.42  1998/01/19 12:15:06  ssk
// Cleaned up some documentation.
//
// Revision 1.41  1998/01/06 22:54:29  kulow
// fixed my new macro. I guess, this way it's better
//
// Revision 1.40  1998/01/06 21:22:47  kulow
// added a macro i18n. klocale->translate is much to long and I should have done
// this way earlier ;)
// I would like to add this macro to klocale.h, but I'm afraid, this will make
// the translation of several office packages harder :)
//
// Revision 1.39  1997/12/28 21:32:15  kulow
// last time I forgot two functions kde_mimedir() and kde_confdir()
// I will move this functions very soon to static functions of a new
// class KPaths. Kapplication is not the optimal class to contain this
// functions, since I need a DISPLAY to find out the paths. But I think,
// we can create some inline methods in kapp then
//
// Revision 1.38  1997/12/13 15:08:58  jacek
// KCharsets support added
// WARNING: kderc file format has changed. Since now charset name
// instead of id is stored.
//
//
// Revision 1.37  1997/11/20 22:24:31  kalle
// new static methods for the various directories
//
// Revision 1.36  1997/11/18 21:40:43  kalle
// KApplication::localconfigdir()
// KApplication::localkdedir()
// KConfig searches in $KDEDIR/share/config/kderc
//
// Revision 1.35  1997/10/22 20:42:56  kalle
// Help menu works as advertised
//
// Revision 1.34  1997/10/17 17:32:08  ettrich
// Matthias: typo in documentation :)
//
// Revision 1.33  1997/10/17 13:30:18  ettrich
// Matthias: registerTopWidget/unregisterTopWidget are obsolete and empty now.
//           Introduced new registration model
//
// Revision 1.32  1997/10/16 11:14:28  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.31  1997/10/12 14:37:22  kalle
// Documentation correction
//
// Revision 1.29  1997/10/11 22:39:30  ettrich
// Matthias: BINARY INCOMPATIBLE CHANGES
//     - removed senseless method setUnsavedData
//
// Revision 1.28  1997/10/11 13:32:14  kalle
// pAppData in KApplication
// KTabListBox now in libkdeui
//
// Revision 1.27  1997/10/10 22:09:18  ettrich
// Matthias: BINARY INCOMPATIBLE CHANGES: extended session management support
//
// Revision 1.26  1997/10/05 12:52:41  kalle
// Three new methods from Mark Donohoe
//
// Revision 1.25  1997/10/04 19:42:45  kalle
// new KConfig
//
// Revision 1.24  1997/09/29 19:27:03  kalle
// Save icon and mini-icon pixmap
// SORRY, BUTTHH THIS IS AGAIN BINARY INCOMPATIBLE!!!!!!!!!!!!!!!
//
// Revision 1.23  1997/09/29 18:57:36  kalle
// BINARY INCOMPATIBLE!!!!!!!!!!!!!!!!!!!!!!
// Support for session management
// make KDebug compile on HP-UX
//
// Revision 1.22  1997/09/26 07:01:13  kalle
// Here are the promised dummies for session management:
//
// - KApplication::saveYourself (signal)
// - KApplication::tempFileName()
// - KApplication::checkRecoverFile()
//
// A short documentation is in kapp.h, more docs will follow.
//
// Revision 1.21  1997/09/19 16:28:42  mark
// MD: Implemented reading of the colour scheme contrast variable. This allows
// you to choose the highlights and lowlights used to draw widgets and has been
// part of the colour scheme specification for some time
//
// Sorry if this breaks anything; it is the last of the binary incompatible
// changes that Kalle announced yesterday.
//
// Revision 1.20  1997/09/11 19:44:54  kalle
// New debugging scheme for KDE (binary incompatible!)

#ifndef _KAPP_H
#define _KAPP_H

#include <kconfig.h>


#ifndef klocale
#define klocale KApplication::getKApplication()->getLocale()
#endif

// klocale->translate is much to long
#ifndef i18n
#define i18n(X) KApplication::getKApplication()->getLocale()->translate(X)
#endif

#define Icon(x) kapp->getIconLoader()->loadIcon(x)         
#define ICON(x) kapp->getIconLoader()->loadIcon(x)

class KIconLoader;
class KCharsets;

//#ifndef _KLOCALE_H
#include <klocale.h>
//#endif

#include <drag.h>

#include <qapp.h>
#include <qfile.h>
#include <qpopmenu.h>
#include <qstrlist.h>

#define kapp KApplication::getKApplication()

/** 
* A base class for all KDE applications.
*
* KApplication provides the application with KDE defaults such as
* accelerators, common menu entries, a KConfig object
* etc. KApplication installs a signal handler for the SIGCHLD signal
* in order to avoid zombie children. If you want to catch this signal
* yourself or don't want it to be caught at all, you have set a new
* signal handler (or SIG_IGN) after KApplication's constructor has
* run. 
*
* @short A base class for all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
* @version $Id$
*/ 
class KApplication : public QApplication
{
  Q_OBJECT
public:
  /** 
	* Constructor. Pass command-line arguments. 
	*
	* A KConfig object is
	* created that contains an application-specific config file whose
	* name is "~/." + argv[0] + "rc". This constructor should be considered
	* obsolete. The state of the application-specific config file may be
	* queried afterwards with getConfigState(). 
	*/
  KApplication( int& argc, char** argv );

  /** 
	* Constructor. Pass command-line arguments. 
	*
	* A KConfig object is
	* created that contains an application-specific config file whose
	* name is "~/." + rAppName + "rc". The state of the application-specific 
	* config file may be queried afterwards with getConfigState(). 
	*/
  KApplication( int& argc, char** argv, const QString& rAppName );

  /** 
	* Destructor 
	*/
  virtual ~KApplication();

  /** A global event filter for KApplication.
	* Filters out Ctrl-Alt-F12 for KDebug.
	*/
  virtual bool eventFilter( QObject*, QEvent* );

  /** 
	* Return the current application object.
	*
	* This is similar to the global QApplication pointer qApp. It allows access
	* to the single global KApplication object, since more than one cannot be
	* created in the same application. It saves the trouble of having to pass
	* the pointer to it explicitly to every function that may require it.
	*/
  static KApplication* getKApplication() { return KApp; }

  /** 
	* Return the logical application name as set in the constructor.
	*/
  const QString& appName() const { return aAppName; }
  
  /** 
	* Retrieve the application config object. 
	*
	* @return a pointer to the application's global KConfig object.
	* @see KConfig
	*/
  KConfig* getConfig() const { return pConfig; }

  /** 
	* Retrieve the application session config object. 
	*
	* @return a pointer to the application's instance specific KConfig object.
	* @see KConfig
	*/
  KConfig* getSessionConfig();

  /** 
	* Is the application restored from the session manager? 
	*
	* @return If true, this application was restored by the session manager.
	*	Note that this may mean the config object returned by 
	*	getSessionConfig() contains data saved by a session closedown.
	* @see #getSessionConfig
	*/
  bool isRestored() const { return bIsRestored; }

  /** 
	* Enable session management
	*
	* If userdefined = True then the WmCommand can be defined with setWmCommand.
	* Note that you do not get an instance specific config object with
	* @ref #getSessionConfig in this case!
	*  
	* Session management will apply to the top widget.
	*/
  void enableSessionManagement(bool userdefined = FALSE);

  /** 
	* Set the WmCommand for the session manager.
	*
	* This has an effect if either session management is disabled (then it
	* is used for pseudo session management) or if session management is
	* enabled with userdefined=True.
	* @see #enableSessionManagement
	*/
  void setWmCommand(const char*);
  

  /**
	* Return a standard help menu
	*
	* @param bAboutQtMenu If true, there is a menu entry for About Qt
	* @return a standard help menu
	*/
  QPopupMenu* getHelpMenu( bool bAboutQtMenu, const char* appAboutText );
							 

  /**
    * Get an iconloader for the application. If it does not yet exist,
    * create one.
    * @return a pointer to the Iconloader of the application
    * @see KIconLoader
    */
  KIconLoader* getIconLoader();

  /**
    * Get a KLocale object for the application. If it does not yet exist,
    * create one.
    * @return a pointer to the KLocale object of the application
    * @see KLocale
    */
  KLocale* getLocale();

  /**
    * Get a KCharsets object for the application. 
    * @return a pointer to the KCharsets object of the application
    * @see KCharsets
    */
  KCharsets* getCharsets()const
	{ return pCharsets; }
     
  /**
	* Get the icon for the application.
	* @return a QPixmap with the icon.
	* @see QPixmap
	*/
  QPixmap getIcon() const
	{ return aIconPixmap; }
  
  
  /**
	* Get the mini-icon for the application.
	* @return a QPixmap with the icon.
	* @see QPixmap
	*/
  QPixmap getMiniIcon() const
	{ return aMiniIconPixmap; }


  /** Sets the top widget of the application . This widget will
    * be used for communication with the session manager.
    * You must not call this function manually if you are using
    * the KTopLevelWidget.
    */
  void setTopWidget( QWidget *topWidget );

  QWidget* topWidget() const
    {
      return pTopWidget;
    }

  
  /*obsolete, will disappear (Matthias) */
  void registerTopWidget();
  /* obsolete, will dissappear (Matthias) */
  void unregisterTopWidget();

  /** 
	* Possible return values for getConfigState().
	*
	* @see #getConfigState
	*/
  enum ConfigState { APPCONFIG_NONE, APPCONFIG_READONLY, 
					 APPCONFIG_READWRITE };

  /** 
	* Retrieve the state of the app-config object. 
	*
	* Possible return values
	* are APPCONFIG_NONE (the application-specific config file could not be
	* opened neither read-write nor read-only), APPCONFIG_READONLY (the
	* application-specific config file is opened read-only, but not
	* read-write) and APPCONFIG_READWRITE (the application-specific config
	* file is opened read-write).
	*
	* @see #ConfigState
	*/
  ConfigState getConfigState() const { return eConfigState; }

  /**
	* Invoke the kdehelp HTML help viewer. 
	*
	* @param aFilename	The filename that is to be loaded. Its location
	*			is computed automatically according to the KFSSTND. 
	*			If aFilename is empty, the logical appname with .html 
	*			appended to it is used.
	* @param aTopic		This allows context-sensitive help. Its value
	*			will be appended to the filename, prefixed with
	*			a "#" (hash) character.
	*/
  void invokeHTMLHelp( QString aFilename, QString aTopic ) const;

  /**
   * Returns the directory where KDE stores its HTML documentation
   * 
   * The default for this directory is $KDEDIR/share/doc/HTML
   * @return the name of the directory
   */
  static const QString& kde_htmldir();

  /**
   * Returns the directory where KDE applications store their .kdelnk file
   *
   * The default for this directory is $KDEDIR/share/applnk
   * @return the name of the directory
   */
  static const QString& kde_appsdir();

  /**
   * Returns the directory where KDE icons are stored
   *
   * The default for this directory is $KDEDIR/share/icons
   * @return the name of the directory
   */
  static const QString& kde_icondir();

  /** 	
   * Returns the directory where KDE applications store their specific data
   * 
   * The default for this directory is $KDEDIR/share/apps
   * @return the name of the directory
   */
  static const QString& kde_datadir();

  /**
   * Returns the directory where locale-specific information (like
   * translated on-screen messages are stored
   *
   * The default for this directory is $KDEDIR/share/locale
   * @return the name of the directory
   */
  static const QString& kde_localedir();

  /**
   * Returns the directory where cgi scripts are stored
   *
   * The default for this directory is $KDEDIR/cgi-bin
   * @return the name of the directory
   */
  static const QString& kde_cgidir();

  /**
   * Returns the directory where sound data are stored.
   * This directory is for KDE specific sounds. Sound data of 
   * Applications should go into kde_datadir()
   *
   * The default for this directory is $KDEDIR/share/sounds
   *
   * @return the name of the directory
   */
  static const QString& kde_sounddir();

  /**
   * Returns the directory where toolbar icons are stored
   *
   * The default for this directory is $KDEDIR/share/toolbar
   * @return the name of the directory
   */
  static const QString& kde_toolbardir();

  /**
   * Returns the directory where wallpapers are stored
   *
   * The default for this directory is $KDEDIR/share/wallpapers
   * @return the name of the directory
   */
  static const QString& kde_wallpaperdir();

  /**
   * Returns the directory where executable programs are stored
   *
   * The default for this directory is $KDEDIR/bin
   * @return the name of the directory
   */
  static const QString& kde_bindir();

  /**
   * Returns the directory where KParts are stored
   *
   * The default for this directory is $KDEDIR/parts
   * @return the name of the directory
   */
  static const QString& kde_partsdir();

  /**
   * Returns the directory where config files are stored
   *
   * The default for this directory is $KDEDIR/share/config
   * @return the name of the directory
   */
  static const QString& kde_configdir();

    
 /**
  * Returns the directory where mimetypes are stored
  * 
  * The default for this directory is $KDEDIR/share/mimelnk
  * @return the name of the directory
  */
  static const QString& kde_mimedir();

  /**
   * Get the local KDE base dir
   *
   * This is usually $HOME/.kde
   *
   * @return the name of the directory
   */
  static QString localkdedir();


  /**
	* Get the local KDE config dir
	*
	* This is usually $HOME/.kde/share/config
	*/
  static QString localconfigdir();

  /** 
	* Find a file using standard KDE search paths. 
	*
	* Possible search paths
	* include $KDEDIR, $KDEPATH, and "[KDE Setup]:Path=" entry in a config
	* file. If file is not found, isEmpty() will return True
	*/
  static QString findFile( const char *file );

  /** 
	* Get the KDE font list.
	*
	* This method allows you to get the KDE font 
	* list which was composed by the user with kfontmanager. Usually you should 
	* work only with those fonts in your kapplication. 
	*  
	*  @return true on success.
	*/
  bool getKDEFonts(QStrList *fontlist);


  /**
	* Return a text for the window caption.
	*
	* This would be set either by
	* "-caption", otherwise it will be equivalent to the name of the
	* executable.
	*/
  const char* getCaption() const;

  /** Get a file name in order to make a temporary copy of your
	* document.
	*
	* @param pFilename The full path to the current file opf your
	* document.
	* @return A new filename for auto-saving. You have to free() this
	* yourself, otherwise you have a memory leak!
	*/
  const char* tempSaveName( const char* pFilename );

  /** Check if there is an auto-save file for the document you want to
	* open.
	*
	* @param pFilename The full path to the document you want to open.
	* @param bRecover  This gets set to true if there was a recover
	* file.
	* @return The full path of the file to open. You must free() this
	* pointer yourself, otherwise you have a memory leak.
	*/
  const char* checkRecoverFile( const char* pFilename, bool& bRecover );


  /**
	* Returns true if the KLocale object for this application has already 
	* been constructed
	*
	* @return whether the KLocale object has already been constructed
	*/
  bool localeConstructed() const { return bLocaleConstructed; }
	
  /**
	* An X11 atom used for IPC
	*/
  Atom getDndSelectionAtom() { return DndSelection; }
  /**
	* An X11 atom used for IPC
	*/
  Atom getDndProtocolAtom() { return DndProtocol; }    
  /**
	* An X11 atom used for IPC
	*/
  Atom getDndEnterProtocolAtom() { return DndEnterProtocol; }    
  /**
	* An X11 atom used for IPC
	*/
  Atom getDndLeaveProtocolAtom() { return DndLeaveProtocol; }    
  /**
	* An X11 atom used for IPC
	*/
  Atom getDndRootProtocolAtom() { return DndRootProtocol; }    

  /**
	* Get the X11 display
	*/
  Display *getDisplay() { return display; }
  /**
	* Used by KDNDDropZone to register
	*/
  virtual void addDropZone( KDNDDropZone *_z ) { dropZones.append( _z ); }
  /**
	* Used by KDNDDropZone during shutdown
	*/
  virtual void removeDropZone( KDNDDropZone *_z ) { dropZones.remove( _z ); }
  /**
	* Set the DropZone which reveives root drop events.
	*/
  virtual void setRootDropZone( KDNDDropZone *_z ) { rootDropZone = _z; }

protected:
  /**
	* List of all DropZones.
	*/
  QList<KDNDDropZone> dropZones;

  /** 
	* The last drop zone the mouse was over.
	*
	* If we get a DndLeaveProtocol we must inform 'lastEnteredDropZone'
	* that the mouse left the DropZone.
	*/
  KDNDDropZone *lastEnteredDropZone;

  /**
	* The DropZone which receives root drop events.
	*/
  KDNDDropZone *rootDropZone;

  /**
	* Used to catch X11 events
	*/
  bool x11EventFilter( XEvent * );

  /**
	* Two X11 atoms used for session management
	*/
  Atom WM_SAVE_YOURSELF;
  Atom WM_PROTOCOLS;

  /**
	* X11 atoms used for IPC
	*/
  Atom DndSelection;
  Atom DndProtocol;
  Atom DndEnterProtocol;
  Atom DndLeaveProtocol;
  Atom DndRootProtocol;

  /**
	* The X11 display
	*/
  Display *display;

  Atom KDEChangePalette;
  Atom KDEChangeGeneral;
  Atom KDEChangeStyle;

  /// Current application object.
  static KApplication *KApp;

  /** 
   * Get the KDE base dir. 
   *
   * This is the value of the KDEDIR
   * environment variable if it is set in the process' environment,
   * the compile time default of, if this was not present, either,
   * /usr/local/kde. 
   * @return the KDE base dir
   */
  static QString kdedir();

private slots:
  void appHelpActivated();
  void aboutKDE();
  void aboutApp();
  void aboutQt();

private:
  void* pAppData; // don't touch this without Kalles permission
  KConfig* pConfig; // application config object
  KConfig* pSessionConfig; //instance specific application config object
  QString aSessionName; // logical name of the instance specific config file
  QWidget* pTopWidget;
  QString aAppName; // logical application name
  QString aCaption; // the name for the window title
  QString aWmCommand; // for userdefined session management
  ConfigState eConfigState;
  static QStrList* pSearchPaths;
  KIconLoader* pIconLoader; // the application's own icon loader
  KLocale* pLocale;  
  static KCharsets* pCharsets;  // it shouldn't be static, but you would loose binary compability 
  void* dummy2; // do not use these without asking kalle@kde.org
  void* dummy3;
  void* dummy4;
  QString aAppAboutString; // The text for the about box
  QString aDummyString2; // do not touch
  QString aDummyString3; // do not touch
  QString aDummyString4; // do not touch
  bool bLocaleConstructed; // has the KLocale object already been constructed
  bool bIsRestored; // is the application restored from the session manager?
  bool bSessionManagement; 
  bool bSessionManagementUserDefined;
  QPixmap aIconPixmap;
  QPixmap aMiniIconPixmap;

  void init( );
  void parseCommandLine( int&, char** ); // search for special KDE arguments

  void buildSearchPaths();
  void appendSearchPath( const char *path );

  virtual void kdisplaySetPalette();
  virtual void kdisplaySetStyle();
  virtual void kdisplaySetFont();
  virtual void kdisplaySetStyleAndFont();
  virtual void readSettings();
  void resizeAll();
  virtual void KApplication::applyGUIStyle(GUIStyle newstyle);

public:

  QColor inactiveTitleColor;
  QColor inactiveTextColor;
  QColor activeTitleColor;
  QColor activeTextColor;
  QColor backgroundColor;
  QColor textColor;
  QColor selectColor;
  QColor selectTextColor;
  QColor windowColor;
  QColor windowTextColor;
  int contrast;
  QFont generalFont;
  QFont fixedFont;
  GUIStyle applicationStyle;

  signals:
  /** 
	* KApplication has changed its Palette due to a KDisplay request.
	*
	* Normally, widgets will update their palettes automatically, but you
	* should connect to this to program special behaviour.
	*/
  void kdisplayPaletteChanged();

  /** 
	* KApplication has changed its GUI Style due to a KDisplay request.
	*
	* Normally, widgets will update their styles automatically (as they would
	* respond to an explicit setGUIStyle() call), but you should connect to
	* this to program special behaviour.
	*/
  void kdisplayStyleChanged();

  /**
	* KApplication has changed its Font due to a KDisplay request.
	*
	* Normally widgets will update their fonts automatically, but you should
	* connect to this to monitor global font changes, especially if you are
	* using explicit fonts.
	*/
  void kdisplayFontChanged();

  /** 
	* KApplication has changed either its GUI style, its font or its palette
	* due to a kdisplay request. Normally, widgets will update their styles
	* automatically, but yout should connect to this to program special
	* behavior. */
  void appearanceChanged();

  /** Session management is about to close your application.
	*
	* Connect to this signal in order to save your data. Do NOT
	* manipulate the UI in that slot, it is blocked by kwm.
	*
	* Use the @ref ::getSessionConfig KConfig object to store all
	* your instance specific datas.
	*
	* Note: You should not use this if you are using the KTopLevelWidget.
	*       Overload @ref KTopLevelWidget::saveProperties and 
	*	@ref KTopLevelWidget::readProperties in that case.
	*	This allows you to simply handle applications with multiple
	*	toplevel windows.
	*/
  void saveYourself();
};


#endif
