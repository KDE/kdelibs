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
#define KDE_VERSION 222
#define KDE_VERSION_STRING "2.2.2"
#define KDE_VERSION_MAJOR 2
#define KDE_VERSION_MINOR 2
#define KDE_VERSION_RELEASE 2

class KConfig;
class KCharsets;
class KStyle;
class DCOPClient;
class DCOPObject;

typedef unsigned long Atom;

#include <qapplication.h>
#include <qpixmap.h>
#include <kinstance.h>
#include <kurl.h>
#if QT_VERSION < 300
#include <kdesktopwidget.h>
#endif

struct _IceConn;
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
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
* @version $Id$
*/
class KApplication : public QApplication, public KInstance
{

  Q_OBJECT
public:
  enum CaptionLayout { CaptionAppLast=1, CaptionAppFirst, CaptionNoApp };

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
   * @deprecated
   *
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
    * Add Qt and KDE command line options to KCmdLineArgs.
    */
  static void addCmdLineOptions();

  virtual ~KApplication();

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
   *    @ref KConfig object.
   * @see KConfig
   */
  KConfig* sessionConfig();

  /**
   * Is the application restored from the session manager?
   *
   * @return If @p true, this application was restored by the session manager.
   *    Note that this may mean the config object returned by
   *    @ref sessionConfig() contains data saved by a session closedown.
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
  bool requestShutDown( bool bFast );

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
   *  need to call this function manually when using @ref KMainWindow.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

  /**
   * Invoke the KHelpCenter HTML help viewer from docbook sources.
   *
   * @param anchor      This has to be a defined anchor in your
   *                    docbook sources. If empty the main index
   *                    is loaded
   * @param appname     This allows you to show the help of another
   *                    application. If empty the current name() is
   *                    used
   */
  void invokeHelp( const QString& anchor = QString::null,
                   const QString& appname = QString::null ) const;

  /**
   * @deprecated
   * Invoke the khelpcenter HTML help viewer from HTML sources.
   * Please use invokeHelp() instead.
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
   * Convenience method; invokes the standard email application.
   *
   * @param address The destination address
   * @param subject Subject string. Can be QString::null.
   */
  void invokeMailer( const QString &address, const QString &subject );

