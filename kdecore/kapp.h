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
#define KDE_VERSION_STRING "Almost 2.0 Alpha"
#define KDE_VERSION_MAJOR 1
#define KDE_VERSION_MINOR 9
#define KDE_VERSION_RELEASE 1

#define Icon(x) kapp->getIconLoader()->loadIcon(x,0,0,false)
#define ICON(x) kapp->getIconLoader()->loadIcon(x,0,0,false)

class KConfig;
class KIconLoader;
class KCharsets;
class KLocale;
class KStyle;

typedef unsigned long Atom; 

#include <qapplication.h>
#include <qpixmap.h>
class QPopupMenu;
class QStrList;
class KDNDDropZone;

#define kapp KApplication::getKApplication()

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the @ref getKApplication method.
*
* This class provides the following services to all KDE applications.
*
* @li It controls the event queue (see @ref QApplication ).
* @li It provides the application with KDE resources such as
* accelerators, common menu entries, a @ref KConfig object. session
* management events, help invocation etc.
* @li Installs a signal handler for the SIGCHLD signal in order to
* avoid zombie children. If you want to catch this signal yourself or
* don't want it to be caught at all, you have set a new signal handler
* (or SIG_IGN) after KApplication's constructor has run.
* @li	Provides a debugging system for all KDE GUI apps. This can be
*	invoked via the key combo Ctrl-Shift-F12 in any KDE application.
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
* @version $Id$
*/
class KApplication : public QApplication
{
  Q_OBJECT
public:
  /**
   * @deprecated
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
	* A KConfig object is created that contains an
	* application-specific config file whose name is "~/." + rAppName
	* + "rc". The state of the application-specific config file may
	* be queried afterwards with getConfigState().
	*/
  KApplication( int& argc, char** argv, const QString& rAppName );

  /** Destructor */
  virtual ~KApplication();

  /** A global event filter for KApplication.
	* Filters out Ctrl-Shift-F12 for KDebug.
	*/
  virtual bool eventFilter( QObject*, QEvent* );

  /**
	* Return the current application object.

	* This is similar to the global QApplication pointer qApp. It
	* allows access to the single global KApplication object, since
	* more than one cannot be created in the same application. It
	* saves the trouble of having to pass the pointer to it explicitly
	* to every function that may require it.
	*/
  static KApplication* getKApplication() { return KApp; }

  /** Return the logical application name as set in the constructor.  */
  QString appName() const { return aAppName; }

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
	* @return A pointer to the application's instance specific
	* 	KConfig object.
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
	* Enable session management.
	* Session management will apply to the top widget.

	* @param userdefined  If this is True, the WmCommand can be
	* defined with @ref #setWmCommand.  Note that you do not get an
	* instance specific config object with @ref #getSessionConfig
	* in this case!
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
  void setWmCommand(const QString&);


  /**
	* Get the standard help menu.
	*
	* @param bAboutQtMenu If true, there is a menu entry for About Qt
	* @return a standard help menu
	*/
  QPopupMenu* getHelpMenu( bool bAboutQtMenu, const QString& appAboutText );
							

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
    * Get character set information.
    * @return a pointer to the KCharsets object of the application
    * @see KCharsets
    */
  KCharsets* getCharsets()const
	{ return pCharsets; }

  /**
	* Get the application icon.
	* @return a QPixmap with the icon.
	* @see QPixmap
	*/
  QPixmap getIcon() const;

  /**
	* Get the mini-icon for the application.
	* @return a QPixmap with the icon.
	* @see QPixmap
	*/
  QPixmap getMiniIcon() const;

  /** Sets the top widget of the application. This widget will
    * be used for communication with the session manager.
    * You must not call this function manually if you are using
    * the KTopLevelWidget.
    */
  void setTopWidget( QWidget *topWidget );

  QWidget* topWidget() const
    {
      return pTopWidget;
    }


  /**
  * @deprecated
  */
  void registerTopWidget();

  /**
  * @deprecated
  */
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
	* @param aFilename	The filename that is to be loaded. Its
	*			location is computed automatically
	*			according to the KFSSTND.  If aFilename
	*			is empty, the logical appname with .html
	*			appended to it is used.
	* @param aTopic		This allows context-sensitive help. Its
	*			value will be appended to the filename,
	*			prefixed with a "#" (hash) character.
	*/
  void invokeHTMLHelp( QString aFilename, QString aTopic ) const;

