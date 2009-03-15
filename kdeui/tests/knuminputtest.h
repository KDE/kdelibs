/*
* Tests the KNumInput Widget class
*
* Copyright 1999 by Dirk A. Mueller <dmuell@gmx.net>
*
* Licensed under the GNU General Public License version 2 or later
*/
#ifndef _KNUMINPUTTEST_H
#define _KNUMINPUTTEST_H

#include <QtGui/QWidget>

class KIntNumInput;
class KDoubleNumInput;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0 );
protected:
    KIntNumInput* i1, *i2, *i3, *i4, *i5, *i6, *i7, *i8;
    KDoubleNumInput* d1, *d2, *d3, *d4, *d5, *d6, *d7;
protected Q_SLOTS:
    void slotPrint( int );
    void slotPrint( double ); 
};

#endif
