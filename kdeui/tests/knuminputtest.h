#ifndef _KNUMINPUTTEST_H
#define _KNUMINPUTTEST_H

#include <qwidget.h>

class KIntNumInput;
class KDoubleNumInput;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );
protected:
    KIntNumInput* i1, *i2, *i3, *i4, *i5, *i6, *i7;
    KDoubleNumInput* d1, *d2, *d3, *d4, *d5, *d6, *d7;
protected slots:
    void slotPrint( int );
    void slotPrint( double ); 
};

#endif
