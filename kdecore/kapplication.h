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
#include "kdeversion.h"

class KConfig;
class KCharsets;
class DCOPClient;
class DCOPObject;

typedef unsigned long Atom;
#ifdef Q_WS_QWS
typedef void Display;
#endif

#include <qapplication.h>
#include <qpixmap.h>
#include <kinstance.h>

struct _IceConn;
class QPopupMenu;
class QStrList;
class KSessionManaged;
class KStyle;
class KURL;

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
* @li It can start new services
*
*
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

#ifndef Q_WS_QWS
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
#endif

  /**
   * @deprecated
   * // REMOVE FOR KDE 4.0 - using it only gives crashing applications because
   * // KCmdLineArgs::init isn't called
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
   * Returns the current application object.
   *
   * This is similar to the global @ref QApplication pointer qApp. It
   * allows access to the single global KApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   * @return the current application object
   */
  static KApplication* kApplication() { return KApp; }

  /**
   * Returns the application session config object.
   *
   * @return A pointer to the application's instance specific
   * @ref KConfig object.
   * @see KConfig
   */
  KConfig* sessionConfig();

  /**
   * Is the application restored from the session manager?
   *
   * @return If true, this application was restored by the session manager.
   *    Note that this may mean the config object returned by
   * @ref sessionConfig() contains data saved by a session closedown.
   * @see #sessionConfig()
   */
  bool isRestored() const { return QApplication::isSessionRestored(); }

  /**
   * Disables session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "startkde" script.
   */
  void disableSessionManagement();

  /**
   * Enables again session management for this application, formerly
   * disabled by calling @ref disableSessionManagement(). You usually
   * shouldn't call this function, as the session management is enabled
   * by default.
   */
  void enableSessionManagement();
  
  /**
   * The possible values for the @p confirm parameter of requestShutDown().
   */
  enum ShutdownConfirm {
    /**
     * Obey the user's confirmation setting.
     */
    ShutdownConfirmDefault = -1,
    /**
     * Don't confirm, shutdown without asking.
     */
    ShutdownConfirmNo = 0,
    /**
     * Always confirm, ask even if the user turned it off.
     */
    ShutdownConfirmYes = 1
  };

  /**
   * The possible values for the @p sdtype parameter of requestShutDown().
   */
  enum ShutdownType {
    /**
     * Select previous action or the default if it's the first time.
     */
    ShutdownTypeDefault = -1,
    /**
     * Only log out.
     */
    ShutdownTypeNone = 0,
    /**
     * Log out and reboot the machine.
     */
    ShutdownTypeReboot = 1,
    /**
     * Log out and halt the machine.
     */
    ShutdownTypeHalt = 2
  };

  /**
   * The possible values for the @p sdmode parameter of requestShutDown().
   */
  enum ShutdownMode {
    /**
     * Select previous mode or the default if it's the first time.
     */
    ShutdownModeDefault = -1,
    /**
     * Schedule a shutdown (halt or reboot) for the time all active sessions
     * have exited.
     */
    ShutdownModeSchedule = 0,
    /**
     * Shut down, if no sessions are active. Otherwise do nothing.
     */
    ShutdownModeTryNow = 1,
    /**
     * Force shutdown. Kill any possibly active sessions.
     */
    ShutdownModeForceNow = 2,
    /**
     * Pop up a dialog asking the user what to do if sessions are still active.
     */
    ShutdownModeInteractive = 3
  };

  /**
   * Asks the session manager to shut the session down.
   *
   * Using @p confirm == ShutdownConfirmYes or @p sdtype != ShutdownTypeDefault or
   * @p sdmode != ShutdownModeDefault causes the use of ksmserver's DCOP
   * interface. The remaining two combinations use the standard XSMP and
   * will work with any session manager compliant with it.
   *
   * @param confirm Whether to ask the user if he really wants to log out.
   *   @ref ShutdownConfirm
   * @param sdtype The action to take after logging out. @ref ShutdownType
   * @param sdmode If/When the action should be taken. @ref ShutdownMode
   * @return true on sucess, false if the session manager could not be
   * contacted.
   */
  bool requestShutDown( ShutdownConfirm confirm = ShutdownConfirmDefault,
                        ShutdownType sdtype = ShutdownTypeDefault,
			ShutdownMode sdmode = ShutdownModeDefault );

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
   * Returns a pointer to a @ref DCOPClient for the application.
   * If a client does not exist yet, it is created when this
   * function is called.
   * @return the DCOPClient for the application
   */
  static DCOPClient *dcopClient();

  /**
   * Disable automatic dcop registration
   * Must be called before creating a KApplication instance to have an effect.
   */
  static void disableAutoDcopRegistration();

  /**
   * Returns a @ref QPixmap with the application icon.
   * @return the application icon
   */
  QPixmap icon() const;

  /**
   * Returns the name of the application icon.
   * @return the icon's name
   */
  QString iconName() const;

  /**
   * Returns the mini-icon for the application as a @ref QPixmap.
   * @return the application's mini icon
   */
  QPixmap miniIcon() const;

  /**
   * Returns the name of the mini-icon for the application.
   * @return the mini icon's name
   */
  QString miniIconName() const;

  /**
   *  Sets the top widget of the application.
   *  This means basically applying the right window caption and
   *  icon. An application may have several top widgets. You don't
   *  need to call this function manually when using @ref KMainWindow.
   *
   *  @param topWidget A top widget of the application.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

  /**
   * Invokes the KHelpCenter HTML help viewer from docbook sources.
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
   * Convenience method; invokes the standard email application.
   *
   * All parameters are optional.
   *
   * @param to          The destination address.
   * @param cc          The Cc field
   * @param bcc         The Bcc field
   * @param subject     Subject string
   * @param body        A string containing the body of the mail (exclusive with messageFile)
   * @param messageFile A file (URL) containing the body of the mail (exclusive with body) - currently unsupported
   * @param attachURLs  List of URLs to be attached to the mail.
   */
  void invokeMailer(const QString &to, const QString &cc, const QString &bcc,
                    const QString &subject, const QString &body,
                    const QString &messageFile = QString::null, const QStringList &attachURLs = QStringList());

