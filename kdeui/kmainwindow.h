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

    $Id$

*/

#ifndef KMAINWINDOW_H
#define KMAINWINDOW_H

#include "kxmlguifactory.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"
#include <qmainwindow.h>
#include <ktoolbar.h>

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
class KToolBarMenuAction;


#define KDE_DEFAULT_WINDOWFLAGS WType_TopLevel | WDestructiveClose


/**
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
 * from @ref KApplication. It provides full session management, and
 * will save its position, geometry and positions of toolbars and
 * menubar on logout. If you want to save additional data, reimplement
 * @ref saveProperties() and (to read them again on next login) @ref
 * readProperties(). To save special data about your data, reimplement
 * @ref saveGlobalProperties(). To warn user that application or
 * windows have unsaved data on close or logout, reimplement @ref
 * queryClose() and/or @ref queryExit().
 *
 * There is also a macro RESTORE which can restore all your windows
 * on next login.
 *
 *  Note that a KMainWindow per-default is created with the
 *  WDestructiveClose flag, i.e. it is automatically destroyed when the
 *  window is closed. If you do not want this behavior, specify 0 as
 *  widget flag in the constructor.
 *
 * @see KApplication
 * @short KDE top level main window
   @author Reginald Stadlbauer (reggie@kde.org) Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by Sven Radej (radej@kde.org)

 */

class KMainWindow : public QMainWindow, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    friend class KMWSessionManaged;
    friend class KTMainWindow;
    Q_OBJECT

