#ifndef _KWIDGET_H
#define _KWIDGET_H




#include <stdlib.h>
#include <qwidget.h>
#include <qlist.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kconfig.h>

 // $Id$
 // $Log$

 /**
  * A Widget that provides toolbars, a status line and a frame.
  * It should be used as a toplevel (parent-less) widget and
  * manages the geometry for all its children, including your
  * main widget.
  *
  * Normally, you will inherit from KTopLevelWidget (further: KTW).
  * Then you must construct (or use some existing) widget that will be
  * your main view. You set that main view only once with @ref #setView .
  *
  * Adding and controll of toolbar(s) is done  with function @ref #toolBar .
  * Menubar is added or handled with function @ref #menuBar , and statusbar
  * with function @ref #statusBar .
  * Toolbars and menubars are handled internaly and you must not delete them on
  * exit. Function bool @ref #confirmExit () is called from closeEvent.
  * Reimplement @ref #confirmExit if you need to control over close events.
  *
  *
  * @ref #updateRects is the function that calculates the layout of all
  * elements (toolbars, statusbar, main widget, etc). It is called from
  * @ref #resizeEvent, and signals that indicate changing position
  * of toolbars and menubar are connected to it. If you reimplement
  * resizeEvent you have to call this function. KTW now handles fixed-size
  * and Y-fixed main views properly. Just @ref QWidget::setFixedSize or
  * @ref QWidget::setFixedHeight on your main view. You can change it runtime,
  * the changes will take effect on next @ref updateRects call. Do not set
  * fixed size on window! You may set minimum or maximum size on window, but
  * only if main view is freely resizable. Minimum width can also be set if main
  * view is Y-fixed.
  * @short KDE top level widget
  * @author Stephan Kullow (coolo@kde.org) Maintained by Sven Radej (sven@lisa.exp.univie.ac.at)
  */
class KTopLevelWidget : public QWidget {
    Q_OBJECT

        friend class KToolBar;

public:
    /**
     * Constructor.
     */
    KTopLevelWidget( const char *name = 0L );
    /**
     * Destructor.
     */
    ~KTopLevelWidget();

    /**
      * THIS FEATURE IS OBSOLETE AND WILL BE REMOVED.
      * DO NOT USE! Use toolBar() instead (Matthias)
      *
      */
    int addToolBar( KToolBar *toolbar, int index = -1 );

    /**
     * Set the main client widget.
     * This is the main widget for your application; it's geometry
     * will be automatically managed by KTopLevelWidget to fit the
     * client area, constrained by the positions of the menu, toolbars
     * and status bar. It can be fixed-width or Y-fixed.
     *
     * Only one client widget can be handled at a time; multiple calls
     * of setView will cause only the last widget to be added to be
     * properly handled.
     *
     * The widget must have been created with this instance of
     * KTopLevelWidget as its parent.
     */
    void setView( QWidget *view, bool show_frame = TRUE );

    /**
      * THIS FEATURE IS OBSOLETE AND WILL BE REMOVED.
      * DO NOT USE! Use menuBar() instead (Matthias)
      *
      */
    void setMenu( KMenuBar *menu );

    /**
      * THIS FEATURE IS OBSOLETE AND WILL BE REMOVED.
      * DO NOT USE! Use statusBar() instead (Matthias)
     */
    void setStatusBar( KStatusBar *statusbar );

    
    /**
     *Enable or disable the status bar.
     */
    void enableStatusBar( KStatusBar::BarStatus stat = KStatusBar::Toggle );