public slots:
  /**
   * Invokes the standard browser.
   * Note that you should only do this when you know for sure that the browser can
   * handle the URL (i.e. its mimetype). In doubt, if the URL can point to an image
   * or anything else than directory or HTML, prefer to use new KRun( url ).
   *
   * @param url The destination address
   */
  void invokeBrowser( const QString &url );

public:
  /**
   * Returns the DCOP name of the service launcher. This will be something like
   * klaucher_$host_$uid.
   * @return the name of the service launcher
   */
  static QCString launcher();

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param name the name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid=0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the (translated) name of the service.
   * E.g. "Web Browser"
   *
   * @param name the name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid=0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param name the path of the desktop file
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid = 0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop path of the service.
   * E.g. "Applications/konqueror.desktop" or "/home/user/bla/myfile.desktop"
   *
   * @param name the path of the desktop file
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopPath( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid = 0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param name the desktop name of the service
   * @param URL if not empty this URL is passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const QString& _name, const QString &URL,
                QString *error=0, QCString *dcopService=0, int *pid = 0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a service based on the desktop name of the service.
   * E.g. "konqueror"
   *
   * @param name the desktop name of the service
   * @param URLs if not empty these URLs will be passed to the service
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param dcopService On success, @p dcopService contains the DCOP name 
   *         under which this service is available. If empty, the service does
   *         not provide DCOP services. If the pointer is 0 the argument
   *         will be ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @param startup_id for app startup notification, "0" for none,
   *           "" ( empty string ) is the default
   * @param noWait if set, the function does not wait till the service is running.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int startServiceByDesktopName( const QString& _name, const QStringList &URLs=QStringList(),
                QString *error=0, QCString *dcopService=0, int *pid = 0, const QCString &startup_id = "", bool noWait = false );

  /**
   * Starts a program via kdeinit.
   * There will be no application startup notification, so you should
   * use it only for starting internal processes.
   *
   * program name and arguments are converted to according to the
   * local encoding and passed as is to kdeinit.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int kdeinitExec( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0 );

  /**
   * Starts a program via kdeinit and wait for it to finish.
   *
   * Like @ref kdeinitExec(), but it waits till the program is finished.
   * As such it behaves similar to the system(...) function.
   *
   * @param name Name of the program to start
   * @param args Arguments to pass to the program
   * @param error On failure, @p error contains a description of the error
   *         that occurred. If the pointer is 0, the argument will be
   *         ignored
   * @param pid On success, the process id of the new service will be written
   *        here. If the pointer is 0, the argument will be ignored.
   * @return an error code indicating success (== 0) or failure (> 0).
   */
  static int kdeinitExecWait( const QString& name, const QStringList &args=QStringList(),
                QString *error=0, int *pid = 0 );

  /**
   * Returns a text for the window caption.
   *
   * This may be set by
   * "-caption", otherwise it will be equivalent to the name of the
   * executable.
   * @return the text for the window caption
   */
  QString caption() const;

  /**
   * @deprecated
   */
  KStyle* kstyle() const { return 0; }

  /**
   * Builds a caption that contains the application name along with the
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
   * @return the created caption
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

#ifdef Q_WS_X11
  /**
   * Get the X11 display
   * @return the X11 Display
   */
  Display *getDisplay() { return display; }
#endif

  /**
   * Enables style plugins.
   *
   * This is useful only to applications that normally
   * do not display a GUI and create the KApplication with
   * allowStyles set to false.
   */
  void enableStyles();

  /**
   * Disables style plugins.
   *
   * Current style plugins do not get unloaded.
   *
   * This is only useful when used in combination with @ref enableStyles().
   */
  void disableStyles();

  /**
   *  Installs widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard @ref QWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   **/
  void installX11EventFilter( QWidget* filter );

  /**
   * Generates a uniform random number.
   * @return A truly unpredictable number in the range [0, RAND_MAX)
   */
  static int random();

  /**
   * Generates a random string.  It operates in the range [A-Za-z0-9]
   * @param length Generate a string of this length.
   * @return the random string
   */
  static QString randomString(int length);

  /**
   * Adds a message type to the KIPC event mask. You can only add "system
   * messages" to the event mask. These are the messages with id < 32.
   * Messages with id >= 32 are user messages.
   * @param id The message id. See @ref KIPC::Message.
   * @see KIPC
   * @see removeKipcEventMask()
   * @see kipcMessage()
   */
  void addKipcEventMask(int id);

  /**
   * Removes a message type from the @ref KIPC event mask. This message will 
   * not be handled anymore.
   * @param id The message id.
   * @see KIPC
   * @see addKipcEventMask()
   * @see kipcMessage()
   */
  void removeKipcEventMask(int id);

  /**
   * Returns the app startup notification identifier for this running 
   * application.
   * @return the startup notification identifier
   */
  QCString startupId() const;

  /**
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows.
   * @param the startup notification identifier
   */
  void setStartupId( const QCString& startup_id );

    /**
    * Returns the argument to --geometry if any, so the geometry can be set
    * wherever necessary
    * @return the geometry argument, or QString::null if there is none
    */
  QString geometryArgument() const;

  /**
   * Install a Qt SQL property map with entries for all KDE widgets
   * Call this in any application using KDE widgets in QSqlForm or QDataView.
   */
  void installKDEPropertyMap();

  /**
   * Returns whether a certain action is authorized
   * @param genericAction The name of a generic  action
   * @return true if the action is authorized
   */
  bool authorize(const QString &genericAction);

  /**
   * Returns whether a certain KAction is authorized.
   *
   * @param action The name of a KAction action. The name is prepended
   * with "action/" before being passed to @ref authorize()
   * @return true if the KAction is authorized
   */
  bool authorizeKAction(const char *action);

  /**
   * Returns whether a certain URL related action is authorized.
   *
   * @param action The name of the action. Known actions are
   * list (may be listed (e.g. in file selection dialog)),
   * link (may be linked to),
   * open (may open) and
   * redirect (may be redirected to)
   * @param baseURL The url where the action originates from
   * @param destURL The object of the action
   * @return true when the action is authorized, false otherwise.
   * @since 3.1
   */
  bool authorizeURLAction(const QString &action, const KURL &baseURL, const KURL &destURL);
                   

  // Same values as ShiftMask etc. in X.h
  enum { ShiftModifier = 1<<0,
         LockModifier = 1<<1,
         ControlModifier = 1<<2,
         Modifier1 = 1<<3,
         Modifier2 = 1<<4,
         Modifier3 = 1<<5,
         Modifier4 = 1<<6,
         Modifier5 = 1<<7 };
  /**
   * Returns the currently pressed keyboard modifiers (e.g. shift, control, etc.)
   * Usually you simply want to test for those in key events, in which case
   * @ref QKeyEvent::state() does the job (or @ref QKeyEvent::key() to 
   * notice when a modifier is pressed alone).
   * But it can be useful to query for the status of the modifiers at another moment
   * (e.g. some KDE apps do that upon a drop event).
   * @return the keyboard modifiers
   * @since 3.1
   */
  static uint keyboardModifiers();

  // Same values as Button1Mask etc. in X.h
  enum { Button1Pressed = 1<<8,
         Button2Pressed = 1<<9,
         Button3Pressed = 1<<10,
         Button4Pressed = 1<<11,
         Button5Pressed = 1<<12 };
  /**
   * Returns the currently pressed mouse buttons.
   * Usually you simply want to test for those in mouse events, in which case
   * @ref QMouseEvent::button() does the job (or @ref QMouseEvent::state()).
   * But it can be useful to query for the status of the mouse buttons at another moment.
   * To query for the mouse pointer position, use @ref QCursor::pos().
   * @return the currently pressed mouse buttons 
   * @since 3.1
   */
  static uint mouseState();


public slots:
  /**
   * Tells KApplication about one more operation that should be finished
   * before the application exits. The standard behavior is to exit on the
   * "last window closed" event, but some events should outlive the last window closed
   * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
   */
  void ref();

  /**
   * Tells KApplication that one operation such as those described in @ref just finished.
   * The application exits if the counter is back to 0.
   */
  void deref();

protected:
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication( bool allowStyles, bool GUIenabled, KInstance* _instance );

#ifdef Q_WS_X11
  /**
   * Used to catch X11 events
   */
  bool x11EventFilter( XEvent * );

  Display *display;
#endif
  Atom kipcCommAtom;
  int kipcEventMask;

  /// Current application object.
  static KApplication *KApp;
  int pArgc;

private slots:
  void dcopFailure(const QString &);
  void dcopBlockUserInput( bool );
  void x11FilterDestroyed();

private:
  QString sessionConfigName() const;
  KConfig* pSessionConfig; //instance specific application config object
  static DCOPClient *s_DCOPClient; // app specific application communication client
  static bool s_dcopClientNeedsPostInit;
  QString aCaption; // the name for the window title
  bool bSessionManagement;
  QPixmap aIconPixmap;
  QPixmap aMiniIconPixmap;
  QString aIconName;
  QString aMiniIconName;
  bool useStyles;
  QWidget *smw;

  void init( bool GUIenabled );

  void parseCommandLine( ); // Handle KDE arguments (Using KCmdLineArgs)

  void read_app_startup_id();

  void dcopAutoRegistration();
  void dcopClientPostInit();
  void initUrlActionRestrictions();
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
         SETTINGS_POPUPMENU, SETTINGS_QT, SETTINGS_SHORTCUTS };

  /**
   * Used to obtain the QPalette that will be used to set the application palette.
   * 
   * This is only useful for configuration modules such as krdb and should not be
   * used in normal circumstances.
   * @return the QPalette
   * @since 3.1
   */
  static QPalette createApplicationPalette();

  /**
   * @internal
   * Raw access for use by KDM.
   */
  static QPalette createApplicationPalette( KConfig *config, int contrast );

