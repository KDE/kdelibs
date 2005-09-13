/*
    This file is part of the KDE libraries

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.


*/

#ifndef KMAINWINDOW_H
#define KMAINWINDOW_H

#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"
#include <q3mainwindow.h>
#include <qmetaobject.h>

class KPopupMenu;
class KXMLGUIFactory;
class KConfig;
class KHelpMenu;
class KStatusBar;
class QStatusBar;
class KMenuBar;
class KMWSessionManaged;
class KMainWindowPrivate;
class KAccel;
class KToolBar;
class KToolBarMenuAction;
class DCOPObject;

#define KDE_DEFAULT_WINDOWFLAGS Qt::WType_TopLevel | Qt::WDestructiveClose


/**
 * @short %KDE top level main window
 *
 * Top level widget that provides toolbars, a status line and a frame.
 *
 * It should be used as a top level (parent-less) widget.
 * It manages the geometry for all its children, including your
 * main widget.
 *
 * Normally, you will inherit from KMainWindow,
 * then construct (or use some existing) widget as
 * your main view. You can set only one main view.
 *
 * You can add as many toolbars as you like. There can be only one menubar
 * and only one statusbar.
 *
 * The toolbars, menubar, and statusbar can be created by the
 * KMainWindow and - unlike the old KMainWindow - may, but do not
 * have to, be deleted by you. KMainWindow will handle that internally.
 *
 * Height and width can be operated independently from each other. Simply
 * define the minimum/maximum height/width of your main widget and
 * KMainWindow will take this into account. For fixed size windows set
 * your main widget to a fixed size.
 *
 * Fixed aspect ratios (heightForWidth()) and fixed width widgets are
 * not supported.
*
 * KMainWindow will set icon, mini icon and caption, which it gets
 * from KApplication. It provides full session management, and
 * will save its position, geometry and positions of toolbars and
 * menubar on logout. If you want to save additional data, reimplement
 * saveProperties() and (to read them again on next login)
 * readProperties(). To save special data about your data, reimplement
 * saveGlobalProperties(). To warn user that application or
 * windows have unsaved data on close or logout, reimplement
 * queryClose() and/or queryExit().
 *
 * There are also kRestoreMainWindows convenience functions which
 * can restore all your windows on next login.
 *
 *  Note that a KMainWindow per-default is created with the
 *  WDestructiveClose flag, i.e. it is automatically destroyed when the
 *  window is closed. If you do not want this behavior, specify 0 as
 *  widget flag in the constructor.
 *
 * @see KApplication
 * @author Reginald Stadlbauer (reggie@kde.org) Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by Sven Radej (radej@kde.org)

 */

class KDEUI_EXPORT KMainWindow : public Q3MainWindow, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    friend class KMWSessionManaged;
    Q_OBJECT

