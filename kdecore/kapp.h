/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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
#define KDE_VERSION 193
#define KDE_VERSION_STRING "1.91 Beta >= 20000713"
#define KDE_VERSION_MAJOR 1
#define KDE_VERSION_MINOR 9
#define KDE_VERSION_RELEASE 3

class KConfig;
class KCharsets;
class KStyle;
class QTDispatcher;
class DCOPClient;
class DCOPObject;

typedef unsigned long Atom;

#include <qapplication.h>
#include <qpixmap.h>
#include <kinstance.h>

class QPopupMenu;
class QStrList;
class KSessionManaged;
#define kapp KApplication::kApplication()

class KApplicationPrivate;

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the @ref kApplication() method.
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
class KApplication : public QApplication, public KInstance
{
  friend QTDispatcher;

  Q_OBJECT
public:
  enum CaptionLayout { CaptionAppLast=1, CaptionAppFirst, CaptionNoApp };

  /**
   * Constructor. Parses command-line arguments.
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call @ref enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  KApplication(int& argc, char** argv,
              const QCString& rAppName, bool allowStyles=true, bool GUIenabled=true);

  /**
   * This constructor takes aboutData and command line
   *  arguments from @ref KCmdLineArgs.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * it normally runs in the background but under special circumstances
   * displays widgets.  Call @ref enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  KApplication( bool allowStyles=true, bool GUIenabled=true);

  /**
   * Constructor. Parses command-line arguments. Use this constructor to use KApplication
   * in a Motif or Xt program.
   *
   * @param display Will be passed to Qt as the X display. The display must be valid and already
   * opened.
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   *
   * @param allowStyles Set to false to disable the loading on plugin based
   * styles. This is only useful to applications that do not display a GUI
   * normally. If you do create an application with @p allowStyles set to false
   * that normally runs in the background but under special circumstances
   * displays widgets call @ref enableStyles() before displaying any widgets.
   *
   * @param GUIenabled Set to false to disable all GUI stuff. This implies
   * no styles either.
   */
  KApplication(Display *display, int& argc, char** argv, const QCString& rAppName,
               bool allowStyles=true, bool GUIenabled=true);

  /**
    * Add Qt and KDE command line options to KCmdLineArgs.
    */
  static void addCmdLineOptions();

  virtual ~KApplication();

  /** Retrieve the number of command line arguments, i. e. the length
   *  of the array @ref KApplication::argv().
   *
   * In this array, the arguments
   *  handled by @ref KApplication itself (ex., '-icon') are already removed
   */
  int argc() const { return pArgc; }

  /**
   * Retrieve the current application object.
   *
   * This is similar to the global @ref QApplication pointer @ref qApp. It
   * allows access to the single global @ref KApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   */
  static KApplication* kApplication() { return KApp; }

  /**
   * Retrieve the application session config object.
   *
   * @return A pointer to the application's instance specific
   * 	@ref KConfig object.
   * @see KConfig
   */
  KConfig* sessionConfig();

  /**
   * Is the application restored from the session manager?
   *
   * @return If @p true, this application was restored by the session manager.
   *	Note that this may mean the config object returned by
   *	@ref sessionConfig() contains data saved by a session closedown.
   * @see sessionConfig()
   */
  bool isRestored() const { return QApplication::isSessionRestored(); }

  /**
   * Disable session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "startkde" script.
   */
  void disableSessionManagement();

  /**
   * Asks the session manager to shut the session down.
   *
   * Returns TRUE on sucess, FALSE if the session manager could not be
   * contacted.
   */
  bool requestShutDown();

  /**
   * Propagates the network address of the session manager in the
   * SESSION_MANAGER environment variable so that child processes can
   * pick it up.
   *
   * If SESSION_MANAGER isn't defined yet, the address is searched in
   * $HOME/.KSMserver.
   *
   * This function is called by clients that are started outside the
   * session ( i.e. before ksmserver is started), but want to launch
   * other processes that should participate in the session.  Examples
   * are kdesktop or kicker.
   */
    void propagateSessionManager();

    /*
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with @ref KSessionManaged.
     * @internal
     */
  void commitData( QSessionManager& sm );

