/*
 * $Id$
 *
 * $Log$
 * Revision 1.1  1997/03/15 21:40:24  kalle
 * Initial revision
 *
 * Revision 1.2.2.1  1997/01/07 14:41:57  alex
 * release 0.1
 *
 * Revision 1.2  1997/01/07 13:47:09  alex
 * first working release
 *
 * Revision 1.1.1.1  1997/01/07 13:44:53  alex
 * imported
 *
 */

#ifndef _KTABCTLTEST_H
#define _KTABCTLTEST_H

class TopLevel : public QWidget
{
    Q_OBJECT;
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
