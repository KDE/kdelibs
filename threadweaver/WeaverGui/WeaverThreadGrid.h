#ifndef WEAVERTHREADGRID_H
#define WEAVERTHREADGRID_H

#include <QFrame>
#include <QMap>
#include <QSize>

#include <WeaverObserver.h>
#include <ThreadWeaver.h>

namespace ThreadWeaver {

    class WeaverThreadGrid : public QFrame
    {
        Q_OBJECT
    public:
        WeaverThreadGrid ( QWidget *parent = 0 );
        ~WeaverThreadGrid ();
	// attach this object to a Weaver to visualize it's activity:
	void attach (Weaver *weaver);
    protected:
        // rewrite the paintEvent handler:
        void paintEvent ( QPaintEvent * event );
        QSize minimumSize () const;
        QSize minimumSizeHint () const;
        QSize sizeHint () const;
	// the WeaverObserver:
	WeaverObserver m_observer;
        // the cells (one for each thread):
        QMap<Thread*, bool> m_cells;
        QColor m_colorBusyFrame;
        QColor m_colorBusy;
        QColor m_colorIdleFrame;
        QColor m_colorIdle;
    protected slots:
        void threadStarted ( Thread* );
        void threadBusy ( Thread*, Job* );
        void threadSuspended ( Thread* );
    };
}

#endif