public:
    /**
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KMainWindow becomes sort of a
     * secondary window.
     *
     * @param name The object name. For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. When passing 0 (the default), KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will append numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     * @param f Specify the widget flags. The default is
     * WType_TopLevel and WDestructiveClose.  TopLevel indicates that a
     * main window is a toplevel window, regardless of whether it has a
     * parent or not. DestructiveClose indicates that a main window is
     * automatically destroyed when its window is closed. Pass 0 if
     * you do not want this behavior.
     *
     * @see http://doc.trolltech.com/3.2/qt.html#WidgetFlags-enum
     *
     * KMainWindows must be created on the heap with 'new', like:
     * \code
     * KMainWindow *kmw = new KMainWindow (...);
     * \endcode
     **/
    KMainWindow( QWidget* parent = 0, const char *name = 0, Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );

    /**
     * Flags that can be passed in an argument to the constructor to
     * change the behavior.
     *
     * NoDCOPObject tells KMainWindow not to create a KMainWindowInterface.
     * This can be useful in particular for inherited classes, which
     * might want to create more specific dcop interfaces. It's a good
     * idea to use KMainWindowInterface as the base class for such interfaces
     * though (to provide the standard mainwindow functionality via DCOP).
     */
    enum CreationFlags
    {
        NoDCOPObject = 1
    };

    /**
     * Overloaded constructor which allows passing some KMainWindow::CreationFlags.
     *
     * @since 3.2
     */
    KMainWindow( int cflags, QWidget* parent = 0, const char *name = 0, Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );

    /**
     * \brief Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    virtual ~KMainWindow();

    /**
     * Retrieve the standard help menu.
     *
     * It contains entires for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box.
     *
     * Example (adding a standard help menu to your application):
     * \code
     * KPopupMenu *help = helpMenu( <myTextString> );
     * menuBar()->insertItem( i18n("&Help"), help );
     * \endcode
     *
     * @param aboutAppText The string that is used in the application
     *        specific dialog box. If you leave this string empty the
     *        information in the global KAboutData of the
     *        application will be used to make a standard dialog box.
     *
     * @param showWhatsThis Set this to false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     */
    KPopupMenu* helpMenu( const QString &aboutAppText = QString::null,
			  bool showWhatsThis = true );

    /**
     * Returns the help menu. Creates a standard help menu if none exists yet.
     *
     * It contains entries for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box. You must create the application
     * specific dialog box yourself. When the "About application"
     * menu entry is activated, a signal will trigger the
     * showAboutApplication slot. See showAboutApplication for more
     * information.
     *
     * Example (adding a help menu to your application):
     * \code
     * menuBar()->insertItem( i18n("&Help"), customHelpMenu() );
     * \endcode
     *
     * @param showWhatsThis Set this to @p false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     */
    KPopupMenu* customHelpMenu( bool showWhatsThis = true );

    /**
     * <b>Session Management</b>
     *
     * Try to restore the toplevel widget as defined by the number (1..X).
     *
     * If the session did not contain so high a number, the configuration
     * is not changed and @p false returned.
     *
     * That means clients could simply do the following:
     * \code
     * if (kapp->isRestored()){
     *   int n = 1;
     *   while (KMainWindow::canBeRestored(n)){
     *     (new childMW)->restore(n);
     *     n++;
     *   }
     * } else {
     *   // create default application as usual
     * }
     * \endcode
     * Note that QWidget::show() is called implicitly in restore.
     *
     * With this you can easily restore all toplevel windows of your
     * application.
     *
     * If your application uses different kinds of toplevel
     * windows, then you can use KMainWindow::classNameOfToplevel(n)
     * to determine the exact type before calling the childMW
     * constructor in the example from above.
     *
     * If your client has only one kind of toplevel widgets (which
     * should be pretty usual) then you should use the RESTORE-macro
     * for backwards compatibility with 3.1 and 3.0 branches:
     *
     * \code
     * if (kapp->isRestored())
     *   RESTORE(childMW)
     * else {
     *   // create default application as usual
     * }
     * \endcode
     *
     * The macro expands to the term above but is easier to use and
     * less code to write.
     *
     * For new code or if you have more than one kind of toplevel
     * widget (each derived from KMainWindow, of course), you can
     * use the templated kRestoreMainWindows global functions:
     *
     * \code
     * if (kapp->isRestored())
     *   kRestoreMainWindows< childMW1, childMW2, childMW3 >();
     * else {
     *   // create default application as usual
     * }
     * \endcode
     *
     * Currently, these functions are provided for up to three
     * template arguments. If you need more, tell us. To help you in
     * deciding whether or not you can use kRestoreMainWindows, a
     * define KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS is provided.
     *
     * @see restore()
     * @see classNameOfToplevel()
     *
     **/
    static bool canBeRestored( int number );

    /**
     * Returns the className() of the @p number of the toplevel window which
     * should be restored.
     *
     * This is only useful if your application uses
     * different kinds of toplevel windows.
     */
    // KDE 4 return QCString - QObject::className() returns const char*
    static const QString classNameOfToplevel( int number );

    /**
     * Reimplementation of QMainWindow::show()
     */
    // KDE4 remove this method if this has been fixed in Qt
    virtual void show();

    /**
     * Reimplementation of QMainWindow::hide()
     */
    // KDE4 remove this method if this has been fixed in Qt
    virtual void hide();

    /**
     * Restore the session specified by @p number.
     *
     * Returns @p false if this
     * fails, otherwise returns @p true and shows the window.
     * You should call canBeRestored() first.
     * If @p show is true (default), this widget will be shown automatically.
     */
    bool restore( int number, bool show = true );

    virtual KXMLGUIFactory *guiFactory();

    /**
     * Create a GUI given a local XML file.
     *
     * If @p xmlfile is NULL,
     * then it will try to construct a local XML filename like
     * appnameui.rc where 'appname' is your app's name.  If that file
     * does not exist, then the XML UI code will only use the global
     * (standard) XML file for the layout purposes.
     *
     * Note that when passing true for the conserveMemory argument subsequent
     * calls to guiFactory()->addClient/removeClient may not work as expected.
     * Also retrieving references to containers like popup menus or toolbars using
     * the container method will not work.
     *
     * @param xmlfile The local xmlfile (relative or absolute)
     * @param _conserveMemory Specify whether createGUI() should call
     * KXMLGuiClient::conserveMemory() to free all memory
     *     allocated by the QDomDocument and by the KXMLGUIFactory.
     */
    void createGUI( const QString &xmlfile = QString::null, bool _conserveMemory = true );

    /**
     * Enables the build of a standard help menu when calling createGUI().
     *
     * The default behavior is to build one, you must call this function
     * to disable it
     */
    void setHelpMenuEnabled(bool showHelpMenu = true);

    /**
     * Return @p true when the help menu is enabled
     */
    bool isHelpMenuEnabled();


    /**
     * Returns true, if there is a menubar
     * @since 3.1
     */
     bool hasMenuBar();

    /**
     * Returns a pointer to the menu bar.
     *
     * If there is no menu bar yet one will be created.
     **/
    KMenuBar *menuBar();

    /**
     * Returns a pointer to the status bar.
     *
     * If there is no status bar yet, one will be created.
     *
     * Note that tooltips for kactions in actionCollection() are not
     * automatically connected to this statusBar.
     * See the KActionCollection documentation for more details.
     *
     * @see KActionCollection
     */
    KStatusBar *statusBar();


    //KDE4: replace with memberList() and make memberList member private
    /**
     * List of members of KMainWindow class.
     * @since 3.4
     */
    static Q3PtrList<KMainWindow>* memberList();

    /**
     * Returns a pointer to the toolbar with the specified name.
     * This refers to toolbars created dynamically from the XML UI
     * framework.  If the toolbar does not exist one will be created.
     *
     * @param name The internal name of the toolbar. If no name is
     *             specified "mainToolBar" is assumed.
     *
     * @return A pointer to the toolbar
     **/
    KToolBar *toolBar( const char *name=0 );

    /**
     * @return A list of all toolbars for this window
     */
    QList<KToolBar*> toolBarList() /*TODO const*/;

    /**
     * @return A KAccel instance bound to this mainwindow. Used automatically
     * by KAction to make keybindings work in all cases.
     */
    KAccel *accel();

    void setFrameBorderWidth( int ) {}

    /**
     * Call this to enable "auto-save" of toolbar/menubar/statusbar settings
     * (and optionally window size).
     * If the *bars were moved around/shown/hidden when the window is closed,
     * saveMainWindowSettings( KGlobal::config(), groupName ) will be called.
     *
     * @param groupName a name that identifies this "type of window".
     * You can have several types of window in the same application.
     *
     * @param saveWindowSize set it to true to include the window size
     * when saving.
     *
     * Typically, you will call setAutoSaveSettings() in your
     * KMainWindow-inherited class constructor, and it will take care
     * of restoring and saving automatically. Make sure you call this
     * _after all_ your *bars have been created.
     *
     * To make sure that KMainWindow propertly obtains the default
     * size of the window you should do the following:
     * - Remove hard coded resize() calls in the constructor or main, they
     *   should be removed in favor of letting the automatic resizing
     *   determine the default window size.  Hard coded window sizes will
     *   be wrong for users that have big fonts, use different styles,
     *   long/small translations, large toolbars, and other factors.
     * - Put the setAutoSaveSettings ( or setupGUI() ) call after all widgets
     *   have been created and placed inside the main window (i.e. for 99% of
     *   apps setCentralWidget())
     * - Widgets that inherit from QWidget (like game boards) should overload
     *   "virtual QSize sizeHint() const;" to specify a default size rather
     *   than letting QWidget::adjust use the default size of 0x0.
     */
    void setAutoSaveSettings( const QString & groupName = QLatin1String("MainWindow"),
                              bool saveWindowSize = true );

    /**
     * Disable the auto-save-settings feature.
     * You don't normally need to call this, ever.
     */
    void resetAutoSaveSettings();

    /**
     * @return the current autosave setting, i.e. true if setAutoSaveSettings() was called,
     * false by default or if resetAutoSaveSettings() was called.
     * @since 3.1
     */
    bool autoSaveSettings() const;

    /**
     * @return the group used for setting-autosaving.
     * Only meaningful if setAutoSaveSettings() was called.
     * This can be useful for forcing a save or an apply, e.g. before and after
     * using KEditToolbar.
     * @since 3.1
     */
    QString autoSaveGroup() const;

    /**
     * Read settings for statusbar, menubar and toolbar from their respective
     * groups in the config file and apply them.
     *
     * @param config Config file to read the settings from.
     * @param groupName Group name to use. If not specified, the last used
     * group name is used.
     * @param force if set, even default settings are re-applied
     */
    void applyMainWindowSettings(KConfig *config, const QString &groupName, bool force);
    // KDE4 merge with force=false
    void applyMainWindowSettings(KConfig *config, const QString &groupName = QString::null);

    /**
     * Save settings for statusbar, menubar and toolbar to their respective
     * groups in the config file @p config.
     *
     * @param config Config file to save the settings to.
     * @param groupName Group name to use. If not specified, the last used
     * group name is used
     */
    void saveMainWindowSettings(KConfig *config, const QString &groupName = QString::null);

    /**
     * Sets whether KMainWindow should provide a menu that allows showing/hiding
     * the available toolbars ( using KToggleToolBarAction ) . In case there
     * is only one toolbar configured a simple 'Show \<toolbar name here\>' menu item
     * is shown.
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted in your
     * menu structure in the 'Settings' menu.
     *
     * If your application uses a non-standard xmlgui resource file then you can
     * specify the exact position of the menu / menu item by adding a
     * &lt;Merge name="StandardToolBarMenuHandler" /&gt;
     * line to the settings menu section of your resource file ( usually appname.rc ).
     *
     * Note that you should enable this feature before calling createGUI() ( or similar ) .
     * You enable/disable it anytime if you pass false to the conserveMemory argument of createGUI.
     * @since 3.1
     */
    void setStandardToolBarMenuEnabled( bool enable );
    /// @since 3.1
    bool isStandardToolBarMenuEnabled() const;


    /**
     * Sets whether KMainWindow should provide a menu that allows showing/hiding
     * of the statusbar ( using KToggleStatusBarAction ).
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted
     * in your menu structure in the 'Settings' menu.
     *
     * Note that you should enable this feature before calling createGUI()
     * ( or similar ).
     *
     * If an application maintains the action on its own (i.e. never calls
     * this function) a connection needs to be made to let KMainWindow
     * know when that status (hidden/shown) of the statusbar has changed.
     * For example:
     * connect(action, SIGNAL(activated()),
     *         kmainwindow, SLOT(setSettingsDirty()));
     * Otherwise the status (hidden/show) of the statusbar might not be saved
     * by KMainWindow.
     * @since 3.2
     */
    void createStandardStatusBarAction();

    /**
     * @see setupGUI()
     */
    enum StandardWindowOptions
    {
        /**
         * adds action to show/hide the toolbar(s) and adds
         * action to configure the toolbar(s).
         * @see setStandardToolBarMenuEnabled
         */
        ToolBar = 1,

        /**
         * adds action to show the key configure action.
         */
        Keys = 2,

        /**
         * adds action to show/hide the statusbar if the
         * statusbar exists.  @see createStandardStatusBarAction
         */
        StatusBar = 4,

        /**
         * auto-saves (and loads) the toolbar/menubar/statusbar settings and
         * window size using the default name.  @see setAutoSaveSettings
         *
         * Typically you want to let the default window size be determined by
         * the widgets size hints. Make sure that setupGUI() is called after
         * all the widgets are created ( including setCentralWidget ) so the
         * default size's will be correct. @see setAutoSaveSettings for
         * more information on this topic.
         */
        Save = 8,

        /**
         * calls createGUI() once ToolBar, Keys and Statusbar have been
         * taken care of.  @see createGUI
         */
        Create = 16
    };

    /**
     * Configures the current windows and its actions in the typical KDE
     * fashion.  The options are all enabled by default but can be turned
     * off if desired through the params or if the prereqs don't exists.
     *
     * Typically this function replaces createGUI().
     *
     * @see StandardWindowOptions
     *
     * @since 3.3
     */
    void setupGUI( int options = ToolBar | Keys | StatusBar | Save | Create, const QString& xmlfile = QString::null );

    /**
     * Configures the current windows and its actions in the typical KDE
     * fashion.  The options are all enabled by default but can be turned
     * off if desired through the params or if the prereqs don't exists.
     *
     * @p defaultSize The default size of the window
     *
     * Typically this function replaces createGUI().
     *
     * @see StandardWindowOptions
     *
     * @since 3.5
     */
    void setupGUI( QSize defaultSize, int options = ToolBar | Keys | StatusBar | Save | Create, const QString& xmlfile = QString::null );

    /**
     * Returns a pointer to the mainwindows action responsible for the toolbars menu
     * @since 3.1
     */
    KAction *toolBarMenuAction();

    /**
     * @internal for KToolBar
     * @since 3.3.1
     */
    void setupToolbarMenuActions();

    // why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
    /// @since 3.1
    virtual void finalizeGUI( KXMLGUIClient *client );

    /**
     * @internal
     */
    void finalizeGUI( bool force );

    /**
     * @return true if a -geometry argument was given on the command line,
     * and this is the first window created (the one on which this option applies)
     */
    bool initialGeometrySet() const;

    /**
     * @internal
     * Used from Konqueror when reusing the main window.
     */
    void ignoreInitialGeometry();

    /**
     * @internal
     */
    // KDE4 remove
    virtual void setIcon( const QPixmap & );

