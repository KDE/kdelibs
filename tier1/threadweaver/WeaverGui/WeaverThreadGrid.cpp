/* -*- C++ -*-

   This file implements the WeaverThreadGrid class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   $Id: ThreadWeaver.h 32 2005-08-17 08:38:01Z mirko $
*/
#include "WeaverThreadGrid.h"

#include <QtGui/QPainter>

#include <Thread.h>

namespace ThreadWeaver {

    // pixel values:
    const int Padding = 3;
    const int Spacing = 3;
    const int CellWidth = 12; // including the frame (darker color)
    const int CellHeight = 7;

    WeaverThreadGrid::WeaverThreadGrid ( QWidget *parent )
        : QFrame ( parent ),
          m_colorBusyFrame ( Qt::darkGreen ),
          m_colorBusy ( Qt::green ),
          m_colorIdleFrame ( Qt::darkGray ),
          m_colorIdle ( Qt::darkGreen ),
          d ( 0 )
    {
    }

    WeaverThreadGrid::~WeaverThreadGrid ()
    {
    }

    void WeaverThreadGrid::attach ( Weaver *weaver )
    {
        weaver->registerObserver ( &m_observer );
        connect ( &m_observer,  SIGNAL ( threadStarted( ThreadWeaver::Thread* ) ),
                  SLOT ( threadStarted ( ThreadWeaver::Thread* ) ) );
        connect ( &m_observer,  SIGNAL ( threadBusy( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ),
                  SLOT ( threadBusy ( ThreadWeaver::Thread*, ThreadWeaver::Job* ) ) );
        connect ( &m_observer,  SIGNAL ( threadSuspended( ThreadWeaver::Thread* ) ),
                  SLOT ( threadSuspended ( ThreadWeaver::Thread* ) ) );
    }

    void WeaverThreadGrid::paintEvent ( QPaintEvent *e )
    {
        int padding  = Padding + frameWidth();
        int count = 0;
        QFrame::paintEvent ( e );

        QPainter painter(this);
        QMapIterator<Thread*, bool> it (m_cells);
        while (it.hasNext())
        {
            it.next();
            int x = padding + count*Spacing + count*CellWidth;
            int y = qMax ( padding,
                           ( height() - 2*frameWidth() - padding ) / 2 );
            if ( x < width() && y < height() )
            {
                if ( it.value() )
                {
                    painter.setPen ( m_colorBusyFrame );
                    painter.setBrush ( m_colorBusy );
                } else {
                    painter.setPen ( m_colorIdleFrame );
                    painter.setBrush ( m_colorIdle );
                }
                painter.drawRect ( x,  y, CellWidth,  CellHeight );
            }
            ++count;
        }
    }

    QSize WeaverThreadGrid::minimumSizeHint () const
    {
        int padding = Padding + frameWidth();
        int num = m_cells.size();
        return QSize ( 2*padding + num*Spacing + num*CellWidth,
                       2*padding + CellHeight );
    }

    QSize WeaverThreadGrid::sizeHint () const
    {
        return minimumSizeHint();
    }

    QSize WeaverThreadGrid::minimumSize () const
    {
        return minimumSizeHint();
    }

    void WeaverThreadGrid::threadStarted ( Thread *t )
    {
        m_cells[t] = true;
        setMinimumSize ( sizeHint() );
    }

    void WeaverThreadGrid::threadBusy ( Thread *t, Job* )
    {
        m_cells[t] = true;
        update ();
    }


    void WeaverThreadGrid::threadSuspended ( Thread *t )
    {
        m_cells[t] = false;
        update ();
    }


}

#include "WeaverThreadGrid.moc"