public:
    /**
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KMainWindow becomes sort of a
     * secondary window.
     *
     * @param name The object name. For session management to work
     * properly, all main windows in the application have to have a
     * different name. When passing 0 (the default), KMainWindow will create
     * such a name for you. So simply never pass anything else ;-)
     *
     * @param f Specify the widget flags. The default is
     * WType_TopLevel and WDestructiveClose.  TopLevel indicates that a
     * main window is a toplevel window, regardless of whether it has a
     * parent or not. DestructiveClose indicates that a main window is
     * automatically destroyed when its window is closed. Pass 0 if
     * you do not want this behavior.
     *
     * KMainWindows must be created on the heap with 'new', like:
     *  <pre> KMainWindow *kmw = new KMainWindow (...</pre>
     *
     * This constructor automatically creates the "Show Toolbar" action
     **/
    KMainWindow( QWidget* parent = 0, const char *name = 0, WFlags f = WType_TopLevel | WDestructiveClose );

    /*
     * this constructor exists for BIC reasons. It allows specifying, if the "Show Toolbar" action should be created
     * automatically. The default is going to be true in KDE 4
     **/
    KMainWindow( QWidget* parent, const char *name, WFlags f, bool createToolbarAction);


    /**
     * Destructor.
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
     * <pre>
     * KPopupMenu *help = helpMenu( <myTextString> );
     * menuBar()->insertItem( i18n("&Help"), help );
     * </pre>
     *
     * @param aboutAppText The string that is used in the application
     *        specific dialog box. If you leave this string empty the
     *        information in the global @ref KAboutData of the
     *        application will be used to make a standard dialog box.
     *
     * @param showWhatsThis Set this to false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     */
    KPopupMenu* helpMenu( const QString &aboutAppText = QString::null,
			  bool showWhatsThis = TRUE );

    /**
     * Returns the help menu. Creates a standard help menu if none exists yet.
     *
     * It contains entries for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box. You must create the application
     * specific dialog box yourself. When the "About application"
     * menu entry is activated, a signal will trigger the
     * @ref showAboutApplication slot. See @ref showAboutApplication for more
     * information.
     *
     * Example (adding a help menu to your application):
     * <pre>
     * menuBar()->insertItem( i18n("&Help"), customHelpMenu() );
     * </pre>
     *
     * @param showWhatsThis Set this to @p false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     */
    KPopupMenu* customHelpMenu( bool showWhatsThis = TRUE );

    /**
     * @sect Session Management
     *
     * Try to restore the toplevel widget as defined by the number (1..X).
     *
     * If the session did not contain so high a number, the configuration
     * is not changed and @p false returned.
     *
     * That means clients could simply do the following:
     * <pre>
     * if (kapp->isRestored()){
     *   int n = 1;
     *   while (KMainWindow::canBeRestored(n)){
     *     (new childMW)->restore(n);
     *     n++;
     *   }
     * } else {
     * // create default application as usual
     * }
     * </pre>
     * Note that @ref QWidget::show() is called implicitly in restore.
     *
     * With this you can easily restore all toplevel windows of your
     * application.
     *
     * If your application uses different kinds of toplevel
     * windows, then you can use @ref KMainWindow::classNameOfToplevel(n)
     * to determine the exact type before calling the childMW
     * constructor in the example from above.
     *
     * If your client has only one kind of toplevel widgets (which should
     * be pretty usual) then you should use the RESTORE-macro:
     *
     * <pre>
     * if (kapp->isRestored())
     *   RESTORE(childMW)
     * else {
     * // create default application as usual
     * }
     * </pre>
     *
     * The macro expands to the term above but is easier to use and
     * less code to write.
     *
     * @see restore()
     * @see classNameOfToplevel()
     *
     */
    static bool canBeRestored( int number );

    /**
     * Returns the @ref className() of the @p number of the toplevel window which
     * should be restored.
     *
     *  This is only useful if your application uses
     * different kinds of toplevel windows.
     */
    static const QString classNameOfToplevel( int number );

    /**
     * Restore the session specified by @p number.
     *
     * Returns @p false if this
     * fails, otherwise returns @p true and shows the window.
     * You should call @ref canBeRestored() first.
     * If @p show is true (default), this widget will be shown automatically.
     */
    bool restore( int number, bool show = TRUE );

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
     *
     * @param xmlfile The local xmlfile (relative or absolute)
     * @param _conserveMemory Specify whether createGUI() should call
     *     @ref KXMLGuiClient::conserveMemory() to free all memory
     *     allocated by the @ref QDomDocument and by the KXMLGUIFactory.
     */
    void createGUI( const QString &xmlfile = QString::null, bool _conserveMemory = TRUE );

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
     *
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
     *  If there is no
     * status bar yet one will be created.
     */
    KStatusBar *statusBar();

    /**
     * List of members of KMainWindow class.
     */
    static QPtrList<KMainWindow>* memberList;

    /**
     * Returns a pointer to the toolbar with the specified name.
     * This refers to toolbars created dynamically from the XML UI
     * framework.  If the toolbar does not exist one will be created.
     *
     * @param name The internal name of the toolbar. If no name is
     *             specified "mainToolBar" is assumed.
     *
     * @return A pointer to the toolbar or @p 0L if it doesn't exist
     **/
    KToolBar *toolBar( const char *name=0 );

    /**
     * @return An iterator over the list of all toolbars for this window.
     */
    QPtrListIterator<KToolBar> toolBarIterator();

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
     */
    void setAutoSaveSettings( const QString & groupName = QString::fromLatin1("MainWindow"),
                              bool saveWindowSize = true );

    /**
     * Disable the auto-save-settings feature.
     * You don't normally need to call this, ever.
     */
    void resetAutoSaveSettings();

    /**
     * Read settings for statusbar, menubar and toolbar from their respective
     * groups in the config file and apply them.
     *
     * @param config Config file to read the settings from.
     * @param groupName Group name to use. If not specified, the last used 
     * group name is used.
     */
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

    // why do we support old gcc versions? using KXMLGUIBuilder::finalizeGUI;
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
     * @return the size the mainwindow should have so that the central 
     * widget will be of @p size.
     */
    QSize sizeForCentralWidgetSize(QSize size);

