#ifndef _KTABCTLTEST_H
#define _KTABCTLTEST_H

#include <qwidget.h>
class KTabCtl;
class QPushButton;
class QTextEdit;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0 );
protected:
    void resizeEvent(QResizeEvent *);
    KTabCtl *test;
    QPushButton *ok, *cancel;
    QTextEdit *e;
    QWidget *pages[3];
public Q_SLOTS:
    void okPressed();
    void tabChanged(int);
};

#endif
