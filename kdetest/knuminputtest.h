#ifndef _KNUMINPUTTEST_H
#define _KNUMINPUTTEST_H

#include <qwidget.h>

class KIntNumInput;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );
protected:
    KIntNumInput* i1, *i2, *i3, *i4;
};

#endif
