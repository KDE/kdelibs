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

#ifndef _KTMAINWINDOW_H
#define _KTMAINWINDOW_H

#include <stdlib.h>
#include <qlist.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <kstatusbar.h>
#include <ktoolbar.h>

#include "kxmlgui.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

class KConfig;
class KHelpMenu;
class KMenuBar;
class KTMLayout;
class KTMainWindowPrivate;

/**
 * Top level widget that provides toolbars, a status line and a frame.
 *
 * It should be used as a top level (parent-less) widget.
 * It manages the geometry for all its children, including your
 * main widget.
 *
 * Normally, you will inherit from KTMainWindow (known also as KTMW),
 * then construct (or use some existing) widget as
 * your main view. You can set only one main view.
 *
 * You can add as many toolbars as you like. There can be only one menubar
 * and only one statusbar.
 *
 * The toolbars, menubar, and statusbar can be created by the
 * KTMainWindow and - unlike the old KTMainWindow - may, but do not
 * have to, be deleted by you. KTMainWindow will handle that internally.
 *
 * Height and width can be operated independently from each other. Simply
 * define the minimum/maximum height/width of your main widget and
 * KTMainWindow will take this into account. For fixed size windows set
 * your main widget to a fixed size.
 *
 * Fixed aspect ratios (heightForWidth()) and fixed width widgets are
 * not supported.
 *
 * KTMainWindow will set icon, mini icon and caption, which it gets
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
 *  Note that a KTMainWindow per-default is created with the
 *  WDestructiveClose flag, i.e. it is automatically destroyed when the
 *  window is closed. If you do not want this behavior, specify 0 as
 *  widget flag in the constructor.
 *
 * @see KApplication
 * @short KDE top level main window
   @author Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by Sven Radej (radej@kde.org)

 */

class KTMainWindow : public QWidget, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    Q_OBJECT

        friend class KToolBar;
        friend class KTLWSessionManaged;

