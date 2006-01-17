/****************************************************************************
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELLO_H
#define HELLO_H

#include <qstring.h>
#include <k3mdichildview.h>

class QWidget;
class QMouseEvent;
class QPaintEvent;

class Hello : public K3MdiChildView
{
    Q_OBJECT
public:
    Hello( const char *title, const char *text, QWidget* parentWidget = 0 );
Q_SIGNALS:
    void clicked();
protected:
    void mouseReleaseEvent( QMouseEvent * );
    void paintEvent( QPaintEvent * );
private Q_SLOTS:
    void animate();
private:
    QString t;
    int     b;
};

#endif
