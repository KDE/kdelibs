/*
 * $Id$
 *
 * $Log$
 * Revision 1.3  1999/01/15 08:59:28  kulow
 * use seperate moc.cpp files - did you know that most header files herein are
 * complete hacks? ;)
 *
 * Revision 1.2  1997/09/10 13:53:15  kdecvs
 * Kalle: Use KApplication instead of QApplication
 *
 * Revision 1.1  1997/04/13 17:49:42  kulow
 * Sources imported
 *
 * Revision 1.1  1997/03/15 21:40:24  kalle
 * Initial revision
 *
 * Revision 1.3.2.1  1997/01/10 19:48:32  alex
 * public release 0.1
 *
 * Revision 1.3  1997/01/10 19:44:33  alex
 * *** empty log message ***
 *
 * Revision 1.2.4.1  1997/01/10 16:46:33  alex
 * rel 0.1a, not public
 *
 * Revision 1.2  1997/01/10 13:05:52  alex
 * *** empty log message ***
 *
 * Revision 1.1.1.1  1997/01/10 13:05:21  alex
 * imported
 *
 */

#include <kapp.h>
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

#include "kmsgbox.h"
#include "kmsgboxtest.h"
#include "kmsgboxtest.h"

QFont default_font("Helvetica", 12);

QApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KMsgBox Test");
    setMinimumSize(300, 200);

    /*
     * add a tabctrl widget
     */
    
    ok = new QPushButton("Click me to quit", this, "_bt1");
    connect(ok, SIGNAL(clicked()), this, SLOT(okPressed()));
    ok->adjustSize();
    e = new QListBox(this, "_listbox");
    e->insertItem("Box with stop sign", -1);
    e->insertItem("Box with question mark", -1);
    e->insertItem("Box with one button and information sign", -1);
    e->insertItem("Box with two buttons and exclamation", -1);
    connect(e, SIGNAL(selected(int)), this, SLOT(showMsgBox(int)));
    adjustSize();
}

void TopLevel::resizeEvent( QResizeEvent * )
{
    ok->move(width() - 110, height() - 30);
    e->setGeometry(10, 10, width() - 20, height() - 70);
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

void TopLevel::doMsgBox()
{
}

void TopLevel::showMsgBox(int item)
{
    switch(item) {
    case 0:
        printf("Result: %d\n",KMsgBox::yesNoCancel(0, "Save", "This file has been modified.\nDo you want to save it ?", KMsgBox::STOP));
        break;

    case 1:
        KMsgBox::yesNo(0, "Hello", "Did you crash your Windows today ?", KMsgBox::QUESTION | KMsgBox::DB_SECOND);
        break;

    case 2:
        KMsgBox::message(0, "This sucks", "I know, where I want to go today", KMsgBox::INFORMATION, "Oops");
        break;
        
    case 3:
        KMsgBox::yesNo(0, "Hi", "Check your power supply, it's broken", KMsgBox::EXCLAMATION, "Arrgh");
        break;
        
    default:
        break;
    }
}

        
int main( int argc, char ** argv )
{
    a = new KApplication ( argc, argv );

    a->setFont(default_font);

    TopLevel *toplevel = new TopLevel(0, "_ktabctl_test");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}
#include "kmsgboxtest.moc"
