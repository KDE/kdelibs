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
	int w;
  
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

private:
	QList <KStatusBarItem> labels;

protected:
	void drawContents ( QPainter * );
	void resizeEvent( QResizeEvent* );
	void init();
	void updateRects( bool resize = FALSE );

protected slots:

signals:
  
};


#endif
