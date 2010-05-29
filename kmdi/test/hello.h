/****************************************************************************
** $Id: hello.h 369568 2004-12-10 04:36:02Z mattr $
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
#include <kmdichildview.h>

class QWidget;
class QMouseEvent;
class QPaintEvent;

class Hello : public KMdiChildView
{
    Q_OBJECT
public:
    Hello( const char *title, const char *text, QWidget* parentWidget = 0 );
signals:
    void clicked();
protected:
    void mouseReleaseEvent( QMouseEvent * );
    void paintEvent( QPaintEvent * );
private slots:
    void animate();
private:
    QString t;
    int     b;
};

#endif
