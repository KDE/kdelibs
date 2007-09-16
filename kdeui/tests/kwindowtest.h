#ifndef testwindow_h
#define testwindow_h

#include <QtCore/QTimer>
#include <QtGui/QProgressBar>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kxmlguiwindow.h>

class QTextEdit;

class TestWindow  : public KXmlGuiWindow
{
    Q_OBJECT

public:
    TestWindow (QWidget *parent=0);
    ~TestWindow ();

public Q_SLOTS:
    void beFixed();
    void beYFixed();

    void slotNew();
    void slotPrint();
    void slotReturn();
    void slotSave();
    void slotList(const QString &str);
    void slotOpen();
    void slotCompletion();
    void slotCompletionsMenu(QAction* action);
    void slotInsertClock();
    void slotLined();
    void slotImportant();
    void slotExit();
    void slotFrame();
    void slotListCompletion();
    void slotMessage(int, bool);
    void slotToggle(bool);
    void slotClearCombo();
    void slotGoGoGoo();
    void slotInsertListInCombo();
    void slotMakeItem3Current();
    void slotToggled(QAction* action);

protected:
    QMenu *itemsMenu;
    QMenu *completions;
    KStatusBar *statusBar;
    KToolBar *tb;
    KToolBar *tb1;
    class KLineEdit* testLineEdit;
    class KComboBox* testComboBox;
    QAction* fileNewAction;
    QAction* exitAction;
    bool lineL;
    bool exitB;
    bool greenF;
    bool ena;
    QTextEdit *widget;
    QTimer *timer;
    QProgressBar *pr;
};
#endif