    /**
     * Enable or disable the toolbar with the ID specified.
     * If no ID is specified, the default ID is 0.
     */
    void enableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle,
                        int ID = 0 );

    /**
     * Set the width of the view frame.
     * If you request a frame around your view with @ref #setView (...,TRUE),
     * you can use this function to set the border width of the frame.
     * The default is 1 pixel. You should call this function before
     * @ref #setView().
     */
    void setFrameBorderWidth( int );

    /**
     * Returns a pointer to the toolbar with the specified ID. 
     * If there is no such tool bar yet, it will be generated.
     * Do not delete toolbars.
     */
    KToolBar *toolBar( int ID = 0 );

    /**
     * Returns a pointer to the menu bar. If there is no
     * menu bar yet, it will be generated. Do not delete menubar.
     */
    KMenuBar *menuBar();

    /**
     * Returns a pointer to the status bar. If there is no
     * status bar yet, it will be generated.
     */
    KStatusBar *statusBar();

    /**
     * Shows toplevel widget. Reimplemented from QWidget, and calls
     * @ref #updateRects . Therefore, it is enough just to show KTW.
     */
    virtual void show ();

    /**
     * Distance from top of window to top of main view,
     * Computed in @ref #updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_top;

    /**
     * Distance from top of window to bottom of main view.
     * Computed in @ref #updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_bottom;

    /**
     * Distance from left edge of window to left border of main view.
     * Computed in @ref #updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_left;

    /**
     * Distance from left edge of window to right edge of main view.
     * Computed in @ref #updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_right;



  /**
   * Try to restore the toplevel widget as defined number (1..X)
   * If the session did not contain that high number, the configuration
   * is not changed and False returned.
   * 
   * That means clients could simply do the following:
   * <pre>
   * if (kapp->isRestored()){
   *   int n = 1;
   *   while (KTopLevelWidget::canBeRestored(n)){
   *     (new childTLW)->restore(n);
   *     n++;
   *   }
   * } else {
   * // create default application as usual
   * }
   * </pre>
   * Note that "show()" is called implicit in restore.
   *
   * With this you can easily restore all toplevel windows of your
   * application.  
   *
   * If your application uses different kinds of toplevel
   * windows, then you can use KTopLevelWidget::classNameOfToplevel(n)
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
   *(Matthias) 
   */
  static bool canBeRestored(int number);


  /** Returns the className of the numberth toplevel window which
    * should be restored. This is only usefull if you application uses
    * different kinds of toplevel windows. (Matthias) 
    */
  static const QString classNameOfToplevel(int number);

  /** try to restore the specified number. Returns "False" if this
   * fails, otherwise returns "True" and shows the window
   */
  bool restore(int number);

  /**
    * Tells the session manager wether the window contains
    * unsaved data which cannot be stored in temporary files
    * during saveYourself. Note that this is somewhat bad style.
    * A really good KDE application should store everything in
    * temporary recover files. Kapplication has some nifty support
    * for that.
    *
    * Default is False == No unsaved data.
    */
  void setUnsavedData( bool );


protected:
  /** Default implementation calls @ref #updateRects if main widget
     * is resizable. If mainWidget is not resizable it does
     * nothing. You shouldn't need to override this function.  
     */
    virtual void resizeEvent( QResizeEvent *e);
    /**
     * Default implementation just calls repaint (FALSE);
     */
    virtual void focusInEvent ( QFocusEvent *);
    /**
     * Default implementation just calls repaint (FALSE);
     */
    virtual void focusOutEvent ( QFocusEvent *);

    /** 
      * This is called when the widget is closed.
      * The default implementation will also destroy the
      * widget. (Matthias)
      * If you reimplement this function, delete the widget if
      * event is accepted. (sven)
      */
    virtual void closeEvent ( QCloseEvent *);

  /** KTopLevelWidget has the nice habbit that it will exit the
    * application when the very last KTopLevelWidget is
    * destroyed. Some applications may not want this default
    * behaviour,for example if the application wants to ask the user
    * wether he really wants to quit the application.  This can be
    * achived by overloading the queryExit() method.  The default
    * implementation simply returns TRUE, which means that the
    * application will be quitted. FALSE will cancel the exiting
    * process. (Matthias) */
    virtual bool queryExit();

  /** Save your instance-specific properties.
   * You MUST NOT change the group of the kconfig object,
   * since KTW uses one group for each window.
   * Please overload these function in childclasses. 
   *
   * Note that any interaction or X calls are forbidden
   * in these functions!
   *
   * (Matthias) */
  virtual void saveProperties(KConfig*){};
  /**
  * Read your instance-specific properties.
  */
  virtual void readProperties(KConfig*){};


    
protected slots:
    /**
     * Updates child widget geometry. This function is now virtual
     * This is automatically called when the widget is created,
     * new components are added or the widget is resized, or showed.
     * updateRects handles fixed-size and Y-fixed widgets properly.
     *
     * Override it if you intend to manage the children yourself.
     * You normally do not need to do this.
     */
    virtual void updateRects();

 private slots:
 /**
   * React on the request of the session manager (Matthias)
   */
    void saveYourself(); 



private:

    /**
     * List of members of KTopLevelWidget class
     */
  static QList<KTopLevelWidget>* memberList;

    /**
     * List of toolbars.
     */
    QList <KToolBar> toolbars;

    /**
     * Main widget. If you want fixed-size or Y-fixed widget just call
     * setFixedSize(w.h) or setFixedWidth (h) on your mainwidget.
     * You should not setFixedSize on KTopLevelWidget.
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

    // this is for binary compatibility
#ifndef KTW_BINCOMPAT
    bool usesNewStyle;
#endif
  // Matthias
protected:
  void savePropertiesInternal (KConfig*, int);
  bool readPropertiesInternal (KConfig*, int);
};


#define RESTORE(type) { int n = 1;\
    while (KTopLevelWidget::canBeRestored(n)){\
      (new type)->restore(n);\
      n++;}}

#endif
