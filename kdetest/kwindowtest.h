#ifndef testwindow_h
#define testwindow_h

#include <qpopupmenu.h>
#include <qtimer.h>
#include <qprogressbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <ktopwidget.h>

class QMultiLineEdit;

class testWindow  : public KTopLevelWidget
{
    Q_OBJECT

public:
    testWindow (QWidget *parent=0, const char *name=0);
    ~testWindow ();
    
public slots:
    void beFixed();
    void beYFixed();

    void slotNew();
    void slotPrint();
    void slotReturn();
    void slotSave();
    void slotList(const char *str);
    void slotOpen();
    void slotCompletion();
    void slotCompletionsMenu(int id);
    void slotHide2 ();
    void slotInsertClock();
    void slotHide1 ();
    void slotLined ();
    void slotImportant ();
    void slotExit();
    void slotFrame();
    void slotListCompletion();
    void slotMessage(int, bool);
    void slotToggle(bool);
    void slotClearCombo();
    void slotGoGoGoo();
    void slotInsertListInCombo ();
    void slotMakeItem3Current ();
    void slotToggled(int);
protected:
    KMenuBar *menuBar;
    QPopupMenu *fileMenu;
    QPopupMenu *itemsMenu;
    QPopupMenu *completions;
    QPopupMenu *toolBarMenu;
    QPopupMenu* helpMenu;
    KStatusBar *statusBar;
    KToolBar *toolBar;
    KToolBar *tb1;
    int toolbar1;
    int toolbar2;
    bool lineL;
    bool exitB;
    bool greenF;
    bool ena;
    QMultiLineEdit *widget;
    QTimer *timer;
    QProgressBar *pr;
    KRadioGroup *rg;
};
#endif

