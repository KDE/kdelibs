/****************************************************************************
    Definition of QXEmbed class

   Copyright (C) 1999-2000 Troll Tech AS

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*****************************************************************************/

#ifndef QXEMBED_H
#define QXEMBED_H

#include <qwidget.h>

#ifndef Q_WS_QWS

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

    static void initialize();

    void embed( WId w );
    WId embeddedWinId() const;

    static void embedClientIntoWindow( QWidget* client, WId window );
    static bool processClientCmdline( QWidget* client, int& argc, char ** argv );


    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSizePolicy sizePolicy() const;

    bool eventFilter( QObject *, QEvent * );

    void setAutoDelete( bool );
    bool autoDelete() const;

    bool customWhatsThis() const;
    void enterWhatsThisMode(); // temporary, fix in Qt (Matthias, Mon Jul 17 15:20:55 CEST 2000  )

signals:
    void embeddedWindowDestroyed();

protected:
    bool event( QEvent * );
    void keyPressEvent( QKeyEvent * );
    void keyReleaseEvent( QKeyEvent * );
    void focusInEvent( QFocusEvent * );
    void focusOutEvent( QFocusEvent * );
    void resizeEvent(QResizeEvent *);
    void showEvent( QShowEvent * );
    bool x11Event( XEvent* );

    virtual void windowChanged( WId );

    bool focusNextPrevChild( bool next );

private:
    WId window;
    QXEmbedData* d;
    void sendSyntheticConfigureNotifyEvent();
};


#endif
#endif
