#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

#include <qframe.h>
#include <qstring.h> 
#include <kbutton.h> 
#include <kpixmap.h>
#include <qlist.h>
#include <qpopmenu.h> 

class KToolBarItem : public KButton {
	Q_OBJECT

public:
	KToolBarItem(QPixmap& pixmap,int ID, QWidget *parent=NULL, char *name=NULL);
	KToolBarItem(QWidget *parent=NULL, char *name=NULL);
	int ID();
	void enable(bool enable);
	void makeDisabledPixmap();
  	QPixmap disabledPixmap;
  	
protected:
	void paletteChange(const QPalette &);

private:
	int id;

	QPixmap enabledPixmap;
	

protected slots:
	void ButtonClicked();
	void ButtonPressed();
	void ButtonReleased();

signals:
	void clicked(int);
	void pressed(int);
	void released(int);
};

class KToolBar : public QFrame {
	Q_OBJECT
    
public:
	enum BarStatus{Toggle, Show, Hide};
	enum Position{Top, Left, Bottom, Right, Floating};
	KToolBar(QWidget *parent=NULL,char *name=NULL);
	~KToolBar();
	int insertItem(KPixmap& pixmap, int ID, bool enabled = TRUE, 
							char *ToolTipText = NULL, int index=-1);
	int insertItem(KPixmap& pixmap, int ID, const char *signal, 
						const QObject *receiver, const char *slot, 
						bool enabled = TRUE, 
						char *tooltiptext = NULL, int index=-1);
	void enableMoving(bool flag = TRUE);
	int insertSeparator(int index=-1);
	void setPos(Position pos);
	Position Pos();
	bool enable(BarStatus stat);
	
	void setItemEnabled( int id, bool enabled );

private:
	QList <KToolBarItem> buttons;
	Position position;
	QPopupMenu *context;
	bool moving;

protected:
	void drawContents ( QPainter *);
	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	void mousePressEvent ( QMouseEvent *);
	void init();
	void updateRects(bool resize = FALSE );

protected slots:
	void ItemClicked(int);
	void ItemPressed(int);
	void ItemReleased(int);
	void ContextCallback(int);

signals:
	void clicked(int);
	void pressed(int);
	void released(int);
};

#endif



