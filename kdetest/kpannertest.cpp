/*
 * $Id$
 */


#include <kapp.h>
#include <qpushbt.h>
#include <qobject.h>
#include <qlistbox.h>
#include <qmlined.h>

#include "kpanner.h"
#include "kpannertest.h"

QFont default_font("Helvetica", 12);

QApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KPanner class test");
    setMinimumSize(300, 200);
    panner = new KPanner(this, "_panner", KPanner::O_HORIZONTAL, 50);
    panner1 = new KPanner(panner->child1(), "_panner1", KPanner::O_VERTICAL, 50);
    panner->resize(300, 200);
    panner->move(0, 0);
    box = new QListBox(panner1->child1(), "_list", 0);
    box->insertItem("Hi, I'am a listbox", -1);
    e = new QMultiLineEdit(panner->child0(), "_edit");
    ok = new QPushButton("Clickme", panner1->child0(), "_ok");
    cancel = new QPushButton("Exitme", panner1->child0(), "_cancel");
    ok->resize(80, 25);
    cancel->resize(80, 25);
    connect(ok, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancel, SIGNAL(clicked()), a, SLOT(quit()));
    connect(panner, SIGNAL(positionChanged()), this, SLOT(pannerHasChanged()));
    connect(panner1, SIGNAL(positionChanged()), this, SLOT(pannerHasChanged()));
    resize(400, 300);
    panner->setLimits(50, 250);
    panner1->setLimits(50, -50);
}

void TopLevel::pannerHasChanged()
{
    resizeEvent(0);
}

void TopLevel::okPressed()
{
    panner->setAbsSeparator(70);
}

void TopLevel::resizeEvent( QResizeEvent * )
{
    panner->setGeometry(5, 5, width() - 10, height() - 10);
    panner1->resize(panner->child1()->width(), panner->child1()->height());
    e->setGeometry(0, 0, panner->child0()->width(), panner->child0()->height());
    box->setGeometry(0, 0, panner1->child1()->width(), panner1->child1()->height());
    ok->setGeometry(0, 0, panner1->child0()->width(), 25);
    cancel->setGeometry(0, panner1->child0()->height() - 25, panner1->child0()->width(), 25);
}

#include "kpannertest.h"

int main( int argc, char ** argv )
{
    a = new KApplication ( argc, argv );

    a->setFont(default_font);
    TopLevel *toplevel = new TopLevel(0, "test");

    toplevel->show();
    a->exec();
}