public slots:
    /**
     * Show a standard configure toolbar dialog.
     *
     * This slot can be connected dirrectly to the action to configure shortcuts.
     * This is very simple to do that by adding a single line
     * \code
     * KStdAction::configureToolbars( guiFactory(), SLOT( configureToolbars() ),
     *                                actionCollection() );
     * \endcode
     *
     * @since 3.3
     */
   int configureToolbars(); // TODO KDE4: make virtual and reimplement in KParts::MainWindow

    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const QString &caption );
    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     * @param modified Specify whether the document is modified. This displays
     * an additional sign in the title bar, usually "**".
     */
    virtual void setCaption( const QString &caption, bool modified );

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption( const QString &caption );

    /**
     * Open the help page for the application.
     *
     *  The application name is
     * used as a key to determine what to display and the system will attempt
     * to open \<appName\>/index.html.
     *
     * This method is intended for use by a help button in the toolbar or
     * components outside the regular help menu. Use helpMenu() when you
     * want to provide access to the help system from the help menu.
     *
     * Example (adding a help button to the first toolbar):
     *
     * \code
     * KIconLoader &loader = *KGlobal::iconLoader();
     * QPixmap pixmap = loader.loadIcon( "help" );
     * toolBar(0)->insertButton( pixmap, 0, SIGNAL(clicked()),
     *   this, SLOT(appHelpActivated()), true, i18n("Help") );
     * \endcode
     *
     */
    void appHelpActivated( void );

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file
     * @since 3.1
     */
    virtual void slotStateChanged(const QString &newstate);

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file,
     * can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
     * @since 3.1
     */
    void slotStateChanged(const QString &newstate,
                          KXMLGUIClient::ReverseStateChange); // KDE 4.0: remove this


    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file,
     * can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
     */