    /*
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManaged.
     * @internal
     */
  void saveState( QSessionManager& sm );

  /**
   * Retrieve a pointer to a @ref DCOPClient for the application.
   */
  virtual DCOPClient *dcopClient();

  /**
   * Get the application icon.
   * @return a @ref QPixmap with the icon.
   * @see QPixmap
   */
  QPixmap icon() const;

  /**
   * Get the mini-icon for the application.
   * @return a @ref QPixmap with the icon.
   * @see QPixmap
	*/
  QPixmap miniIcon() const;

  /**
   *   Set the top widget of the application.
   *
   *   @param topWidget A top widget of the application.
   *
   *  This means basically applying the right window caption and
   *  icon. An application may have several top widgets. You don't
   *  need to call this function manually when using @ref KTMainWindow.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

  /**
   * Invoke the KHelpCenter HTML help viewer from docbook sources.
   *
   * @param anchor	This has to be a defined anchor in your
   *                    docbook sources. If empty the main index
   *                    is loaded
   * @param appname	This allows you to show the help of another
   *                    application. If empty the current name() is
   *                    used
   */
  void invokeHelp( const QString& anchor = QString::null,
		   const QString& appname = QString::null ) const;

  /**
   * Invoke the khelpcenter HTML help viewer from HTML sources.
   *
   * @param aFilename  The filename that is to be loaded. Its
   *                   location is computed automatically
   *                   according to the KFSSTND.  If @p aFilename
   *                   is empty, the logical appname with .html
   *                   appended to it is used.
   * @param aTopic     This allows context-sensitive help. Its
   *                   value will be appended to the filename,
   *                   prefixed with a "#" (hash) character.
   */
  void invokeHTMLHelp( const QString& aFilename, const QString& aTopic = QString::null ) const;

  /**
   * Invoke the standard email application.
   *
   * @param address The destination address
   * @param subject Subject string. Can be QString::null.
   */
  void invokeMailer( const QString &address, const QString &subject );

  /**
   * Invoke the standard browser.
   *
   * @param url The destination address
   */
  void invokeBrowser( const QString &url );

  /**
   * Get the DCOP name of the service launcher. This will be something like
   * klaucher_$host_$uid.
   * @return The DCOP name of the launcher.
   */
  static QCString launcher();

  /**
   * Get the full path of the LD_PRELOAD library which sets the
   * _NET_WM_PID property. (used for app start notification)
   */
  static QString libmapnotify();

  /**
   * The way a service gets started depends on the 'X-DCOP-ServiceType'
   * entry in the desktop file of the service:
   *
   * There are three possibilities:
   * @li X-DCOP-ServiceType=None (default)
   *    Always start a new service,
   *    don't wait till the service registers with dcop.
   * @li X-DCOP-ServiceType=Multi
   *    Always start a new service,
   *    wait until the service has registered with dcop.
   * @li X-DCOP-ServiceType=Unique
   *    Only start the service if it isn't already running,
   *    wait until the service has registered with dcop.
   */

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param URL - if not empty this URL is passed to the service
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'dcopService' contains the DCOP name under which
   *         this service is available. If empty, the service does
   *         not provide DCOP services.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int startServiceByName( const QString& _name, const QString &URL,
		QString *error=0, QCString *dcopService=0, int *pid=0 );
  static int startServiceByName( const QString& _name, const QStringList &URLs=QStringList(),
		QString *error=0, QCString *dcopService=0, int *pid=0 );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param URL - if not empty this URL is passed to the service
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'dcopService' contains the DCOP name under which
   *         this service is available. If empty, the service does
   *         not provide DCOP services.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int startServiceByDesktopPath( const QString& _name, const QString &URL,
		QString *error=0, QCString *dcopService=0, int *pid = 0 );
  static int startServiceByDesktopPath( const QString& _name, const QStringList &URLs=QStringList(),
		QString *error=0, QCString *dcopService=0, int *pid = 0 );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param URL - if not empty this URL is passed to the service
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'dcopService' contains the DCOP name under which
   *         this service is available. If empty, the service does
   *         not provide DCOP services. iff a process was started, 'pid'
   *         contains its pid.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int startServiceByDesktopName( const QString& _name, const QString &URL,
		QString *error=0, QCString *dcopService=0, int *pid = 0 );
  static int startServiceByDesktopName( const QString& _name, const QStringList &URLs=QStringList(),
  		QString *error=0, QCString *dcopService=0, int *pid = 0 );

