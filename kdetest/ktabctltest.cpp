/*
 * $Id$
 *
 * $Log$
 * Revision 1.1  1997/03/15 21:40:24  kalle
 * Initial revision
 *
 * Revision 1.3.2.1  1997/01/07 14:41:57  alex
 * release 0.1
 *
 * Revision 1.3  1997/01/07 14:39:15  alex
 * some doc added, tested - ok.
 *
 * Revision 1.2  1997/01/07 13:47:09  alex
 * first working release
 *
 *
 */

#include <qapp.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qobject.h>
#include <qlistbox.h>
#include <qgrpbox.h>
#include <qevent.h>
#include <qcombo.h>
#include <qlined.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qtabdlg.h>
#include <qtooltip.h>
#include <qmsgbox.h>
#include <qtabbar.h>
#include <qpalette.h>
#include <qmlined.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ktabctl.h"
#include "ktabctltest.moc"
#include "ktabctltest.h"

QFont default_font("Helvetica", 12);

QApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KTabCtl test application");
    setMinimumSize(300, 200);

    /*
     * add a tabctrl widget
     */
    
    test = new KTabCtl(this, "test");
    connect(test, SIGNAL(tabSelected(int)), this, SLOT(tabChanged(int)));
    QWidget *w = new QWidget(test, "_page1");
    QPushButton *bt = new QPushButton("Click me to quit", w, "_bt1");
    connect(bt, SIGNAL(clicked()), this, SLOT(okPressed()));
    bt->adjustSize();
    bt->move(20, 20);
    test->addTab(w, "Seite 1");
    pages[0] = w;
    w = new QWidget(test, "_page2");
    e = new QMultiLineEdit(w, "_editor");
    e->setText("Write some usesless stuff here :-)");
    w->resize(width(), height());
    test->addTab(w, "Seite 2");
    pages[1] = w;
    w = new QWidget(test, "_page3");
    bt = new QPushButton("This button does absolutely nothing", w, "_bt3");
    bt->adjustSize();
    bt->move(20, 20);
    test->addTab(w, "Seite 3");
    pages[2] = w;
    test->resize(200, 200);
    test->move(0, 0);
    move(20, 20);
    resize(400, 300);
    adjustSize();
}

void TopLevel::resizeEvent( QResizeEvent * )
{
    test->resize(width(), height());
    e->setGeometry(10, 10, pages[1]->width() - 20, pages[1]->height() - 20);
}

void TopLevel::tabChanged(int newpage)
{
    printf("tab number %d selected\n", newpage);
    if(newpage == 1)
        e->setFocus();
}

void TopLevel::okPressed()
{
    a->quit();
}

int main( int argc, char ** argv )
{
    a = new QApplication ( argc, argv );

    a->setFont(default_font);

    TopLevel *toplevel = new TopLevel(0, "_ktabctl_test");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}
