#ifndef _KWIDGET_H
#define _KWIDGET_H

#include <stdlib.h>
#include <qwidget.h>
#include <qlist.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>

         /**
          * A Widget that provides toolbars, a status line and a frame.
          * It should be used as a toplevel (parent-less) widget and
          * manages the geometry for all its children, including your
          * main widget (set with @ref setView ).
          * Normaly, you will inherit from KTopLevelWidget (further: KTW). You have
          * follow some simple rules. First, @ref updateRects is the function that
          * calculates the layout of all elements (toolbars, statusbar, main widget, etc).
          * It is called from @ref resizeEvent, and signals that indicate changing position
          * of toolbars and menubar are connected to it. If you reimplement
          * resizeEvent you have to call this function. KTW now handles fixed-size main views
          * propperly. Just setFixedSize on your main widget.
          * @short KDE top level widget
          * @author Maintained by Sven Radej <a9509961@unet.univie.ac.at>
          */
class KTopLevelWidget : public QWidget {
    Q_OBJECT

        friend class KToolBar;

public:
    /**
     * Constructor.
     */
    KTopLevelWidget( const char *name = NULL );
    /**
     * Destructor. Do not forget to delete all toolbars in destructor of your
     * reimplementation
     */
    ~KTopLevelWidget();

    /**
     * Add a toolbar to the widget.
     * A toolbar added to this widget will be automatically laid out
     * by it.
     *
     * The toolbar must have been created with this instance of
     * KTopLevelWidget as its parent.
     */
    int addToolBar( KToolBar *toolbar, int index = -1 );

    /**
     * Set the main client widget.
     * This is the main widget for your application; it's geometry
     * will be automatically managed by KTopLevelWidget to fit the
     * client area, constrained by the positions of the menu, toolbars
     * and status bar. It can be fixed-width.
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
     * Sets the main menubar for this widget.
     * Unless a main menu is added, its geometry will not be properly
     * taken into account when laying out the other children of this
     * widget.
     *
     * Only one menubar can be added to the widget and it must have
     * been created with this instance of KTopLevelWidget as its parent.
     */
    void setMenu( KMenuBar *menu );

    /**
     * Sets the status bar for this widget.
     * For the status bar to be automatically laid out at the bottom
     * of the widget, this must be called.
     *
     * Only once status bar can be added to the widget and it must have
     * been created with this instance of KTopLevelWidget as its parent.
     */
    void setStatusBar( KStatusBar *statusbar );

    /**
     * This function saves window properties: geometry, state and position of
     * toolbars, menubar and statusbar to application's config file. If parametar
     * 'global' is TRUE, properties will be saved under group name [WindowProperties].
     * If 'globals' is FALSE, group name will be the window-title. You must set window-title
     * (with setCaption) in that case, otherwise function use global group. Toolbar properties
     * are saved in the order they are in the internal list of toolbars.
     * Properties of elements that do not exist will be not saved (Surprise!). You
     * call to this function in your destructor - normaly before you delete toolbars
     * and menubar. You may add other entrys to this group.
     */
    bool saveProperties (bool global = TRUE);

    /**
     * This function reads properties for KTW. If you want to load
     * specific properties, be sure to set window caption first. If parametar 'global'
     * is FALSE, first the group-name equal to the window-title is searched for, and if
     * it does not exist, group [WindowProperties] is searched for. If parametar 'global' is
     * TRUE only group [WindowProperties] is used.
     * You must create and set statusbar (@ref setStatusBar ), create and add all toolbars
     * (@ref addToolBar ) and create and set menubar (@ref setMenu ) before you call
     * this function. All elements will be handled according to configuration, and
     * window will be resized and moved. Window will be not shown. Function returns TRUE
     * on success and FALSE on failure. If no window-title is set (with setCaption) and
     * function was called with FALSE, it will fail.
     */
    bool readProperties (bool global = FALSE);
    
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
     * If you request a frame around your view with setView(...,TRUE),
     * you can use this function to set the border width of the frame.
     * The default is 1 pixel. You should call this function before
     * setView().
     */
    void setFrameBorderWidth( int );

    /**
     * Returns a pointer to a toolbar in the toolbar list.
     * The toolbar must have been added to the widget with
     * addToolBar.
     *
     * If no toolbars were added, NULL is returned.
     */
    KToolBar *toolBar( int ID = 0 );

    /**
     * Returns a pointer to the status bar.
     * The status bar must have been added to the widget with
     * addStatusBar.
     *
     * If no status bar was added, NULL is returned.
     */
    KStatusBar *statusBar();

    /**
     * Shows toplevel widget. Reimplemented from QWidget, and calls
     * @ref updateRects. Therefore, it is now enough just to show KTW.
     */
    virtual void show ();

    /**
     * Distance from top of window to top of main view,
     * Computed in @ref updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_top;

    /**
     * Distance from top of window to bottom of main view.
     * Computed in @ref updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_bottom;

    /**
     * Distance from left edge of window to left border of main view.
     * Computed in @ref updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_left;

    /**
     * Distance from left edge of window to right edge of main view.
     * Computed in @ref updateRects. Changing of this variable
     * has no effect. Avoid using it, it might be removed in future.
     */
    int view_right;

protected:
    /**
     * Default implementation calls @ref updateRects if main widget is resizable. If
     * mainWidget is not resizable  it does
     * nothing. You shouldn't need to override this function.
     */
    void resizeEvent( QResizeEvent *e);
    /**
     * Default implementation just calls repaint (FALSE);
     */
    void focusInEvent ( QFocusEvent *);
    /**
     * Default implementation just calls repaint (FALSE);
     */
    void focusOutEvent ( QFocusEvent *);
    
protected slots:
    /**
     * Updates child widget geometry. This function is now virtual
     * This is automatically called when the widget is created,
     * new components are added or the widget is resized, or showed.
     * updateRects handles fixed-size widgets propperly.
     *
     * Override it if you intend to manage the children yourself.
     * You normally do not need to do this.
     */
    virtual void updateRects();

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
     * Main widget. If you want fixed-widget just call setFixedSize(w.h)
     * on your mainwidget.
     * You should not setFixedSize on KTopLevelWidget..
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
};

#endif
