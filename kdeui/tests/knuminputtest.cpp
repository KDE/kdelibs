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

#include <kapp.h>
#include <knuminput.h>

#include "knuminputtest.h"

KApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KNumInput test application");

    QBoxLayout* l = new QHBoxLayout(this, 10);

    QGroupBox* b1 = new QVGroupBox("KIntNumInput", this);

    i1 = new KIntNumInput("percent of usage (no slider)",
                          0, 100, 5, 42, "%", 10, false, b1, "perc_no_slider");
    i2 = new KIntNumInput("percentage of usage (with slider)",
                          0, 100, 5, 42, "%", 10, true, b1, "perc_with_slider");
    i3 = new KIntNumInput("Hex byte (no slider)",
                          0, 255, 1, 0xAF, "(hex)", 16, false, b1, "hex_no_slider");
    i4 = new KIntNumInput("Hex byte (with slider)",
                          0, 255, 1, 0xFE, QString::null, 16, true, b1, "hex_with_slider");

    l->addWidget(b1);

    QGroupBox* b2 = new QVGroupBox("KDoubleNumInput", this);

    d1 = new KDoubleNumInput(0.42, b2, "perc_double_no_slider");
    d1->setLabel("percent of usage (no slider)");
    d1->setRange(0.0, 1.0, 0.005, false);

    d2 = new KDoubleNumInput(d1, 0.422, b2, "perc_double_with_slider");
    d2->setLabel("percentage of usage (with slider)");
    d2->setRange(0, 1.0, 0.005);
    d2->setSuffix("%");

    d3 = new KDoubleNumInput(d2, 16.20, b2);
    d3->setLabel("cash: ", AlignVCenter | AlignLeft);
    d3->setRange(0.10, 100, 0.1);
    d3->setPrefix("p");
    d3->setSuffix("$");

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