public:
    /**
     * Construct a main window.
     *
     * @param name The object name. For session management to work
     * properly, all main windows in the application have to have a
     * different name. When passing 0 (the default), KTMainWindow will create
     * such a name for you. So simply never pass anything else ;-)
     *
     * @param f Specify the widget flags. The default is WDestructiveClose.
     * It indicates that a main window is automatically destroyed when its
     * window is closed. Pass 0 if you do not want this behaviour.
     *
     * KTMainWindows must be created on the heap with 'new', like:
     *  <pre> KTMainWindow *ktmw = new KTMainWindow (...</pre>
     **/
    KTMainWindow( const char *name = 0L, WFlags f = WDestructiveClose );

    /**
     * Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    ~KTMainWindow();

    /**
     * Add a toolbar to the widget.
     *
     * A toolbar added to this widget will be automatically laid out
     * by it.
     *
     * The toolbar must have been created with this instance of
     * KTMainWindow as its parent.
     *
     * Usually you do not need this function. Just refer to a toolbar
     * with @ref toolBar(index) instead and the KTMainWindow will
     * create it for you. Anyway @ref addToolBar() is useful if you want
     * to pass additional arguments to the toolbar's constructor.
     */
    int addToolBar( KToolBar *toolbar, int index = -1 );

    /**
     * Set the main client widget.
     *
     * This is the main widget for your application; it's geometry
     * will be automatically managed by KTMainWindow to fit the
     * client area, constrained by the positions of the menu, toolbars
     * and status bar. It can be fixed-width or Y-fixed.
     *
     * Only one client widget can be handled at a time.  Multiple calls
     * of @ref setView() will cause only the last widget to be added to be
     * properly handled. The layout management will not start before this
     * function has been called. It increases the application start
     * speed to call this function after all bars have been registered. The
     * presence of the view widget is mandatory for the class to operate.
     *
     * The widget must have been created with this instance of
     * KTMainWindow as its parent.
     */
    void setView( QWidget *view, bool show_frame = TRUE );

    /**
     * Retrieve the view widget.
     *
     * @see setView()
     **/
    QWidget *view() const { return kmainwidget; }

    /**
     * Set the given widget as the "indicator" widget found either in
     * the menubar or the statusbar.  The widget may be any size...
     * but keep in mind where it will be going.  A maximum size of
     * 30x22 is recommended.
     *
     * @param indicator The widget to be displayed in the indicator space
     */
    void setIndicatorWidget( QWidget *indicator );

    /**
     * @return A pointer to the indicator widget or 0L if it doesn't
     *         exist.
     */
    QWidget *indicator();

    /**
     * Enable or disable the status bar.
     */
    void enableStatusBar( KStatusBar::BarStatus stat = KStatusBar::Toggle );

    /**
     * Enable or disable the toolbar with the specified @id.
     *
     * If no id is specified, the default id of 0 is used.
     */
    void enableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle,
                        int id = 0 );

    /**
     * Enable or disable the toolbar with the specified name (as
     * determined by the XML UI framework).
     */
    void setEnableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle,
                           const QString& name = QString::fromLocal8Bit("mainToolBar") );

    /**
     * Set the width of the view frame.
     *
     * If you request a frame around your view with @ref setView(...,TRUE),
     * you can use this function to set the border width of the frame.
     * The default is 1 pixel. You should call this function before
     * @ref setView().
     */
    void setFrameBorderWidth( int );

    /**
     * Set the maximum number of wraps for a single block of
     * subsequent non-full-size tool bars.
     *
     * If more wraps would be
     * necessary to properly layout the tool bars the bars will extend
     * outside of the window. This behaviour is helpful when having
     * many toolbars on small displays. Not all toolbars are
     * accessible any longer but at least the main view keeps
     * reasonably visible and is not squished by all the tool
     * bars. Since the user cannot easily distinguish between
     * full-size and non full-size bars, they should not be mixed when
     * using this function.  Technically there is no reason but it is
     * very confusing when some bars automatically wrap (full-size
     * bars) while other extend out of sight.  See @ref KTMLayout for
     * more details. The toolbar wrapping limitation is disabled by
     * default.
     **/
    void setMaximumToolBarWraps(unsigned int wraps);

    /**
     * Retrieve a pointer to the toolbar with the specified ID.
     *
     * If there is no such tool bar yet, it will be generated.
     **/
    KToolBar *toolBar( int ID = 0 );

    /**
     * Retrieve a pointer to the toolbar with the specified name.
     * This refers to toolbars created dynamically from the XML UI
     * framework.  If the toolbar does not exist, then 0L will be
     * returned.
     *
     * @param name The internal name of the toolbar ("mainToolBar",
     *             for instance)
     *
     * @return A pointer to the toolbar or 0L if it doesn't exist
     **/
    KToolBar *toolBar( const QString& name );

    /**
     * @return An iterator over the list of all toolbars for this window.
     */
    QListIterator<KToolBar> toolBarIterator() const;

    /**
     * Retrieve a pointer to the menu bar.
     *
     * If there is no menu bar yet on will be created.
     **/
    KMenuBar *menuBar();

    /**
     * If you constructed a @ref KMenuBar yourself, you must set it with this
     * function.
     *
     * You can use it also if you want to replace an old menu bar
     * with a new one. There can be only one menu bar at a time. After this
     * function is called the layout will be updated.
     * @see menuBar()
     */
    void setMenu (KMenuBar *menuBar);

    /**
     * Retreieve a pointer to the status bar.
     *
     *  If there is no
     * status bar yet one will be created.
     */
    KStatusBar *statusBar();

    /**
     * If you constructed a @ref KStatusBar yourself, you must set it with this
     * function.
     *
     *  You can use it also if you want to replace an old status bar
     * with a new one. There can be only one status bar at a time. After this
     * function is called the layout will be updated.
     * @see statusBar()
     */
    void setStatusBar (KStatusBar *statusBar);


    /**
     * Show the toplevel widget.
     *
     * Reimplemented from @ref QWidget.  Calls
     * @ref updateRects() (i.e. updates layout).
     */
    virtual void show ();

    /**
     * Retrieve the minimum size of the main window.
     */
    QSize sizeHint() const;

    /**
     * Retrieve the geometry of the main view widget. This function is provided
     * for legacy reasons. Do not use it! It might be removed.
     */
    QRect mainViewGeometry() const;

    /**
     * Retrieve the standard help menu which contains entires for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box.
     *
     * Example (adding a standard help menu to your application):
     * <pre>
     * QPopupMenu *help = helpMenu( <myTextString> );
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
    QPopupMenu* helpMenu( const QString &aboutAppText=QString::null,
			  bool showWhatsThis=true );


    /**
     * Retrieve the standard help menu which contains entires for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box. You must create the application
     * specific dialog box yourself. When the "About application"
     * menu entry is activated, a signal will trigger the @ref
     * showAboutApplication slot. See @ref showAboutApplication for more
     * information.
     *
     * Example (adding a help menu to your application):
     * <pre>
     * menuBar()->insertItem( i18n("&Help"), customHelpMenu() );
     * </pre>
     *
     * @param showWhatsThis Set this to false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     */
    QPopupMenu* customHelpMenu( bool showWhatsThis=true );


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
     *   while (KTMainWindow::canBeRestored(n)){
     *     (new childTLW)->restore(n);
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
     * windows, then you can use @ref KTMainWindow::classNameOfToplevel(n)
     * to determine the exact type before calling the childTLW
     * constructor in the example from above.
     *
     * If your client has only one kind of toplevel widgets (which should
     * be pretty usual) then you should use the RESTORE-macro:
     *
     * <pre>
     * if (kapp->isRestored())
     *   RESTORE(childTLW)
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
    static bool canBeRestored(int number);

    /**
     * Retrieve the @ref className() of the @p number th toplevel window which
     * should be restored.
     *
     *  This is only useful if you application uses
     * different kinds of toplevel windows.
     */
    static const QString classNameOfToplevel(int number);

    /**
     * Restore the session specified by @p number.
     *
     * Return @p false if this
     * fails, otherwise returns @p true and shows the window.
     * You should call @ref canBeRestored() first.
     */
    bool restore(int number);

    /**
     * @sect Methods you probably don't need.
     *
     * You probably do not need this.
     *
     *  Anyway, if you are porting code
     * which had been written for the former @ref KTMainWindow you may
     * find the following three boolean "has" functions useful.
     *
     * @return @p true if the menubar exists.
     */
    bool hasMenuBar();

    /**
     * @return @p true if the statusbar exists.
     */
    bool hasStatusBar();

    /**
     * @return @p true if the specified toolbar exists.
     */
    bool hasToolBar( int ID = 0 );

    virtual KXMLGUIFactory *guiFactory();

    /**
     * Create a GUI given a local XML file.  If @ref #xmlfile is NULL,
     * then it will try to construct a local XML filename like
     * appnameui.rc where 'appname' is your app's name.  If that file
     * does not exist, then the XML UI code will only use the global
     * (standard) XML file for the layout purposes.
     *
     * @param xmlfile The local xmlfile (relative or absolute)
     * @param _conserveMemory Specifiy whether createGUI should call @ref conserveMemory to free
     *                        all memory allocated by the @ref QDomDocument .
     */
    virtual void createGUI( const QString &xmlfile = QString::null, bool _conserveMemory = true );

protected:
    /**
     * @sect And now back to methods you might need...
     *
     * Default implementation calls @ref #updateRects if main widget
     * is resizable. If mainWidget is not resizable it does
     * nothing. You shouldn't need to override this function.
     */
    virtual void resizeEvent( QResizeEvent *e );

    /**
     * We need to trap the layout hint. Otherwise we will miss when our
     * view widget or some bar changes the size constrains on it's own.
     */
    virtual bool event(QEvent *);

    /**
     * Default implementation just calls repaint (FALSE); You may
     * reimplement this function if you want to.
     */
    virtual void focusInEvent ( QFocusEvent *);

    /**
     * Default implementation just calls repaint (FALSE); You may
     * reimplement this function if you want to.
     */
    virtual void focusOutEvent ( QFocusEvent *);

    /**
     * Reimplemented to call the queryClose() and queryExit() handlers.
     *
     * We recommend to reimplement the handlers rather than closeEvent().
     * If you do it anyway, ensure to call the base implementation to keep
     * queryExit() running.
     */
    virtual void closeEvent ( QCloseEvent *);

    /**
       Called before the very last window is closed, either by the
       user or indirectely by the session manager.

       It is not recommended to do any user interaction in this
       function other than indicating severe errors. Better ask the
       user on queryClose() (see below).

       However, queryExit() is useful to do some final cleanups. A
       typical example would be to write configuration data back.

       Note that the application may continue to run after queryExit()
       (the user may have cancelled a shutdown). The purpose of
       queryExit() is purely to prepare the application (with possible
       user interaction) so it can safely be closed later (without
       user interaction).

       If you need to do serious things on exit (like shutting a
       dial-up connection down), connect to the signal
       @ref KApplication::shutDown().

       Default implementation returns true. Returning false will
       cancel the exiting. In the latter case, the last window will
       remain visible.

       @see #queryClose
     */
    virtual bool queryExit();

    /**
       Called before the window is closed, either by the user or indirectely by
       the session manager.

       The purpose of this function is to prepare the window in a way that it is
       safe to close it, i.e. without the user losing some data.

       Default implementation returns true. Returning false will cancel
       the closing.

       Reimplement this function to prevent the user from loosing data.
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
     * You MUST NOT change the group of the kconfig object, since
     * KTMainWindow uses one group for each window.  Please
     * reimplement these function in childclasses.
     *
     * Note that no user interaction is allowed
     * in this function!
     *
     */
    virtual void saveProperties(KConfig*){};

   /**
    * Read your instance-specific properties.
    */
   virtual void readProperties(KConfig*){};

   /**
     * Save your application-wide properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * The function is similar to saveProperties() but is only called for
     * the very first main window, regardless how many main window are open.

    * Override it if you need to save other data about your documents on
    * session end. sessionConfig is a config to which that data should be
    * saved. Normally, you don't need this function. But if you want to save
    * data about your documents that are not in opened windows you might need
    * it.
    *
    * Default implementation does nothing.
    */
   virtual void saveGlobalProperties(KConfig* sessionConfig);


    /**
     * The counter-part of saveGlobalProperties(). Reads the application
     * specific properties in again.
     */
   virtual void readGlobalProperties(KConfig* sessionConfig);

public slots:
    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. DO NOT include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const QString &caption );

    /**
     * Makes a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption( const QString &caption );

    /**
     * Opens the help page for the application. The application name is
     * used as a key to determine what to display and the system will attempt
     * to open <appName>/index.html.
     *
     * This method is intended for use by a help button in the toolbar or
     * components outside the regular help menu. Use @ref helpMenu when you
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


protected slots:

   /**
    * This slot must be called whenever the arrangement of the child element
    * has been changed. It needs not to be called for a resize operation.
    * This is handled by Qt layout management.
    */
   virtual void updateRects();

   /**
    * This slot does nothing. It must be reimplemented if you want
    * to use a custom About Application dialog box. This slot is
    * connected to the "About Application" entry in the menu returned
    * by @ref customHelpMenu.
    *
    * Example:
    * <pre>
    *
    * void MyTopLevel::setupInterface()
    * {
    *   ..
    *   menuBar()->insertItem( i18n("&Help"), customHelpMenu() );
    *   ..
    * }
    *
    * void MyTopLevel::showAboutApplication( void )
    * {
    *   <activate your custom dialog>
    * }
    * </pre>
    */
   void showAboutApplication( void );

