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
    
    d1 = new KDoubleNumInput("percent of usage (no slider)",
                          0, 100, 5, 42, "%", "%.1g", false, b2, "perc_no_slider");
    d2 = new KDoubleNumInput("percentage of usage (with slider)",
                          0, 100, 5, 42, "%", (const char*)0, true, b2, "perc_with_slider");
#if 0    
    d3 = new KDoubleNumInput("Hex byte (no slider)",
                          0, 255, 1, 0xAF, "(hex)", 16, 0, false, b2, "hex_no_slider");
    d4 = new KDoubleNumInput("Hex byte (with slider)",
                          0, 255, 1, 0xFE, QString::null, 16, 0, true, b2, "hex_with_slider");
#endif    

    l->addWidget(b2);
}


    

int main( int argc, char ** argv )
{
    a = new KApplication ( argc, argv, "KNuminputTest" );

    TopLevel *toplevel = new TopLevel(0, "_ktabctl_test");

    toplevel->show();
    a->setMainWidget(toplevel);
    a->exec();
}

#include "knuminputtest.moc"

