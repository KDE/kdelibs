#ifndef TAB_H
#define TAB_H

#include <kiconloader.h>
#include <qwidget.h>
#include <qtimer.h>
#include "../ktabwidget.h"
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <stdlib.h>
#include <qvbox.h>
#include <time.h>

class Test : public QVBox
{
  Q_OBJECT
public:
  Test( QWidget* parent=0, const char *name =0 );

private slots:
  void timerDone();
  void addTab();
  void removeCurrentTab();
  void toggleLeftButton(bool);
  void toggleRightButton(bool);
  void toggleLeftPopup(bool);
  void toggleRightPopup(bool);
  void toggleTabPosition(bool);
  void toggleTabShape(bool);
  void leftPopupActivated(int);
  void rightPopupActivated(int);
  void currentChanged(QWidget*);

private:
  KTabWidget*     mWidget;
  int             mChange;

  QPopupMenu*     mLeftPopup;
  QPopupMenu*     mRightPopup;

  typedef QValueList<int> IntList;
  IntList         mList;

  QTimer         *mTimer;
  bool            mRed;
};


#endif
