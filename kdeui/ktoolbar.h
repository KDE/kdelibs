#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

#include <qframe.h>
#include <qstring.h> 
#include <qpushbt.h> 
#include <kpixmap.h>
#include <qlist.h>
#include <qpopmenu.h> 
#include "kbutton.h"

class KToolbarItem : public KButton {
  Q_OBJECT

public:
  KToolbarItem(QPixmap& pixmap,int ID, QWidget *parent=NULL, char *name=NULL);
  KToolbarItem(QWidget *parent=NULL, char *name=NULL);
  int ID();
  void enable(bool enable);
  void drawButton( QPainter * );
  
protected:
  void calcDisabled();

private:
  int id;
  
  QPixmap EnabledPixmap;
  QPixmap DisabledPixmap;

  protected slots:
    void ButtonClicked();
  void ButtonPressed();
  void ButtonReleased();

signals:
  void clicked(int);
  void pressed(int);
  void released(int);
};

class KToolbar : public QFrame {
  Q_OBJECT
    
public:
  enum BarStatus{Toggle, Show, Hide};
  enum Position{Top, Left, Bottom, Right, Floating};
  KToolbar(QWidget *parent=NULL,char *name=NULL);
  ~KToolbar();
  int insertItem(KPixmap& pixmap, int ID, bool enabled = TRUE, 
		 char *ToolTipText = NULL, int index=-1);
  int insertItem(KPixmap& pixmap, const char *signal, 
		 const QObject *receiver, const char *slot, 
		 bool enabled = TRUE, 
		 char *tooltiptext = NULL, int index=-1);
  int insertSeparator(int index=-1);
  void setPos(Position pos);
  Position Pos();
  bool enable(BarStatus stat);

private:
  QList<KToolbarItem> buttons;
  Position position;
  QPopupMenu *context;

protected:
  void drawContents ( QPainter *);
  void resizeEvent(QResizeEvent*);
  void mousePressEvent ( QMouseEvent *);
  void init();
  void updateRects(bool resize);

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



