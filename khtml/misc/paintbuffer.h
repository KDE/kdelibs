/*
 * This file is part of the KDE libraries
 *
 * Copyright (C) 2007 Germain Garand <germain@ebooksfrance.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

// The PaintBuffer class provides a shared buffer for efficient repetitive painting.
//
// It will grow to encompass the biggest size encountered, in order to avoid
// constantly resizing.
// When it grows over maxPixelBuffering, it periodically checks if such a size
// is still needed.  If not, it shrinks down to the biggest size < maxPixelBuffering
// that was requested during the overflow lapse.

#ifndef html_paintbuffer_h
#define html_paintbuffer_h

#include <QPixmap>

namespace khtml {

class PaintBuffer: public QObject
{
public:
    static const int maxPixelBuffering;
    static const int leaseTime;
    
    static QPixmap* grab( QSize s = QSize() );
    static void release();
    
    static void cleanup();
    
    PaintBuffer();
    virtual void timerEvent(QTimerEvent* e);
    
private:
    QPixmap* getBuf( QSize S );

    static PaintBuffer* m_inst;
    QPixmap m_buf;
    bool m_overflow;
    bool m_grabbed;
    int m_timer;
    int m_resetWidth;
    int m_resetHeight;
};

}
#endif
