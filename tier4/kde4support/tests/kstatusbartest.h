#ifndef test_kstatusbar_h
#define test_kstatusbar_h

#include <kmenubar.h>
#include <kstatusbar.h>
#include <kxmlguiwindow.h>

class QTextEdit;

class testWindow  : public KXmlGuiWindow
{
    Q_OBJECT

public:
    testWindow (QWidget *parent=0);
    ~testWindow ();

public Q_SLOTS:
    void slotPress(int i);
    void slotClick(int i);
    void slotMenu(QAction*);

protected:
    QMenu *fileMenu;
    QMenu *smenu;
    KMenuBar *menuBar;
    KStatusBar *statusbar;
    bool insert;
    QTextEdit *widget;
};
#endif