public slots:
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
     * to open <appName>/index.html.
     *
     * This method is intended for use by a help button in the toolbar or
     * components outside the regular help menu. Use @ref helpMenu() when you
     * want to provide access to the help system from the help menu.
     *
     * Example (adding a help button to the first toolbar):
     *
     * <pre>
     * KIconLoader &loader = *KGlobal::iconLoader();
     * QPixmap pixmap = loader.loadIcon( "help" );
     * toolBar(0)->insertButton( pixmap, 0, SIGNAL(clicked()),
     *   this, SLOT(appHelpActivated()), true, i18n("Help") );
     * </pre>
     *
     */
    void appHelpActivated( void );

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file
     */
    virtual void slotStateChanged(const QString &newstate);

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file,
     * can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
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
     * @internal. Used for the auto-save-settings feature.
     */
    void setSettingsDirty();

   /**
    * Returns a pointer to the mainwindows action responsible for the toolbars menu
    */
    KToolBarMenuAction *toolBarMenuAction();

protected:
    void paintEvent( QPaintEvent* e );
    void childEvent( QChildEvent* e);
    void resizeEvent( QResizeEvent* e);
    /**
     * Reimplemented to call the queryClose() and queryExit() handlers.
     *
     * We recommend that you reimplement the handlers rather than @ref closeEvent().
     * If you do it anyway, ensure to call the base implementation to keep
     * @ref queryExit() running.
     */
    virtual void closeEvent ( QCloseEvent *);
    /**
       Called before the very last window is closed, either by the
       user or indirectly by the session manager.

       It is not recommended to do any user interaction in this
       function other than indicating severe errors. Better ask the
       user on @ref queryClose() (see below).

       However, @ref queryExit() is useful to do some final cleanups. A
       typical example would be to write configuration data back.

       Note that the application may continue to run after @ref queryExit()
       (the user may have cancelled a shutdown). The purpose of
       @ref queryExit() is purely to prepare the application (with possible
       user interaction) so it can safely be closed later (without
       user interaction).

       If you need to do serious things on exit (like shutting a
       dial-up connection down), connect to the signal
       @ref KApplication::shutDown().

       Default implementation returns @p true. Returning @p false will
       cancel the exiting. In the latter case, the last window will
       remain visible.

       @see queryClose()
     */
    virtual bool queryExit();

    /**
       Called before the window is closed, either by the user or indirectly by
       the session manager.

       The purpose of this function is to prepare the window in a way that it is
       safe to close it, i.e. without the user losing some data.

       Default implementation returns true. Returning @p false will cancel
       the closing.

       Reimplement this function to prevent the user from losing data.
       Example:
       <pre>

           switch ( KMessageBox::warningYesNoCancel( this,
				   i18n("Save changes to Document Foo?")) ) {
           case KMessageBox::Yes :
             // save document here. If saving fails, return FALSE;
             return TRUE;
           case KMessageBox::No :
             return TRUE;
           default: // cancel
             return FALSE;

    </pre>

   @see queryExit()

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
     * This function is similar to @ref saveProperties() but is only called for
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
     * The counterpart of @ref saveGlobalProperties().
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
    * This slot does nothing.
    *
    * It must be reimplemented if you want
    * to use a custom About Application dialog box. This slot is
    * connected to the About Application entry in the menu returned
    * by @ref customHelpMenu.
    *
    * Example:
    * <pre>
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
    * </pre>
    */
    virtual void showAboutApplication();

private slots:
   /**
    * Called when the app is shutting down.
    */
    void shuttingDown();

private:
    QMenuBar *internalMenuBar();
    QStatusBar *internalStatusBar();
    KHelpMenu *mHelpMenu, *helpMenu2;
    KXMLGUIFactory *factory_;
    QPtrList<KToolBar> toolbarList;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KMainWindowPrivate *d;
    void initKMainWindow(bool createToolbarAction,const char *name);
};

#define RESTORE(type) { int n = 1;\
    while (KMainWindow::canBeRestored(n)){\
      (new type)->restore(n);\
      n++;}}


#endif
