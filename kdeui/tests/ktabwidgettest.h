#ifndef KTABWIDGETTEST_H
#define KTABWIDGETTEST_H

#include <QWidget>
#include <ktabwidget.h>
#include <QLabel>
#include <QPushButton>
#include <stdlib.h>
#include <time.h>
#include <QCheckBox>
#include <QToolButton>

class Test : public QWidget
{
  Q_OBJECT
public:
  Test( QWidget* parent=0 );

private Q_SLOTS:
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
  void toggleScrollButtons(bool);
  void toggleEliding(bool);

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

  void leftPopupActivated(QAction*);
  void rightPopupActivated(QAction*);
  void contextMenuActivated(QAction*);
  void tabbarContextMenuActivated(QAction*);

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
  int mContextWidgetIndex;
};


#endif
