#ifndef _KTABCTLTEST_H
#define _KTABCTLTEST_H

#include <qwidget.h>
class KTabCtl;
class QPushButton;
class Q3MultiLineEdit;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0 );
protected:
    void resizeEvent(QResizeEvent *);
    KTabCtl *test;
    QPushButton *ok, *cancel;
    Q3MultiLineEdit *e;
    QWidget *pages[3];
public slots:
    void okPressed();
    void tabChanged(int);
};

#endif