  /**
   * Retrieve the current KDE style object if a plugin is in use,
   *  or else NULL if a Qt internal style is being used.
   *
   */
  KStyle* kstyle() const {return pKStyle;}

  /**
   * Get the KDE font list.
   *
   * This method allows you to get the KDE font list which was
   * composed by the user with kfontmanager. Usually you should
   * work only with those fonts in your KDE application.
   *
   * @param fontlist A reference to a list to fill with fonts.
   * @return true on success.
   */
  bool kdeFonts(QStringList &fontlist) const;

  /**
   * Return a text for the window caption.
   *
   * This may be set by
   * "-caption", otherwise it will be equivalent to the name of the
   * executable.
   */
  QString caption() const;

  /**
   * Build a caption that contains the application name along with the
   * @ref userCaption() using a standard layout.
   *
   * To make a compliant caption
   * for your window, simply do: @p setCaption(kapp->makeStdCaption(yourCaption));
   *
   * @param userCaption The caption string you want to display in the
   * window caption area. Do not include the application name!
   * @param withAppName Indicates that the method shall include or ignore
   * the application name when making the caption string. You are not
   * compliant if you set this to @p false.
   * @param modified If true, a 'modified' sign will be included in the
   * returned string. This is useful when indicating that a file is
   * modified, i.e., it contains data that has not been saved.
   */
  QString makeStdCaption( const QString &userCaption,
			  bool withAppName=true, bool modified=false ) const;

  /**
   * Get a file name in order to make a temporary copy of your document.
   *
   * @param pFilename The full path to the current file of your
   * document.
   * @return A new filename for auto-saving.
   */
  QString tempSaveName( const QString& pFilename ) const;

  /**
   * Check whether  an auto-save file exists for the document you want to
   * open.
   *
   * @param pFilename The full path to the document you want to open.
   * @param bRecover  This gets set to true if there was a recover
   * file.
   * @return The full path of the file to open.
   */
  QString checkRecoverFile( const QString& pFilename, bool& bRecover ) const;

  /**
   * Get the X11 display
   */
  Display *getDisplay() { return display; }

  /**
   * Enable style plugins.
   *
   * This is useful only to applications that normally
   * do not display a GUI and create the @ref KApplication with
   *  @p allowStyles set to @p false.
   */
  void enableStyles();



  /**
   *  Install widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard @ref QWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   **/
  void installX11EventFilter( QWidget* filter );

  /**
   * Generate a uniform random number.
   * @return A truly unpredictable number in the range [0, MAX_RAND)
   */
  static int random();

  /**
   * Generate a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.  If negative, the
   *               length will be random as well (from 0 to MAX_RAND)
   */
  static QString randomString(int length=-1);

  /**
   * Add a message type to the KIPC event mask. You can only add "system
   * messages" to the event mask. These are the messages with id < 32.
   * Messages with id >= 32 are user messages.
   * @param id The message id. See @ref #KIPC::Message.
   */
  void addKipcEventMask(int id);

  /**
   * Remove a message type from the KIPC event mask. This message will not
   * be handled anymore.
   * @param id The message id.
   */
  void removeKipcEventMask(int id);

  /**
   * Tell KApplication about one more operation that should be finished
   * before the application exits. The standard behaviour is to exit on the
   * "last window closed" event, but some events should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  void ref();

  /**
   * Tell KApplication that one operation such as those described in @ref just finished.
   * The application exits if the counter is back to 0;
   */
  void deref();

protected:
  /**
   * Used to catch X11 events
   */
  bool x11EventFilter( XEvent * );


  Display *display;
  Atom kipcCommAtom;
  int kipcEventMask;

