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

#include "paintbuffer.h"
#include <QPixmap>
#include <QTimerEvent>
#include <assert.h>

using namespace khtml;

const int PaintBuffer::maxPixelBuffering = 320*200;
const int PaintBuffer::leaseTime = 20*1000;

PaintBuffer::PaintBuffer()
:   m_overflow(false), 
    m_grabbed(false),
    m_timer(0), 
    m_resetWidth(0), 
    m_resetHeight(0)
{

}

void PaintBuffer::cleanup()
{
    delete m_inst;
}

QPixmap *PaintBuffer::grab( QSize s ) 
{
    if (!m_inst)
        m_inst = new PaintBuffer;
    return m_inst->getBuf( s );
}

void PaintBuffer::release()
{ 
    m_inst->m_grabbed = false;
}

void PaintBuffer::timerEvent(QTimerEvent* e)
{
    assert( m_timer == e->timerId() );
    if (m_grabbed)
        return;
    m_buf = QPixmap(m_resetWidth, m_resetHeight);
    m_resetWidth = m_resetHeight = 0;
    killTimer( m_timer );
    m_timer = 0;
}

QPixmap *PaintBuffer::getBuf( QSize s )
{
    assert( !m_grabbed );
    if (s.isEmpty())
        return 0;

    m_grabbed = true;
    bool cur_overflow = false;
    int nw = qMax(m_buf.width(), s.width());
    int nh = qMax(m_buf.height(), s.height());

    if (!m_overflow && (nw*nh > maxPixelBuffering))
        cur_overflow = true;

    if (nw != m_buf.width() || nh != m_buf.height())
        m_buf = QPixmap(nw, nh);

    if (cur_overflow) {
        m_overflow = true;
        m_timer = startTimer( leaseTime );
    } else if (m_overflow) {
        if( s.width()*s.height() > maxPixelBuffering ) {
            killTimer( m_timer );
            m_timer = startTimer( leaseTime );
        } else {
            if (s.width() > m_resetWidth)
                m_resetWidth = s.width();
            if (s.height() > m_resetHeight)
                m_resetHeight = s.height();
        }
    }
    return &m_buf;
}

PaintBuffer *PaintBuffer::m_inst = 0;
