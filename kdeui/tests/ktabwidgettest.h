#ifndef KTABWIDGETTEST_H
#define KTABWIDGETTEST_H

#include <kiconloader.h>
#include <qwidget.h>
#include <ktabwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3popupmenu.h>
#include <stdlib.h>
#include <kvbox.h>
#include <time.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>

class Test : public KVBox
{
  Q_OBJECT
public:
  Test( QWidget* parent=0, const char *name =0 );

private slots:
  void addTab();
  void removeCurrentTab();
  void toggleLeftButton(bool);
  void toggleRightButton(bool);
  void toggleLeftPopup(bool);
  void toggleRightPopup(bool);
  void toggleTabPosition(bool);
  void toggleTabShape(bool);
  void toggleCloseButtons(bool);
  void toggleLabels(bool);

  void currentChanged(QWidget*);
  void contextMenu(QWidget*, const QPoint&);
  void tabbarContextMenu(const QPoint&);
  void testCanDecode(const QDragMoveEvent *, bool & /* result */);
  void receivedDropEvent( QDropEvent* );
  void initiateDrag( QWidget * );
  void receivedDropEvent( QWidget *, QDropEvent * );
  void mouseDoubleClick(QWidget*);
  void mouseMiddleClick(QWidget*);
  void movedTab( int, int );

  void leftPopupActivated(int);
  void rightPopupActivated(int);
  void contextMenuActivated(int);
  void tabbarContextMenuActivated(int);

private:
  KTabWidget*     mWidget;
  int             mChange;

  QCheckBox *     mLeftButton;
  QCheckBox *     mRightButton;
  QCheckBox *     mTabsBottom;

  QToolButton*    mLeftWidget;
  QToolButton*    mRightWidget;

  QMenu*     mLeftPopup;
  QMenu*     mRightPopup;
  QMenu*     mTabbarContextPopup;
  QMenu*     mContextPopup;
  QWidget*        mContextWidget;
};


#endif
