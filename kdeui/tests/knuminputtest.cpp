/*
* Tests the KNumInput Widget class
*
* Copyright (c) 1999 by Dirk A. Mueller <dmuell@gmx.net>
*
* License: GPL, version 2
* Version: $Id:
*
*/

#include <qlayout.h>
#include <qvgroupbox.h>

#include <kapplication.h>
#include <knuminput.h>

#include "knuminputtest.h"

KApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KNumInput test application");

    QBoxLayout* l = new QHBoxLayout(this, 10);

    QGroupBox* b1 = new QVGroupBox("KIntNumInput", this);

    i1 = new KIntNumInput(42, b1, 10, "perc_no_slider");
    i1->setLabel("percent of usage (no slider)");
    i1->setRange(0, 100, 5, false);

    i2 = new KIntNumInput(i1, 42, b1);
    i2->setLabel("percentage of usage (with slider)");
    i2->setRange(0, 100, 5);
    i2->setSuffix(" %");

    i3 = new KIntNumInput(i2, 0xAF, b1, 16);
    i3->setLabel("Hex byte (no slider)");
    i3->setRange(0, 255, 1, false);
    i3->setSuffix(" (hex)");

    i4 = new KIntNumInput(i3, 0xfe, b1, 16);
    i4->setLabel("Hex byte (with slider)");
    i4->setRange(0, 255, 1);

    l->addWidget(b1);

    QGroupBox* b2 = new QVGroupBox("KDoubleNumInput", this);

    d1 = new KDoubleNumInput(4.0, b2, "perc_double_no_slider");
    d1->setLabel("percent of usage (no slider)", AlignTop | AlignRight);
    d1->setRange(0.0, 4000.0, 0.01, false);
    //d1->setValue(1.00000000000000000001);

    d2 = new KDoubleNumInput(d1, 0.422, b2, "perc_double_with_slider");
    d2->setLabel("percentage of usage (with slider)", AlignBottom | AlignLeft);
    d2->setRange(0, 1.0, 0.005);
    d2->setSuffix("%");

    d3 = new KDoubleNumInput(d2, 16.20, b2);
    d3->setLabel("cash: ", AlignVCenter | AlignHCenter);
    d3->setRange(0.10, 100, 0.1);
    d3->setPrefix("p");
    d3->setSuffix("$");

    d4 = new KDoubleNumInput(d3, 1e6, b2);
    d4->setLabel("math test: ", AlignVCenter | AlignLeft);
    d4->setRange(-1e10, 1e10, 1);
    d4->setFormat("%g");

    l->addWidget(b2);
}




int main( int argc, char ** argv )
{
    a = new KApplication ( argc, argv, "KNuminputTest" );

    TopLevel *toplevel = new TopLevel(0, "knuminputtest");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}

#include "knuminputtest.moc"

