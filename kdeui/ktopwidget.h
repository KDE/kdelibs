#ifndef _KWIDGET_H
#define _KWIDGET_H

#include <stdlib.h>
#include <qwidget.h>
#include <qlist.h>
#include <ktoolbar.h>
#include <kmenubar.h> 
#include <kstatusbar.h>


class KTopLevelWidget : public QWidget {
	Q_OBJECT
  
	friend class KToolBar;
  
public:
	KTopLevelWidget( const char *name = NULL );
	~KTopLevelWidget();
	
	int addToolBar( KToolBar *toolbar, int index = -1 );
	void setView( QWidget *view, bool show_frame = TRUE );
	void setMenu( KMenuBar *menu );
	void setStatusBar( KStatusBar *statusbar );
	void enableStatusBar( KStatusBar::BarStatus stat = KStatusBar::Toggle );
	void enableToolBar( KToolBar::BarStatus stat = KToolBar::Toggle,
						int ID = 0 ); 

	/// Set the width of the view frame.
	/** If you request a frame around your view with setView(...,TRUE),
	    you can use this function to set the border width of the frame.
	    The default is 1 pixel. You should call this function before
	    setView();
	*/
	void setFrameBorderWidth(int );

	KToolBar *toolBar( int ID = 0 );
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
	void updateRects();

private:
	QList <KToolBar> toolbars;
	QWidget *kmainwidget;
	KMenuBar *kmenubar;
	KStatusBar *kstatusbar;
	QFrame *kmainwidgetframe;

	/// stores the width of the view frame
	int borderwidth;
};

#endif