  /**
   * Returns a KDE style object if a plugin is in use, or else NULL
   * if a Qt internal style is being used.
   *
   */
  KStyle* kstyle() const {return(pKStyle);}
  
  /**
   * Returns the directory where KDE stores its HTML documentation
   *
   * The default for this directory is $KDEDIR/share/doc/HTML
   * @return the name of the directory
   */
  static QString kde_htmldir();

  /**
   * Returns the directory where KDE applications store their .desktop file
   *
   * The default for this directory is $KDEDIR/share/applnk
   * @return the name of the directory
   */
  static QString kde_appsdir();

  /**
   * Returns the directory where KDE icons are stored
   *
   * The default for this directory is $KDEDIR/share/icons
   * @return the name of the directory
   */
  static QString kde_icondir();

  /** 	
   * Returns the directory where KDE applications store their specific data
   *
   * The default for this directory is $KDEDIR/share/apps
   * @return the name of the directory
   */
  static QString kde_datadir();

  /**
   * Returns the directory where locale-specific information (like
   * translated on-screen messages are stored
   *
   * The default for this directory is $KDEDIR/share/locale
   * @return the name of the directory
   */
  static QString kde_localedir();

  /**
   * Returns the directory where cgi scripts are stored
   *
   * The default for this directory is $KDEDIR/cgi-bin
   * @return the name of the directory
   */
  static QString kde_cgidir();

  /**
   * Returns the directory where sound data are stored.
   * This directory is for KDE specific sounds. Sound data of
   * Applications should go into kde_datadir()
   *
   * The default for this directory is $KDEDIR/share/sounds
   *
   * @return the name of the directory
   */
  static QString kde_sounddir();

  /**
   * Returns the directory where toolbar icons are stored
   *
   * The default for this directory is $KDEDIR/share/toolbar
   * @return the name of the directory
   */
  static QString kde_toolbardir();

  /**
   * Returns the directory where wallpapers are stored
   *
   * The default for this directory is $KDEDIR/share/wallpapers
   * @return the name of the directory
   */
  static QString kde_wallpaperdir();

  /**
   * Returns the directory where executable programs are stored
   *
   * The default for this directory is $KDEDIR/bin
   * @return the name of the directory
   */
  static QString kde_bindir();

  /**
   * Returns the directory where KParts are stored
   *
   * The default for this directory is $KDEDIR/parts
   * @return the name of the directory
   */
  static QString kde_partsdir();

  /**
   * Returns the directory where config files are stored
   *
   * The default for this directory is $KDEDIR/share/config
   * @return the name of the directory
   */
  static QString kde_configdir();


 /**
  * Returns the directory where mimetypes are stored
  *
  * The default for this directory is $KDEDIR/share/mimelnk
  * @return the name of the directory
  */
  static QString kde_mimedir();

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
  static QString findFile( const QString& file );

  /**
	* Get the KDE font list.

	* This method allows you to get the KDE font list which was
	* composed by the user with kfontmanager. Usually you should
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
  QString getCaption() const;

  /** Get a file name in order to make a temporary copy of your
	* document.
	*
	* @param pFilename The full path to the current file of your
	* document.
	* @return A new filename for auto-saving. You have to free() this
	* yourself, otherwise you have a memory leak!
	*/
  QString tempSaveName( const QString& pFilename );

  /** Check if there is an auto-save file for the document you want to
	* open.
	*
	* @param pFilename The full path to the document you want to open.
	* @param bRecover  This gets set to true if there was a recover
	* file.
	* @return The full path of the file to open. You must free() this
	* pointer yourself, otherwise you have a memory leak.
	*/
  QString checkRecoverFile( const QString& pFilename, bool& bRecover );

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
  KStyle *pKStyle; // A KDE style object if available (mosfet)
  void* styleHandle; // A KDE style dlopen handle, if used
  
  void init( );
  void parseCommandLine( int&, char** ); // search for special KDE arguments

  void buildSearchPaths();
  void appendSearchPath( const QString& path );

  virtual void kdisplaySetPalette();
  virtual void kdisplaySetStyle();
  virtual void kdisplaySetFont();
  virtual void kdisplaySetStyleAndFont();
  virtual void readSettings();
  void resizeAll();
  virtual void applyGUIStyle(GUIStyle newstyle);

  QColor inactiveTitleColor_;
  QColor inactiveTextColor_;
  QColor activeTitleColor_;
  QColor activeTextColor_;
  int contrast_;
  QFont generalFont_;
  QFont fixedFont_;
  GUIStyle applicationStyle_;

public:

