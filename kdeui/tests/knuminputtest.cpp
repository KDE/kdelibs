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

#include <kdebug.h>
void TopLevel::slotPrint( int n ) {
  kdDebug() << "slotPrint( " << n << " )" << endl;
}
void TopLevel::slotPrint( double n ) {
  kdDebug() << "slotPrint( " << n << " )" << endl;
}

#define conn(x,y) connect( x, SIGNAL(valueChanged(y)), SLOT(slotPrint(y)))
TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("KNumInput test application");

    QBoxLayout* l = new QHBoxLayout(this, 10);

    QGroupBox* b1 = new QVGroupBox("KIntNumInput", this);

    i1 = new KIntNumInput(42, b1, 10, "perc_no_slider");
    i1->setLabel("percent of usage (no slider)");
    i1->setRange(0, 100, 5, false);
    conn(i1,int);

    i2 = new KIntNumInput(i1, 42, b1);
    i2->setLabel("percentage of usage (with slider)");
    i2->setRange(0, 100, 5);
    i2->setSuffix(" %");
    conn(i2,int);

    i3 = new KIntNumInput(i2, 0xAF, b1, 16);
    i3->setLabel("Hex byte (no slider)");
    i3->setRange(0, 255, 1, false);
    i3->setSuffix(" (hex)");
    conn(i3,int);

    i4 = new KIntNumInput(i3, 0xfe, b1, 16);
    i4->setLabel("Hex byte (with slider)");
    i4->setRange(0, 255, 1);
    conn(i4,int);

    i5 = new KIntNumInput(i4, 10, b1, 10);
    i5->setLabel("Width (keeps aspect ratio):");
    i5->setRange(0, 200, 1, false);
    i5->setReferencePoint( 5 );

    i6 = new KIntNumInput(i5, 20, b1, 10);
    i6->setLabel("Height (should be 2xWidth):");
    i6->setRange(0, 200, 1, false);
    i6->setReferencePoint( 10 );
    connect( i5, SIGNAL(relativeValueChanged(double)),
	     i6, SLOT(setRelativeValue(double)) );
    connect( i6, SIGNAL(relativeValueChanged(double)),
	     i5, SLOT(setRelativeValue(double)) );

    i7 = new KIntNumInput(i6, 0, b1, 10);
    i7->setLabel("math test:", AlignVCenter|AlignLeft );
    i7->setRange( INT_MIN, INT_MAX, 1 );
    conn(i7,int);

    l->addWidget(b1);

    QGroupBox* b2 = new QVGroupBox("KDoubleNumInput", this);

    d1 = new KDoubleNumInput(4.0, b2, "perc_double_no_slider");
    d1->setLabel("percent of usage (no slider)", AlignTop | AlignRight);
    d1->setRange(0.0, 4000.0, 0.01, false);
    //d1->setValue(1.00000000000000000001);
    conn(d1,double);

    d2 = new KDoubleNumInput(d1, 0.422, b2, "perc_double_with_slider");
    d2->setLabel("percentage of usage (with slider)", AlignBottom | AlignLeft);
    d2->setRange(0, 1.0, 0.005);
    d2->setSuffix("%");
    conn(d2,double);

    d3 = new KDoubleNumInput(d2, 16.20, b2);
    d3->setLabel("cash: ", AlignVCenter | AlignHCenter);
    d3->setRange(0.10, 100, 0.1);
    d3->setPrefix("p");
    d3->setSuffix("$");
    conn(d3,double);

    d4 = new KDoubleNumInput(d3, INT_MAX/10000.0, b2);
    d4->setPrecision(3);
    d4->setRange(double(INT_MIN+1)/1000.0, double(INT_MAX)/1000.0, 1);
    d4->setLabel("math test: ", AlignVCenter | AlignLeft);
//    d4->setFormat("%g");
    conn(d4,double);

    d5 = new KDoubleNumInput(d4, double(INT_MIN+1)/1e9, double(INT_MAX-1)/1e9,
			     0.1, 0.001, 9, b2, "d5");
    d5->setLabel("math test 2: ", AlignVCenter|AlignLeft);
    conn(d5,double);

    d6 = new KDoubleNumInput(d5, -10, 10, 0, 0.001, 3, b2, "d6");
    d6->setLabel("aspect ratio test with a negative ratio:");
    d6->setReferencePoint( 1 );

    d7 = new KDoubleNumInput(d6, -30, 30, 0, 0.001, 3, b2, "d7");
    d7->setReferencePoint( -3 );

    connect( d6, SIGNAL(relativeValueChanged(double)),
	     d7, SLOT(setRelativeValue(double)) );
    connect( d7, SIGNAL(relativeValueChanged(double)),
	     d6, SLOT(setRelativeValue(double)) );

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

