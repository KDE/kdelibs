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

#include <kapp.h>
#include <knuminput.h>

#include "knuminputtest.h"

KApplication *a;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KNumInput test application");

    QBoxLayout* l = new QVBoxLayout(this, 20);
    
    i1 = new KIntNumInput("percent of usage (no slider)",
                          0, 100, 5, 42, "%", 10, false, this, "perc_no_slider");
    l->addWidget(i1);

    i2 = new KIntNumInput("percentage of usage (with slider)",
                          0, 100, 5, 42, "%", 10, true, this, "perc_with_slider");
    l->addWidget(i2);

    i3 = new KIntNumInput("Hex byte (no slider)",
                          0, 255, 1, 0xAF, "(hex)", 16, false, this, "hex_no_slider");
    l->addWidget(i3);

    i4 = new KIntNumInput("Hex byte (with slider)",
                          0, 255, 1, 0xFE, QString::null, 16, true, this, "hex_with_slider");
    l->addWidget(i4);
}

int main( int argc, char ** argv )
{
    a = new KApplication ( argc, argv );

    TopLevel *toplevel = new TopLevel(0, "_ktabctl_test");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}

#include "knuminputtest.moc"