  /// Current application object.
  static KApplication *KApp;
  int pArgc;

private slots:
  void dcopFailure(const QString &);
  void dcopBlockUserInput( bool );
  void x11FilterDestroyed();
  void kstyleDestroyed();

private:
  KApplicationPrivate* d;
  KConfig* pSessionConfig; //instance specific application config object
  DCOPClient *pDCOPClient; // instance specific application communication client
  QString aCaption; // the name for the window title
  bool bSessionManagement;
  QPixmap aIconPixmap;
  QPixmap aMiniIconPixmap;
  KStyle *pKStyle; // A KDE style object if available (mosfet)
  bool useStyles;
  void* styleHandle; // A KDE style dlopen handle, if used
  QWidget *smw;

  void init( bool GUIenabled );

  void parseCommandLine( int&, char** ); // search for special KDE arguments
  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)

public:
  /**
      @internal
    */
  int xioErrhandler();

  /**
   * @internal
   */
  static bool loadedByKdeinit;

  /**
   * Valid values for the settingsChanged signal
   */
  enum SettingsCategory { SETTINGS_MOUSE, SETTINGS_COMPLETION, SETTINGS_PATHS,
         SETTINGS_POPUPMENU, SETTINGS_QT };

  signals:
  /**
   * KApplication has changed its palette due to a KDisplay request.
   *
   * Normally, widgets will update their palettes automatically, but you
   * should connect to this to program special behaviour.
   */
  void kdisplayPaletteChanged();

  /**
   * KApplication has changed its GUI style in response to a @ref KDisplay request.
   *
   * Normally, widgets will update their styles automatically (as they would
   * respond to an explicit setGUIStyle() call), but you should connect to
   * this to program special behaviour.
   */
  void kdisplayStyleChanged();

  /**
   * KApplication has changed its font in response to a @ref KDisplay request.
   *
   * Normally widgets will update their fonts automatically, but you should
   * connect to this to monitor global font changes, especially if you are
   * using explicit fonts.
   */
  void kdisplayFontChanged();

  /**
   * @ref KApplication has changed either its GUI style, its font or its palette
   * in response to a kdisplay request. Normally, widgets will update their styles
   * automatically, but you should connect to this to program special
   * behavior. */
  void appearanceChanged();

  /**
   * The desktop background has been changed by @p kcmdisplay.
   *
   * @param desk The desktop whose background has changed.
   */
  void backgroundChanged(int desk);

  /**
   * The global settings have been changed - see KGlobalSettings
   * KApplication takes care of calling reparseConfiguration on KGlobal::config()
   * so that applications/classes using this only have to re-read the configuration
   * @param category the category among the enum above
   */
  void settingsChanged(int category);

  /**
   * The global icon settings have been changed.
   */
  void iconChanged(int group);

  /**
   * Emitted when a KIPC user message has been received.
   */
  void kipcMessage(int id, int data);

  /**
      Session management asks you to save the state of your application.

     This signal is provided for compatibility only. For new
     appliations, simply use KTMainWindow. By reimplementing @ref
     KTMainWindow::queryClose(), @ref KTMainWindow::saveProperties() and
     @ref KTMainWindow::readProperties() you can simply handle session
     management for applications with multiple toplevel windows.

     For purposes without KTMainWindow, create an instance of
     KSessionManaged and reimplement the functions @ref
     KSessionManaged::commitData() and/or @ref
     KSessionManaged::saveState()

     If you still want to use this signal, here is what you should do:

     Connect to this signal in order to save your data. Do NOT
     manipulate the UI in that slot, it is blocked by the session
     manager.

     Use the @ref #sessionConfig() KConfig object to store all your
     instance specific datas.

     Do not do any closing at this point! The user may still select
     Cancel  wanting to continue working with your
     application. Cleanups could be done after shutDown() (see
     the following).

  */
  void saveYourself();

  /** Your application is killed. Either by your program itself,
      @p xkill or (the usual case) by KDE's logout.

      The signal is particularly useful if your application has to do some
      last-second cleanups. Note that no user interaction is possible at
      this state.
   */
  void shutDown();


private:
  void propagateSettings(SettingsCategory category);
  void kdisplaySetPalette();
  void kdisplaySetStyle();
  void kdisplaySetFont();
  void applyGUIStyle(GUIStyle);

  int captionLayout;

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


