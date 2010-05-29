/*
 * $Id: ktabctltest.h 237592 2003-07-17 13:00:10Z binner $
 */

#ifndef _KTABCTLTEST_H
#define _KTABCTLTEST_H

#include <qwidget.h>
class KTabCtl;
class QPushButton;
class QMultiLineEdit;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent(QResizeEvent *);
    KTabCtl *test;
    QPushButton *ok, *cancel;
    QMultiLineEdit *e;
    QWidget *pages[3];
public slots:
    void okPressed();
    void tabChanged(int);
};

#endif
