/****************************************************************************
** $Id$
**
** Definition of QXEmbed class
**
** Created :
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QXEMBED_H
#define QXEMBED_H

#include <qwidget.h>
#include <qvaluelist.h>

/*
  Documentation for this class is in the cpp-file!
 */


class QXEmbedData;
class QXEmbed : public QWidget
{
    Q_OBJECT

public:

    QXEmbed( QWidget *parent=0, const char *name=0, WFlags f = 0 );
    ~QXEmbed();

    void embed( WId w );
    WId embeddedWinId() const;

    static void embedClientIntoWindow( QWidget* client, WId window );
    static bool processClientCmdline( QWidget* client, int& argc, char ** argc );


    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSizePolicy sizePolicy() const;


protected:
    void keyPressEvent( QKeyEvent * );
    void keyReleaseEvent( QKeyEvent * );
    void focusInEvent( QFocusEvent * );
    void focusOutEvent( QFocusEvent * );
    void resizeEvent(QResizeEvent *);
    void showEvent( QShowEvent * );
    void wheelEvent( QWheelEvent * );
    bool x11Event( XEvent* );

    virtual void windowChanged( WId );

    bool focusNextPrevChild( bool next );

private:

    void sendFocusIn();
    void sendFocusOut();
    WId window;
    bool window_supports_tab_focus;
    QXEmbedData* d;
};


#endif