    QColor inactiveTitleColor();
    QColor inactiveTextColor();
    QColor activeTitleColor();
    QColor activeTextColor();
    int contrast();
    QFont generalFont();
    QFont fixedFont();
    GUIStyle applicationStyle();


  /** 
  	@internal
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
  bool checkAccess(const QString& pathname, int mode);

#endif

// $Log$
// Revision 1.73  1999/05/18 00:40:23  steffen
// Implemented lazy getIcon/getMiniIcon. Otherwise the code to defer creation of the iconloader is worthless. We now have 46 less system calls (mainly access() and other filesystem stuff) at startup ;-)
//
// Revision 1.72  1999/05/06 10:37:12  kulow
// I don't know why I spent several days removing headers from kapp.h when
// the next commit adds one ;(
//
// Revision 1.71  1999/05/06 02:50:30  pbrown
// refer to .desktop files not .kdelnk files.
//
// Revision 1.70  1999/05/03 07:02:33  garbanzo
// Might as well bump the version string and numbers to avoid confusion.
//
// Revision 1.69  1999/04/23 13:42:14  mosfet
// KStyle class addition and the routines to allocate it.
//
// Revision 1.68  1999/04/19 15:49:31  kulow
// cleaning up yet some more header files (fixheaders is your friend).
// Adding copy constructor to KPixmap to avoid casting while assingment.
//
// The rest of the fixes in kdelibs and kdebase I will commit silently
//
// Revision 1.67  1999/04/17 19:15:41  kulow
// cleaning up kapp.h, so that only needed headers are included. Guess how
// many files include kapp.h because it includes almost anything they need ;)
// If you find problems after this, please use
//
//   make -k 2>&1 | perl ..../kdesdk/script/fixheaders
//
// And if you find a problem, fixheaders doesn't fix, fix fixheaders
//
// Revision 1.66  1999/04/12 16:47:15  ssk
// Wrote and updated some documentation.
//
// Revision 1.65  1999/03/04 17:49:08  ettrich
// more fixes for Qt-2.0
//
// Revision 1.64  1999/03/02 16:22:18  kulow
// i18n is no longer a macro, but a function defined in klocale.h. So you
// don't need to include kapp.h when you want to use i18n. I see klocale->translate
// as obsolute (actually I seded it all over KDE :)
// I wanted to remove the #include <klocale.h> from kapp.h, but this broke
// too much, so I readded it after fixing half of kdeui. I guess I will
// write a script once that fixed compilation problems (I like the qt20fix way :),
// but for now it's only important to know that i18n works without kapp.h
//
// Revision 1.63  1999/03/02 00:09:41  dfaure
// Fix for ICON() when icon not found. Now returns a default pixmap, unknown.xpm,
// instead of 0L. Will prevent koffice apps and some others from crashing when
// not finding an icon. Approved by Reggie.
//
// loadIcon not changed, since I tried and it broke kpanel (which uses loadIcon
// even on empty string in configuration item). This means loadIcon and ICON are
// no longer equivalent : loadIcon is for apps that want to do complex things
// with icons, based on whether they're installed or not, ICON() is for apps
// that just want an Icon and don't want to care about it !
//
// Of course, unknown.xpm is WAYS to big for a toolbar - that's the point :
// you easily see that the icon is missing....   :)))
//
// Not tested with Qt2.0, of course, but it's time for binary incompat changes...
//
// Revision 1.62  1999/03/01 23:33:11  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.60.2.8  1999/02/23 18:21:01  pbrown
// reverted changes to kapp -- old dnd functionality should work again.
//
// Revision 1.60.2.6  1999/02/22 12:14:11  kulow
// CVS_SILENT: merging 1.1 and 1.2
//
// Revision 1.60.2.5  1999/02/17 17:33:13  kulow
// reverting Alex's patch.
//
// Revision 1.60.2.4  1999/02/17 05:36:42  garbanzo
// i18n() seems to be returning a QString now.  This is a quick hack to get
// around this.  i18n_r() returns the QString, i18n() returns the
// QString.data().  Fully supporting QStrings means using QFiles
// throughout...
//
// Revision 1.60.2.3  1999/02/14 02:05:35  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.60.2.2  1999/01/30 23:55:51  kulow
// more porting - it actually compiles now ;)
//
// Revision 1.60.2.1  1999/01/30 20:18:45  kulow
// start porting to Qt2.0 beta
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
