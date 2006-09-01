#include <QPainter>

#include "WeaverThreadGrid.h"
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
          m_colorIdle ( Qt::darkGreen )
    {
    }

    WeaverThreadGrid::~WeaverThreadGrid ()
    {
    }

    void WeaverThreadGrid::attach ( Weaver *weaver )
    {
        weaver->registerObserver ( &m_observer );
        connect ( &m_observer,  SIGNAL ( threadStarted( Thread* ) ),
                  SLOT ( threadStarted ( Thread* ) ) );
        connect ( &m_observer,  SIGNAL ( threadBusy( Thread*, Job* ) ),
                  SLOT ( threadBusy ( Thread*, Job* ) ) );
        connect ( &m_observer,  SIGNAL ( threadSuspended( Thread* ) ),
                  SLOT ( threadSuspended ( Thread* ) ) );
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
