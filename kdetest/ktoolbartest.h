#include <kmenubar.h>
#include <qpopmenu.h>
#include <kstatusbar.h>
#include "ktoolbar.h"
#include <qmlined.h>


class testWindow  : public KTopLevelWidget
{
    Q_OBJECT

public:
    testWindow (QWidget *parent=0, const char *name=0);

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
    
protected:
    KMenuBar *menuBar;
    QPopupMenu *fileMenu;
    QPopupMenu *completions;
    QPopupMenu *toolBarMenu;
    KStatusBar *statusBar;
    KToolBar *toolBar;
    KToolBar *tb1;
    int toolbar1;
    int toolbar2;
    QMultiLineEdit *widget;
};




    

        
