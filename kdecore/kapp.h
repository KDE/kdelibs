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

#ifndef _KAPP_H
#define _KAPP_H

// Version macros. Never put this further down.
#define KDE_VERSION_STRING "1.1.2"
#define KDE_VERSION_MAJOR 1
#define KDE_VERSION_MINOR 1
#define KDE_VERSION_RELEASE 5

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

#include <qapplication.h>
#include <qfile.h>
#include <qpopupmenu.h>
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
	* @param bAboutQtMenu not used anymore
	* @param appAboutText a little text about the application
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
  /* obsolete, will disappear (Matthias) */
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
	* @param pFilename The full path to the current file of your
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
	* Set the DropZone which receives root drop events.
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
  static KCharsets* pCharsets;  // it shouldn't be static, but you would loose binary compatibility 
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
  virtual void applyGUIStyle(GUIStyle newstyle);

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
  
  /** for internal purposes only
    */
  int xioErrhandler();

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
	* automatically, but you should connect to this to program special
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
	* Do not do any closing at this point! The user may still
	* select "cancel" and then he wants to continue working with
	* your application. Cleanups could be done after shutDown()
	* (see below)
	*
	* Note: You should not use this if you are using the KTopLevelWidget.
	*       Overload @ref KTopLevelWidget::saveProperties and 
	*	@ref KTopLevelWidget::readProperties in that case.
	*	This allows you to simply handle applications with multiple
	* toplevel windows.  */
  void saveYourself();

  /** Your application is killed. Either by kwm's killwindow function,
	* xkill or (the usual case) by KDE's logout.  
	*/
  void shutDown();


private:
  
  KApplication(const KApplication&);
  KApplication& operator=(const KApplication&);
};

  /** Check, if a file may be accessed in a given mode.
        * This is a wrapper around the access() system call.
        * checkAccess() calls access() with the given parameters.
        * If this is OK, checkAccess() returns true. If not, and W_OK
        * is part of mode, it is checked if there is write access to
        * the directory. If yes, checkAccess() returns true.
        * In all other cases checkAccess() returns false.
        *
        * Other than access() this function EXPLICITELY ignores non-existant
        * files if checking for write access.
        *
        * @param pathname The full path of the file you want to test
        * @param mode     The access mode, as in the access() system call.
        * @return Whether the access is allowed, true = Access allowed
        */
  bool checkAccess(const char *pathname, int mode);

#endif

// $Log$
// Revision 1.61.2.2  1999/04/17 22:12:18  kulow
// setting RELEASE to 4
//
// Revision 1.61.2.1  1999/04/10 15:13:41  dfaure
// in getHelpMenu(), the first param, bAboutQtMenu, doesn't do anything
// anymore -> fixed docu.
//
// Revision 1.61  1999/02/06 16:27:11  kulow
// make KDE "1.1" and 1 - 1 - 3 (pre1 and 2 was 1 - 1 - 1). Then again
// 1.1.1 has to be 1 - 1 - 4. Would be strange, not? I see no other way around.
//
// Revision 1.60  1998/12/06 08:56:10  garbanzo
// Typo police.
//
// Revision 1.59  1998/12/06 08:43:47  garbanzo
// Oh hell, it looks ok at the bottom.  I hope nobody gets their tighty
// whitites in a knot.
//