//     void slotStateChanged(const QString &newstate,
//                           bool reverse); // KDE 4.0: enable this

    /**
     * Tell the main window that it should save its settings when being closed.
     * This is part of the auto-save-settings feature.
     * For everything related to toolbars this happens automatically,
     * but you have to call setSettingsDirty() in the slot that toggles
     * the visibility of the statusbar.
     */
    void setSettingsDirty();

protected:
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e);
    /**
     * Reimplemented to call the queryClose() and queryExit() handlers.
     *
     * We recommend that you reimplement the handlers rather than closeEvent().
     * If you do it anyway, ensure to call the base implementation to keep
     * queryExit() running.
     */
    virtual void closeEvent ( QCloseEvent *);

    // KDE4 This seems to be flawed to me. Either the app has only one
    // mainwindow, so queryClose() is enough, or if it can have more of them,
    // then the windows should take care of themselves, and queryExit()
    // would be useful only for the annoying 'really quit' dialog, which
    // also doesn't make sense in apps with multiple mainwindows.
    // And saving configuration in something called queryExit()? IMHO
    // one can e.g. use KApplication::shutDown(), which if nothing else
    // has at least better fitting name.
    // See also KApplication::sessionSaving().
    // This stuff should get changed somehow, so that it at least doesn't
    // mess with session management.
    /**
       Called before the very last window is closed, either by the
       user or indirectly by the session manager.

       It is not recommended to do any user interaction in this
       function other than indicating severe errors. Better ask the
       user on queryClose() (see below).

       A typical usage of queryExit() is to write configuration data back.
       Note that the application may continue to run after queryExit()
       (the user may have canceled a shutdown), so you should not do any cleanups
       here. The purpose of queryExit() is purely to prepare the application
       (with possible user interaction) so it can safely be closed later (without
       user interaction).

       If you need to do serious things on exit (like shutting a
       dial-up connection down), connect to the signal
 KApplication::shutDown().

       Default implementation returns @p true. Returning @p false will
       cancel the exiting. In the latter case, the last window will
       remain visible. If KApplication::sessionSaving() is true, refusing
       the exit will also cancel KDE logout.

       @see queryClose()
       @see KApplication::sessionSaving()
     */
    virtual bool queryExit();

    /**
       Called before the window is closed, either by the user or indirectly by
       the session manager.

       The purpose of this function is to prepare the window in a way that it is
       safe to close it, i.e. without the user losing some data.

       Default implementation returns true. Returning @p false will cancel
       the closing, and, if KApplication::sessionSaving() is true, it will also
       cancel KDE logout.

       Reimplement this function to prevent the user from losing data.
       Example:
       \code
       switch ( KMessageBox::warningYesNoCancel( this,
                i18n("Save changes to document foo?")) ) {
       case KMessageBox::Yes :
         // save document here. If saving fails, return false;
         return true;
       case KMessageBox::No :
         return true;
       default: // cancel
         return false;
       \endcode

       Note that you should probably @em not actually close the document from
       within this method, as it may be called by the session manager before the
       session is saved. If the document is closed before the session save occurs,
       its location might not be properly saved. In addition, the session shutdown
       may be canceled, in which case the document should remain open.

       @see queryExit()
       @see KApplication::sessionSaving()
    */
    virtual bool queryClose();

    /**
     * Save your instance-specific properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * You @em must @em not change the group of the @p kconfig object, since
     * KMainWindow uses one group for each window.  Please
     * reimplement these function in childclasses.
     *
     * Note: No user interaction is allowed
     * in this function!
     *
     */
    virtual void saveProperties( KConfig* ) {}

   /**
    * Read your instance-specific properties.
    */
    virtual void readProperties( KConfig* ) {}

   /**
     * Save your application-wide properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * This function is similar to saveProperties() but is only called for
     * the very first main window, regardless how many main window are open.

     * Override it if you need to save other data about your documents on
     * session end. sessionConfig is a config to which that data should be
     * saved. Normally, you don't need this function. But if you want to save
     * data about your documents that are not in opened windows you might need
     * it.
     *
     * Default implementation does nothing.
     */
    virtual void saveGlobalProperties( KConfig* sessionConfig );

    /**
     * The counterpart of saveGlobalProperties().
     *
     * Read the application-specific properties in again.
     */
    virtual void readGlobalProperties( KConfig* sessionConfig );
    void savePropertiesInternal( KConfig*, int );
    bool readPropertiesInternal( KConfig*, int );

    /**
     * For inherited classes
     */
    bool settingsDirty() const;
    /**
     * For inherited classes
     */
    QString settingsGroup() const;
    /**
     * For inherited classes
     * Note that the group must be set before calling
     */
    void saveWindowSize( KConfig * config ) const;
    /**
     * For inherited classes
     * Note that the group must be set before calling, and that
     * a -geometry on the command line has priority.
     */
    void restoreWindowSize( KConfig * config );

    /// parse the geometry from the geometry command line argument
    void parseGeometry(bool parsewidth);

