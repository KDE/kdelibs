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
         * It is meant be used as a toplevel (parent-less) widget and
         * manages the geometry for all its children, including your
         * main widget (set with @ref setView ).
         * @short KDE top level widget
         */
class KTopLevelWidget : public QWidget {
	Q_OBJECT
  
	friend class KToolBar;
  
public:
        /**
         * constructor
         */
	KTopLevelWidget( const char *name = NULL );
        /**
         * destructor
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
         * and status bar.
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

	int view_top;
	int view_bottom;
	int view_left;
	int view_right;
  
protected:
	void resizeEvent( QResizeEvent *e);
	void focusInEvent ( QFocusEvent *);
	void focusOutEvent ( QFocusEvent *);

protected slots:
        /**
         * Updates child widget geometry.
         * This is automatically called when the widget is created,
         * new components are added or the widget is resized.
         *
         * Override it if you intend to manage the children yourself.
         * You normally do not need to do this.
         */
	void updateRects();

  void setFixed();

private:
	QList <KToolBar> toolbars;
	QWidget *kmainwidget;
	KMenuBar *kmenubar;
	KStatusBar *kstatusbar;
	QFrame *kmainwidgetframe;

        /**
         * stores the width of the view frame
         */
	int borderwidth;

  int reclvl;	/* recursion level of updateRects()
		 * this will prevent from a program to go wild
		 * if an indefinite recursion occures.
		 */
  bool nores;	/* do not call updateRects() from resizeEvent()
		 * if this flag is set
		 */

  QSize newmins, newmaxs;
};

#endif
