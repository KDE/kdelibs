#include <kmenubar.h>
#include <qpopmenu.h>
#include <kstatusbar.h>
#include "kdocktoolbar.h"


class testWindow  : public KTopLevelWidget
{
  Q_OBJECT

public:
  testWindow (QWidget *parent=0, const char *name=0);
  ~testWindow ();
    
 public slots:
 void slotNew();
  void slotPrint();
  void slotReturn();
  void slotSave();
  void slotList(const char *str);
  void slotOpen();
  void slotCompletion();
  void slotCompletionsMenu(int id);
  void slotHide2 ();
  void slotHide1 ();
  void slotLined ();
  void slotExit();
  void slotFrame();
  void slotListCompletion();
  void slotWidth();
  void slotToggle(bool);
  void slotClearCombo();
  void slotInsertListInCombo ();
  void slotMakeItem3Current ();
protected:
  KMenuBar *menuBar;
  QPopupMenu *fileMenu;
  QPopupMenu *itemsMenu;
  QPopupMenu *completions;
  QPopupMenu *toolBarMenu;
  KStatusBar *statusBar;
  KDockToolBar *toolBar;
  KToolBar *tb1;
  int toolbar1;
  int toolbar2;
  bool lineL;
  bool exitB;
  bool greenF;
  QMultiLineEdit *widget;
};




    

        
