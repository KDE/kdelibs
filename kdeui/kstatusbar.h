#ifndef _KSTATUSBAR_H
#define _KSTATUSBAR_H

#include <qframe.h>
#include <qstring.h> 
#include <qlist.h>
#include <qlabel.h> 


class KStatusBarItem : public QLabel {
	Q_OBJECT

public:
	KStatusBarItem( char *text, int ID, QWidget *parent=NULL,
		char *name=NULL );
	int ID();

	/// store width and height of the KStatusBarItem
	int w,h;
  
protected:

private:
	int id;

protected slots:
  
signals:

};

class KStatusBar : public QFrame {
	Q_OBJECT
    
public:
	enum BarStatus{ Toggle, Show, Hide };
	enum Position{Top, Left, Bottom, Right, Floating};
	enum InsertOrder{LeftToRight, RightToLeft};

	KStatusBar(QWidget *parent = NULL, char *name = NULL );
	~KStatusBar();
	
	bool enable( BarStatus stat );
	
	/**

		Insert field into the status bar. When inserting the item send the
		longest text you expect to go into the field as the first argument.
		The field is sized to accomodate this text. However, the last field
		inserted is always stretched to fit the window width.
		
	**/
	
	int insertItem( char *text, int ID );
	
	/**
		Change the text in a status bar field. The field is not resized !!!
	**/
	
	void changeItem( char *text, int id );

	/** 
	        If order is KStatusBar::LeftToRight the field are inserted from left
                to right, in particular the last field ist streched to the right
                border of the app. If order is KStatusBar::RightToLeft the fields
		are inserted from the right.
	**/

	void setInsertOrder(InsertOrder order);

	/**     Sets the alignment of a field. By default all fields are aligned left.
	 */

	void setAlignment(int id, int align);

	/**     Sets the Height of the StatusBar
	 */

	void setHeight(int);

	/**     Sets the border width of the status bar seperators and frame.
	 */

	void setBorderWidth(int);

private:
	QList <KStatusBarItem> labels;
	InsertOrder insert_order;
	int fieldheight, borderwidth;

protected:
	void drawContents ( QPainter * );
	void resizeEvent( QResizeEvent* );
	void init();
	void updateRects( bool resize = FALSE );

protected slots:

signals:
  
};


#endif