protected slots:
   /**
    * Rebuilds the GUI after KEditToolbar changed the toolbar layout.
    * @see configureToolbars()
    */
   void saveNewToolbarConfig(); // TODO KDE4: make virtual and reimplement in KParts::MainWindow

    /**
    * This slot does nothing.
    *
    * It must be reimplemented if you want
    * to use a custom About Application dialog box. This slot is
    * connected to the About Application entry in the menu returned
    * by customHelpMenu.
    *
    * Example:
    * \code
    *
    * void MyMainLevel::setupInterface()
    * {
    *   ..
    *   menuBar()->insertItem( i18n("&Help"), customHelpMenu() );
    *   ..
    * }
    *
    * void MyMainLevel::showAboutApplication()
    * {
    *   <activate your custom dialog>
    * }
    * \endcode
    */
    virtual void showAboutApplication();

   /**
    * This slot should only be called in case you reimplement closeEvent() and
    * if you are using the "auto-save" feature. In all other cases,
    * setSettingsDirty() should be called instead to benefit from the delayed
    * saving.
    *
    * @see setAutoSaveSettings
    * @see setSettingsDirty
    *
    * @since 3.2
    *
    * Example:
    * \code
    *
    * void MyMainWindow::closeEvent( QCloseEvent *e )
    * {
    *   // Save settings if auto-save is enabled, and settings have changed
    *   if ( settingsDirty() && autoSaveSettings() )
    *     saveAutoSaveSettings();
    *   ..
    * }
    * \endcode
    */
    void saveAutoSaveSettings();

