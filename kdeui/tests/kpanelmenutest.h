#ifndef __KPANELMENUTEST_H
#define __KPANELMENUTEST_H

#include <kpanelmenu.h>
#include <qlabel.h>

class TestWidget : public QLabel
{
    Q_OBJECT
public:
    TestWidget(QWidget *parent=0, const char *name=0);
    ~TestWidget(){delete testMenu;}
public slots:
    void slotMenuCalled(int id);
    void slotSubMenuCalled(int id);
private:
    void init();
    KPanelMenu *testMenu, *subMenu;
};

#endif
