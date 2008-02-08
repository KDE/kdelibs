/* -*- C++ -*-

   This file declares the WeaverThreadGrid class.

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

#ifndef WEAVERTHREADGRID_H
#define WEAVERTHREADGRID_H

#include <QtCore/QMap>
#include <QtCore/QSize>
#include <QtGui/QFrame>

#include <WeaverObserver.h>
#include <ThreadWeaver.h>
#include <threadweaver_export.h>

namespace ThreadWeaver {

    class THREADWEAVER_EXPORT WeaverThreadGrid : public QFrame
    {
        Q_OBJECT
    public:
        WeaverThreadGrid ( QWidget *parent = 0 );
        ~WeaverThreadGrid ();
	// attach this object to a Weaver to visualize its activity:
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
    protected Q_SLOTS:
        void threadStarted ( ThreadWeaver::Thread* );
        void threadBusy ( ThreadWeaver::Thread*, ThreadWeaver::Job* );
        void threadSuspended ( ThreadWeaver::Thread* );
    private:
        class Private;
        Private * const d;
    };
}

#endif