private slots:
   /**
    * Called when the app is shutting down.
    */
    void shuttingDown();

private:
    KMenuBar *internalMenuBar();
    KStatusBar *internalStatusBar();
    KHelpMenu *mHelpMenu, *helpMenu2;
    KXMLGUIFactory *factory_;
    QList<KToolBar*> toolbarList;
    /**
     * List of members of KMainWindow class.
     */
    static Q3PtrList<KMainWindow>* mMemberList;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KMainWindowPrivate *d;
    void initKMainWindow(const char *name, int cflags);
};

#define RESTORE(type) { int n = 1;\
    while (KMainWindow::canBeRestored(n)){\
      (new type)->restore(n);\
      n++;}}

#define KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS 3

/**
 *  These global convenience functions (that come with a varying
 *  number of template arguments) are a replacement for the RESTORE
 *  macro provided in earlier versions of KDE. The old RESTORE macro
 *  is still provided for backwards compatibility. See
 *  KMainWindow documentation for more.
 *
 * \since KDE 3.2
 *
 **/
template <typename T>
inline void kRestoreMainWindows() {
  for ( int n = 1 ; KMainWindow::canBeRestored( n ) ; ++n ) {
    const QString className = KMainWindow::classNameOfToplevel( n );
    if ( className == QLatin1String( T::staticMetaObject.className() ) )
      (new T)->restore( n );
  }
}

