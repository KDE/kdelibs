#ifndef __PANELMENUTEST_H
#define __PANELMENUTEST_H

#include <panelmenu.h>
#include <qlabel.h>

class TestWidget : public QLabel
{
    Q_OBJECT
public:
    TestWidget(QWidget *parent=0, const char *name=0);
    ~TestWidget(){delete testMenu;}
public slots:
    void slotMenuCalled(int id);
private:
    void init();
    PanelMenu *testMenu;
};

#endif