class KSessionManagedPrivate;

/**
   Provides highlevel aceess to session management on a per-object
   base.

   KSessionManaged makes it possible to provide implementations for
   @ref QApplication::commitData() and @ref QApplication::saveState(), without
   subclassing KApplication. KTMainWindow internally makes use of this.

   You don't need to do anything with this class when using
   KTMainWindow. Instead, use @ref KTMainWindow::saveProperties(),
   @ref KTMainWindow::readProperties(), @ref KTMainWindow::queryClose(),
   @ref KTMainWindow::queryExit() and friends.

  @short Highlevel access to session management.
  @author Matthias Ettrich <ettrich@kde.org>
 */
class KSessionManaged
{
public:
  KSessionManaged();
  virtual ~KSessionManaged();

    /**
       See @ref QApplication::saveState() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.
     */
  virtual bool saveState( QSessionManager& sm );
    /**
       See @ref QApplication::commitData() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.
     */
  virtual bool commitData( QSessionManager& sm );

private:
  KSessionManagedPrivate *d;
};


#endif

// $Log$
// Revision 1.174  2000/07/13 16:08:49  coolo
// increased date. I've seen bug reports >= 20000708 and I still have 0629 - bah :)
//
// Revision 1.173  2000/07/02 11:45:51  jansen
// Adding KApplication::libmapnotify()
//
// Revision 1.172  2000/06/29 07:51:07  coolo
// the after wednesday version
//
// Revision 1.171  2000/06/25 17:09:26  pfeiffer
// #define KDE_VERSION 193
// due to all the doc-updates you have to recompile everything anyway
//
// Revision 1.170  2000/06/25 16:51:37  dsweet
// Doc edits.
//
// Revision 1.169  2000/06/24 15:24:29  faure
// Implemented Waldo's idea. KApplication holds a reference counter:
// * This counter indicates when to exit the application.
// * It starts at 1, is decremented by the "last window close" event, but
// * is incremented by operations that should outlive the last window closed
// * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
//
// This fixes "konqueror aborts a save as operation when closing the last window"
// (#4561, from Waldo as well :-) and could be useful to e.g. kmail.
//
// Note: QApplication's lastWindowClosed signal breaks this. BUT KTMainWindow
// was already doing the app->quit on last window, so it was already useless.
// Do NOT use QApplication's lastWindowClosed.
//
// The change is binary compatible, long live d pointers.
//
// Revision 1.168  2000/06/24 00:35:18  dsweet
// Editing header file documentation.
//
// Revision 1.167  2000/06/21 20:23:09  molnarc
//
// bumbed date up to 20000621 since everyone gets to recompile anyway!
//
// Revision 1.166  2000/06/21 17:33:57  putzer
//
// - cleaned up some more cruft
//
// Revision 1.165  2000/06/21 11:05:13  putzer
//
// - moved KApplication::mimeSourceFactory to KInstance
// - cleanup in kapp.*: I removed the old (#if 0) helpmenu stuff and renamed
//   pAppData to d, as this name is used everywhere else.
//
// Revision 1.164  2000/06/15 08:54:53  putzer
// - added KMimeSourceFactory
// - automatically set KMimeSourceFactory as the defaultFactory in KApplication
//
// Revision 1.163  2000/06/14 09:43:01  faure
// Bumped version-string's date, for Chris and for bug reports.
//
// Revision 1.162  2000/06/13 04:13:01  waba
// WABA: Keep track of whether we are a DSO loaded by kdeinit or not.
//
// Revision 1.161  2000/06/08 06:00:51  hoelzer
// Adjusted versions.
//
// Revision 1.160  2000/06/06 22:53:35  gehrmab
// Beauty and wellness for the API documentation
//
// Revision 1.159  2000/06/03 23:19:28  gehrmab
// For consistency, moved contrast() from KApplication to KGlobalSettings, like the colors. Also, some Qt settings like the double click interval are propagated to QApplication now
//
// Revision 1.158  2000/06/03 01:18:17  gehrmab
// KGlobal <-> KGlobalSettings <-> KApplication cleanup
//
// Revision 1.157  2000/05/31 21:40:26  ettrich
//
// Make dcop calls smarter: no more hanging user interfaces
//
// From the documentation:
//
//    * A call blocks the application until the process receives the
//    * answer, for a maximum of 1/10 of a second. If the call was not
//    * answered by then, the client opens a local event loop in order to
//    * keep the user interface updated (by processing paint events and
//    * such) until the answer message finally drops in.
//
// Revision 1.156  2000/05/30 20:53:17  coolo
// >= 20000531 (at least in Moscwa :)
//
// Revision 1.155  2000/05/24 08:24:35  kulow
// fixes kdelibs and kdebase, the rest has to work with grep invokeHTMLHelp :)
//
// Revision 1.154  2000/05/24 08:18:17  kulow
// adding default parameter for topic
//
// Revision 1.153  2000/05/24 07:51:57  kulow
// renamed invokeHTMLHelp to invokeHTMLHelp_x to find misuses. This function
// is declared deprecated and invokeHelp should be used, which takes two
// completly different arguments: docbook anchor and alternative appname
//
// Revision 1.152  2000/05/21 23:09:03  pfeiffer
// fix against people calling QApplication::setStyle()
// I.e. Magellan doesn't crash on startup anymore.
//
// Revision 1.151  2000/05/17 09:30:12  faure
// Updated date in version number - since you have to recompile all of
// KDE anyway :/
//
// Revision 1.150  2000/05/11 19:24:21  coolo
// including a date in the version string so we have an idea what it is
// when bug reports come in
//
// Revision 1.149  2000/05/09 22:50:56  mueller
// typo fix
//
// Revision 1.148  2000/05/09 22:31:30  granroth
// #define KDE_VERSION_STRING "1.90 Beta (Konfucious)"
//
// Revision 1.147  2000/05/09 21:44:46  coolo
// is that ok?
// -#define KDE_VERSION_STRING "Almost 2.0 Alpha"
// +#define KDE_VERSION_STRING "Almost 2.0 - Beta1"
// -#define KDE_VERSION_RELEASE 1
// +#define KDE_VERSION_RELEASE 2
//
// Revision 1.146  2000/05/08 01:16:46  waba
// WABA: Explain the X-DCOP-ServiceType fields in desktop files.
//
// Revision 1.145  2000/05/01 23:32:43  waba
// WABA: startServiceBy... now takes a list of URLs as argument as well.
// Rearranged the other parameters in a way that makes more sense.
//
// Revision 1.144  2000/04/27 10:48:30  elter
// Add a global setting for popupmenu tear-off handles.
// A tear-off handle is a special menu item, that - when selected - creates a copy of the menu. This "torn off" copy lives in a separate window. It contains the same choices as the original menu, with the exception of the tear-off handle.
//
// Binary compatible.
//
// Revision 1.143  2000/04/21 16:04:12  jansen
// Adding a signal to KApplication for notification of icon changes.
//
// Revision 1.142  2000/04/13 19:14:34  sschiman
// Added KApp constructor to pass a X display handle to Qt.
//
// Revision 1.141  2000/03/25 20:08:00  charles
// Why have we had a random() function, but not a randomString function?
// What gives?!
// Well. Ok. we have one now. muahaha!
//
// Revision 1.140  2000/03/24 22:18:59  ettrich
// session management fixes
//
// Revision 1.139  2000/03/19 19:16:01  faure
// Let the thing be called SettingsChanged, and defined an enum for each
// type of settings. Currently only SETTINGS_MOUSE is used, but feel free
// to add support for SETTINGS_COMPLETION in both kcontrol and the relevant
// classes. One day SETTINGS_PATHS will be supported too (needs a UI first).
//
// Revision 1.138  2000/03/19 13:33:06  jansen
// * Improved KIPC. There are 2 message types now: system messages and user
//   messages. There is an event mask related to the system messages so they
//   can be blocked. The system messages invoke a certain action inside
//   KApplication, the user messages emit KApplication::kipcMessage(id, arg).
//
// * Cleaned up the change message handlers. The "ChangeGeneral" is gone and
//   has been renamed to FontChanged. Ever wondered why there was so much
//   flicker when changing the font? Previously, kapp would emit 1 style
//   changed, 1 font changed, 2 palette changed and 4 (!) appearance changed
//   singals on _one_ font change request...
//
// David, I suggested the name "IconviewChanged" for the iconview changes in
// this patch. There's a notice in KApplication::x11EventFilter() for it.
//
// Revision 1.137  2000/03/17 00:16:45  faure
// Added atom for KDEChangeSettings, thanks Geert for the very precious tips.
// Added signal in kapp.h : settingsChanged(), used by K*Views
// (TODO: same for other widgets that use the settings in there)
//
// Revision 1.136  2000/03/13 13:29:49  jansen
// Fixes for running nonlocal applications. I changed two things:
// 1. klauncher registers as "klaucher_$host_$uid" now. This name is available
//    trough KApplication::launcher() (static)
// 2. The socketname of kdeinit now includes the display as there can be
//    multiple kdeinit's per user on a certain host.
//
// Revision 1.135  2000/02/26 13:53:10  waba
// WABA: Added some examples to startServiceBy....()
//
// Revision 1.134  2000/02/23 17:11:12  waba
// WABA: Moved KService::startServiceByXXX to KApplication.
//
// Revision 1.133  2000/01/12 02:39:35  dsweet
// doc cleaning
//
// Revision 1.132  2000/01/09 23:01:22  waba
// WABA: AboutData is taken from KCmdLineArgs.
//
// Revision 1.131  2000/01/09 19:18:30  dsweet
// Spiffed up documentation.
//
// Revision 1.129  2000/01/07 21:50:29  waba
// WABA: KApplication can now make use of KCmdLineArgs. (Optional)
//
// Revision 1.128  2000/01/03 15:52:25  waba
// WABA: Allow to make KApplications without a GUI which saves
// you a wopping 80Kb. Not that we have a lot of such applications
// around.
//
// Revision 1.127  1999/12/21 17:39:06  faure
// Here comes the new kDebug. Merged into kdebug.h, but keeping support
// for kdebug(), for compatibility. Also fixed it to work without a KInstance,
// just in case.
// The implementation looks a bit ugly (va_start/va_end for each API call,
// but I didn't find another way. Nice API but heavy code for it).
// Removed kdebug-specific event filter from KApplication.
// kdebugdialog moved to kdebase, as a separate app.
//
// Revision 1.126  1999/12/10 16:47:32  waba
// WABA: Adding KApplication::random() for real random numbers.
//
// Revision 1.125  1999/12/05 13:48:17  coolo
// removing useless ::config()
//
// Revision 1.124  1999/11/26 17:08:10  waba
// WABA: KUniqueApp always forks, added static KUniqueApp::start()
//
// Revision 1.123  1999/11/25 13:25:46  waba
// WABA: Reverted last change
//
// Revision 1.121  1999/11/22 12:33:31  mkoch
// docu
//
// Revision 1.120  1999/11/07 01:40:14  ettrich
// some updates, new kwin access to window manager functionality
//
// Revision 1.119  1999/11/03 03:14:00  ettrich
//
// session management update. New docs, new functions, more fun.
//
// Revision 1.118  1999/10/30 09:46:16  dfaure
// Documented rAppName in kapp.h
// Added fourth argument to kuniqueapp, which has now exactly the same args as kapp.
//
// Revision 1.117  1999/10/23 16:16:23  kulow
// here comes KInstance - "KApplication light"
// It's a new KLibGlobal and KGlobal only keeps a pointer to a global
// instance. KApplication is derived from KInstance - making it possible
// to move things out of KApplication into KInstance to make Components
// easier.
// Needs some testings and I bet some tweaks here and there :)
//
// Revision 1.116  1999/10/22 21:08:44  mosfet
// Added an additional bool parameter for disabling of plugin styles. Intended
// only for apps that link to kdecore but don't display a GUI. For apps that
// usually don't use a GUI but may on error see enableStyles().
//
// Revision 1.115  1999/10/21 22:20:09  espensa
// helpMenu(), aboutKDE(), aboutApp() and appHelpActivated() has moved to
// ktmainwindow (kdeui). This will break a lot of code. KDE2PORTING has
// been updated. NOTE: I have left the old code on kapp for now (ifdef'ed
// away) in case it is really needed. It will be removed soon so instead
// of using it, you should convert the code that depends on it.
//
// Revision 1.114  1999/10/20 14:30:11  pbrown
// kuniqueapp moved to its own file.
//
// Revision 1.113  1999/10/19 18:51:09  pbrown
// KUniqueApplication class utilizing KApplication and a DCOPObject is
// available.  It works. :)
//
// Revision 1.112  1999/10/19 10:55:00  jansen
// Added "desk" argument to signal backgroundChanged().
//
// Revision 1.111  1999/10/18 19:25:16  jansen
// 1. Added KIPC class: "Old style" IPC for KDE applications.
// 2. Added backgroundChanged() signal to KApplication.
//
// Revision 1.110  1999/10/15 15:06:51  pbrown
// slot for DCOP communication faillures.
//
// Revision 1.109  1999/10/13 15:13:09  ettrich
// the appname is programmers space and therefore QCString
//
// Revision 1.108  1999/10/13 02:39:02  boloni
// fix it to compile. (dcopclient can not be declared const)
//
// Revision 1.107  1999/10/10 22:02:50  pbrown
// require libDCOP, set up DCOPClient in kapp constructor.
//
// Revision 1.106  1999/10/09 00:08:27  kalle
// The dreaded library cleanup: getConfig() -> config() and friends (see separate mail)
//
// Revision 1.105  1999/10/08 22:49:15  bero
// - Replace KTopLevelWidget with KTMainWindow where it's still used
// - Disable ktopwidget.h
// - Remove ktopwidget stuff from libs
//
// Revision 1.104  1999/10/05 21:52:02  espensa
// Added invokeMailer() and invokeBrowser(). I have hardcoded kmail
// and kfmclient for now.
//
// Revision 1.103  1999/09/21 10:34:50  waba
// WABA: Made destructor of KSessionManaged virtual.
//
// Revision 1.102  1999/09/20 18:22:31  espensa
// Here comes the KApplication::makeStdCaption() addition.
//
// Usage: setCaption(kapp->makeStdCaption(yourCaptionWithNOAppName));
//
// The current layout of the returned caption is:
// "yourCaptionWithNOAppName - <appName>"
// (without the "<" and ">" of course ;)
//
// Revision 1.101  1999/09/20 13:11:26  ettrich
//    ported to new session management
//    usable private data class (use pAppData)
//
// Revision 1.100  1999/08/06 18:58:45  gehrmab
//
// * KApplication::argc()
// * Fixed documentation of tempSaveName()
//
// Revision 1.99  1999/08/03 22:56:28  ettrich
// removed some debug output (it was impossible to work with this),
// some smaller cleanups, reintroduced support for local colos/fonts
// in addition to styles. Will soon be property based.
//
// Revision 1.98  1999/08/03 20:21:32  kulow
// removing the last kde_ function and kdedir() with it  ;)
//
// Revision 1.97  1999/07/23 17:07:15  steffen
// be QTDispatchers friend
//
// Revision 1.96  1999/07/22 10:01:25  kulow
// if drag.h is obsolete, kde_datadir is for long :)
//
// Revision 1.95  1999/07/20 00:47:45  pbrown
// removed kdnd stuff from kapp.
//
// Revision 1.94  1999/06/23 19:13:39  knoll
// removed xxxFont() methods. Use KGlobal::xxxFont() instead.
//
// Revision 1.93  1999/06/20 09:41:48  kulow
// big patch:
// removed const QString as return values
// marked some const functions const
// made FALSE and TRUE consistent with the rest (besides that I still
//   believe that passing TRUE is an implicit cast - it may be optimized
//   away, but it's still there somehow ;)
// removing some more functions from kapp.h (registerTopWidget, etc)
//
// BTW: What applications use kclipboard?
//
// Revision 1.92  1999/06/19 21:16:26  kulow
// found the reason why KConfig parses all files twice at startup.
// KApplication does reparseConfig right after creating the config
// object (through KGlobal). Fixed in adding a bool reparse parameter
// to readSettings
//