template <typename T0, typename T1>
inline void kRestoreMainWindows() {
  const char * classNames[2];
  classNames[0] = T0::staticMetaObject.className();
  classNames[1] = T1::staticMetaObject.className();
  for ( int n = 1 ; KMainWindow::canBeRestored( n ) ; ++n ) {
    const QString className = KMainWindow::classNameOfToplevel( n );
    if ( className == QLatin1String( classNames[0] ) )
      (new T0)->restore( n );
    else if ( className == QLatin1String( classNames[1] ) )
      (new T1)->restore( n );
  }
}

template <typename T0, typename T1, typename T2>
inline void kRestoreMainWindows() {
  const char * classNames[3];
  classNames[0] = T0::staticMetaObject.className();
  classNames[1] = T1::staticMetaObject.className();
  classNames[2] = T2::staticMetaObject.className();
  for ( int n = 1 ; KMainWindow::canBeRestored( n ) ; ++n ) {
    const QString className = KMainWindow::classNameOfToplevel( n );
    if ( className == QLatin1String( classNames[0] ) )
      (new T0)->restore( n );
    else if ( className == QLatin1String( classNames[1] ) )
      (new T1)->restore( n );
    else if ( className == QLatin1String( classNames[2] ) )
      (new T2)->restore( n );
  }
}

#endif