  /**
   * Invokes the standard email application.
   *
   * @param mailtoURL A mailto URL.
   */
  void invokeMailer( const KURL &mailtoURL );

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
   * @param startup_id - for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'dcopService' contains the DCOP name under which
   *         this service is available. If empty, the service does
   *         not provide DCOP services.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int startServiceByName( const QString& _name, const QString &URL,
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  static int startServiceByName( const QString& _name, const QStringList &URLs,
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  // KDE 3.0: BCI merge with the above
  // KDE 3.0: Add bool to process events while waiting for response.
  static int startServiceByName( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid=0 );
  static int startServiceByName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid=0 );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param URL - if not empty this URL is passed to the service
   * @param startup_id - for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'dcopService' contains the DCOP name under which
   *         this service is available. If empty, the service does
   *         not provide DCOP services.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int startServiceByDesktopPath( const QString& _name, const QString &URL,
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  static int startServiceByDesktopPath( const QString& _name, const QStringList &URLs,
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  // KDE 3.0: BCI merge with the above
  // KDE 3.0: Add bool to process events while waiting for response.
  static int startServiceByDesktopPath( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid = 0 );
  static int startServiceByDesktopPath( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid = 0 );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param URL - if not empty this URL is passed to the service
   * @param startup_id - for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
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
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  static int startServiceByDesktopName( const QString& _name, const QStringList &URLs,
                QString *error, QCString *dcopService, int *pid, const QCString& startup_id );
  // KDE 3.0: BCI merge with the above
  // KDE 3.0: Add bool to process events while waiting for response.
  static int startServiceByDesktopName( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid = 0 );
  static int startServiceByDesktopName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid = 0 );

  /**
   * Start a program via kdeinit.
   * There will be no application startup notification, so you should
   * use it only for starting internal processes. 
   *
   * program name and arguments are converted to according to the
   * local encoding and passed as is to kdeinit.
   *
   * @param prog Name of the program to start
   * @param args Arguments to pass to the program
   *
   * @return an error code indicating success (== 0) or failure (> 0).
   * @return On success, 'pid' contains the pid of the started process.
   * @return On failure, 'error' contains a description of the error
   *         that occured.
   */
  static int kdeinitExec( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0 );

  /**
   * Start a program via kdeinit and wait for it to finish.
   *
   * Like kdeinitExec but it waits till the program is finished.
   * As such it behaves similair to the system(...) function.
   */
  static int kdeinitExecWait( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0 );

  /**
   * Retrieve the current KDE style object if a plugin is in use,
   *  or else NULL if a Qt internal style is being used.
   *
   */
  KStyle* kstyle() const {return pKStyle;}

  /**
   * @deprecated Don't use.
   */
  bool kdeFonts(QStringList &) const;

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
   * Disable style plugins.
   *
   * Current style plugins do not get unloaded.
   *
   * This is only usefull when used in combination with @ref enableStyles.
   */
  void disableStyles();

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
   * @return A truly unpredictable number in the range [0, RAND_MAX)
   */
  static int random();

  /**
   * Generate a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.
   */
  static QString randomString(int length);

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

#if QT_VERSION < 300
  /**
   * Return a desktop widget that overrides the one in QT.  This is to
   * provide Xinerama support.
   */
  static KDesktopWidget *desktop();
#endif

  /**
   * Returns the app startup notification identifier for this running application.
   */
  const QCString startupId() const;
  
  /**
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows.
   */
  void setStartupId( const QCString& startup_id );

public slots:
  /**
   * Tell KApplication about one more operation that should be finished
   * before the application exits. The standard behaviour is to exit on the
   * "last window closed" event, but some events should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  void ref();

  /**
   * Tell KApplication that one operation such as those described in @ref ref just finished.
   * The application exits if the counter is back to 0.
   */
  void deref();

    /**
    * Returns the argument to --geometry if any, so the geometry can be set
    * wherever necessary
    */
  QString geometryArgument() const;

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

  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)
  
  void read_app_startup_id();

public:
  /**
   * @internal
   */
  bool notify(QObject *receiver, QEvent *event);

  /**
      @internal
    */
  int xioErrhandler();

  /**
   * @internal
   */
  void iceIOErrorHandler( _IceConn *conn );

  /**
   * @internal
   */
  static bool loadedByKdeinit;

  /**
   * @internal
   */
  static void startKdeinit();

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
   * The settings for toolbars have been changed. KToolBar will know what to do.
   */
  void toolbarAppearanceChanged(int);

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
     appliations, simply use KMainWindow. By reimplementing @ref
     KMainWindow::queryClose(), @ref KMainWindow::saveProperties() and
     @ref KMainWindow::readProperties() you can simply handle session
     management for applications with multiple toplevel windows.

     For purposes without KMainWindow, create an instance of
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
   subclassing KApplication. KMainWindow internally makes use of this.

   You don't need to do anything with this class when using
   KMainWindow. Instead, use @ref KMainWindow::saveProperties(),
   @ref KMainWindow::readProperties(), @ref KMainWindow::queryClose(),
   @ref KMainWindow::queryExit() and friends.

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
// Revision 1.227.2.4  2001/09/15 06:31:33  binner
// Is it really CVS which is stupid?
//
// Revision 1.227.2.2  2001/09/07 20:53:09  waba
// KDE 2.2.1
//
// Revision 1.227.2.1  2001/08/22 18:57:52  waba
// Preparing for KDE 2.2.1
//
// Revision 1.227  2001/07/29 06:03:38  waba
// -#define KDE_VERSION_STRING "2.2beta1"
// +#define KDE_VERSION_STRING "2.2"
//
// Revision 1.226  2001/07/20 22:25:10  waba
// Add KDE 3.0 todo items.
//
// Revision 1.225  2001/07/17 01:00:56  waba
// * Single "start kdeinit" function.
// * Show busy cursor to make the wait more pleasant :-]
//
// Revision 1.224  2001/07/08 19:05:59  faure
// Committing patch by Lubos to fix startup notification for the kfmclient case.
// Not that I can really review it, but who can ? :)
//
// Revision 1.223  2001/06/25 20:07:37  waba
// -#define KDE_VERSION_STRING "2.2alpha2"
// +#define KDE_VERSION_STRING "2.2beta1"
//
// Revision 1.222  2001/06/11 19:29:00  malte
// alternate background colour belongs into KGlobalSettings, not KApplication
//
// Revision 1.221  2001/06/11 02:48:26  malte
// Alternate background in listviews
//
// Revision 1.220  2001/06/10 21:37:39  ellis
// Add support for quit logout (i.e., without confirmation)
//
// Revision 1.219  2001/06/06 20:47:50  lunakl
// More work on app startup notification.
//
// Revision 1.218  2001/05/31 09:36:46  mueller
// Xinerama support, based on patch by  Balaji Ramani <balaji@spinnakernet.com>
//
// Revision 1.217  2001/05/17 20:08:40  waba
// KDE 2.2alpha2
//
// Revision 1.216  2001/05/03 00:15:40  mueller
// update version string
//
// Revision 1.215  2001/04/13 01:37:37  waba
// -#define KDE_VERSION_STRING "2.1.9 >= 20010310"
// +#define KDE_VERSION_STRING "2.2alpha1"
//
// Revision 1.214  2001/03/10 15:45:59  faure
// While I'm at it, update the date.
//
// Revision 1.213  2001/03/10 15:36:44  faure
// Removing the code from kdeFonts() and marking it as deprecated
// (just in case some stupid app is still calling it)
//
// Revision 1.212  2001/02/26 13:52:24  dnaber
// document the fact that invokeHTMLHelp() is deprecated
//
// Revision 1.211  2001/02/23 21:26:20  waba
// -#define KDE_VERSION 210
// -#define KDE_VERSION_STRING "2.1"
// +#define KDE_VERSION 220
// +#define KDE_VERSION_STRING "2.1.9 >= 20010223"
//
// Revision 1.210  2001/02/19 11:05:10  faure
// 2.1, and fixing a docu bug found by werner
//
// Revision 1.209  2001/02/16 23:54:27  pfeiffer
// KDoc fixes from Otto Bruggeman <bruggie@home.nl>
//
// Revision 1.208  2001/01/30 21:19:21  coolo
// moved --geometry from qt options to kde options and support it in kmainwindow.
// the qt option only works for setMainWidget calls and is called -geometry not
// --geometry as --help-qt says (the same with every qt option)
//
// Revision 1.207  2001/01/28 14:47:59  mueller
// make HEAD distinguishable from beta2
//
// Revision 1.206  2001/01/22 11:56:47  faure
// 2.1 Beta 2
//
// Revision 1.205  2001/01/08 10:22:38  hausmann
// - made ref() and deref() public slots
//