signals:
  /**
   * Emitted when KApplication has changed its palette due to a KControl request.
   *
   * Normally, widgets will update their palette automatically, but you
   * should connect to this to program special behavior.
   */
  void kdisplayPaletteChanged();

  /**
   * Emitted when KApplication has changed its GUI style in response to a KControl request.
   *
   * Normally, widgets will update their styles automatically (as they would
   * respond to an explicit setGUIStyle() call), but you should connect to
   * this to program special behavior.
   */
  void kdisplayStyleChanged();

  /**
   * Emitted when KApplication has changed its font in response to a KControl request.
   *
   * Normally widgets will update their fonts automatically, but you should
   * connect to this to monitor global font changes, especially if you are
   * using explicit fonts.
   */
  void kdisplayFontChanged();

  /**
   * Emitted when KApplication has changed either its GUI style, its font or its palette
   * in response to a kdisplay request. Normally, widgets will update their styles
   * automatically, but you should connect to this to program special
   * behavior. */
  void appearanceChanged();

  /**
   * Emitted when the settings for toolbars have been changed. KToolBar will know what to do.
   */
  void toolbarAppearanceChanged(int);

  /**
   * Emitted when the desktop background has been changed by @p kcmdisplay.
   *
   * @param desk The desktop whose background has changed.
   */
  void backgroundChanged(int desk);

  /**
   * Emitted when the global settings have been changed - see KGlobalSettings
   * KApplication takes care of calling reparseConfiguration on KGlobal::config()
   * so that applications/classes using this only have to re-read the configuration
   * @param category the category among the enum above
   */
  void settingsChanged(int category);

  /**
   * Emitted when the global icon settings have been changed.
   * @param group the new group
   */
  void iconChanged(int group);

  /**
   * Emitted when a KIPC user message has been received.
   * @param id the message id
   * @param data the data
   * @see KIPC
   * @see KIPC::Message
   * @see addKipcEventMask
   * @see removeKipcEventMask
   */
  void kipcMessage(int id, int data);

  /**
      Session management asks you to save the state of your application.

     This signal is provided for compatibility only. For new
     applications, simply use KMainWindow. By reimplementing @ref
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
     instance specific data.

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
  void applyGUIStyle();

  int captionLayout;

  KApplication(const KApplication&);
  KApplication& operator=(const KApplication&);
protected:
  virtual void virtual_hook( int id, void* data );
private:
  KApplicationPrivate* d;
};


/** Check, if a file may be accessed in a given mode.
 * This is a wrapper around the access() system call.
 * checkAccess() calls access() with the given parameters.
 * If this is OK, checkAccess() returns true. If not, and W_OK
 * is part of mode, it is checked if there is write access to
 * the directory. If yes, checkAccess() returns true.
 * In all other cases checkAccess() returns false.
 *
 * Other than access() this function EXPLICITLY ignores non-existant
 * files if checking for write access.
 *
 * @param pathname The full path of the file you want to test
 * @param mode     The access mode, as in the access() system call.
 * @return Whether the access is allowed, true = Access allowed
 */
bool checkAccess(const QString& pathname, int mode);


class KSessionManagedPrivate;

/**
   Provides highlevel access to session management on a per-object
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

       Return true to indicate a successful state save or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool saveState( QSessionManager& sm );
    /**
       See @ref QApplication::commitData() for documentation.

       This function is just a convenience version to avoid subclassing KApplication.

       Return true to indicate a successful commit of data or false to
       indicate a problem and to halt the shutdown process (will
       implicitly call sm.cancel() ).
     */
  virtual bool commitData( QSessionManager& sm );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KSessionManagedPrivate *d;
};


#endif