private slots:
   /**
    * Notices when toolbar is deleted.
    */
   void toolbarKilled();

   /**
    * Notices when menubar is killed.
    */
   void menubarKilled();

   /**
	* Called when the app is shutting down.
	*/
   void shuttingDown();

public:

   /**
    * List of members of KTMainWindow class
    */
   static QList<KTMainWindow>* memberList;

private:
   /**
    * List of toolbars.
    */
   QList <KToolBar> toolbars;

   /**
    * Main widget. If you want fixed-widget just call setFixedSize(w.h)
    * on your mainwidget.
    * You should not setFixedSize on KTMainWindow.
    */
   QWidget *kmainwidget;

   /**
    * Menubar.
    */
   KMenuBar *kmenubar;

   /**
    * Statusbar
    */
   KStatusBar *kstatusbar;

   /**
    * Frame around main widget
    */
   QFrame *kmainwidgetframe;

   /**
    * Stores the width of the view frame
    */
    int borderwidth;

   /**
    * True if toolbars are killed by this destructor.
    */
   bool localKill;


   KTMLayout* layoutMgr;

   KHelpMenu    *mHelpMenu;

   KTMainWindowPrivate *d;

protected:
  void savePropertiesInternal (KConfig*, int);
  bool readPropertiesInternal (KConfig*, int);
};


#define RESTORE(type) { int n = 1;\
    while (KTMainWindow::canBeRestored(n)){\
      (new type)->restore(n);\
      n++;}}

#endif
