/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
//#define DEBUG_LAYOUT

#include "render_hr.h"

#include <qpixmap.h>
#include <qdrawutil.h>
#include <qpalette.h>

#include <kdebug.h>

#include "rendering/render_style.h"
#include "rendering/render_flow.h"

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------

RenderHR::RenderHR()
    : RenderReplaced()
{
    // init RenderObject attributes
    m_inline = false;   // our object is not Inline

    m_printSpecial=false;
}

RenderHR::~RenderHR()
{
}


void RenderHR::printReplaced(QPainter *p, int _tx, int _ty)
{
    // add offset for relative positioning
    if(isRelPositioned())
        relativePositionOffset(_tx, _ty);


    QColorGroup colorGrp( Qt::black, Qt::black, QColor(220,220,220), QColor(100,100,100),
    Qt::gray, Qt::black, Qt::black );

    int xp = _tx;

    RenderObject *prev = m_previous;
    while(prev && !prev->isFlow())
        prev = prev->previousSibling();
    if(prev && static_cast<RenderFlow *>(prev)->floatBottom() > prev->height() )
        xp += static_cast<RenderFlow *>(prev)->leftOffset( prev->height() );


    int yp = _ty ;

    //kdDebug() << "tx = " << xp << " m_width = " << m_width << " length = " << length << endl;

    switch(m_style->textAlign()) {
    case LEFT:
        break;
    case RIGHT:
        xp += m_width - length;
        break;
    case JUSTIFY:
    case CENTER:
    case KONQ_CENTER:
        //kdDebug() << "centered" << endl;
        xp += (m_width - length)/2;
        break;
    }

    int lw = size/2;

    if ( shade )
    {
        if(size < 2) size = 2, lw = 1;
        qDrawShadePanel( p, xp, yp, length, size,
                colorGrp, true, lw, 0 );
    }
    else
    {
        if(size < 1) size = 1;
        p->fillRect( xp, yp, length, size, Qt::black );
    }
}

void RenderHR::layout()
{
    calcMinMaxWidth();
    m_height = size+2;
    if( length == 0 )
        length = m_width;
    calcHeight();
    setLayouted(true);
}

void RenderHR::calcMinMaxWidth()
{
    // contentWidth
    Length w = m_style->width();

    // a bit hacky....
    calcWidth();
    length = m_width;
    //m_width = intrinsicWidth();

    switch(w.type)
    {
    case Fixed:
        m_minWidth = m_width;
        m_maxWidth = m_width;
        break;
    case Percent:
        m_minWidth = 1;
        m_maxWidth = m_width;
        break;
    default:
        m_minWidth = 0;
        m_maxWidth = 0;
    }
}

short RenderHR::intrinsicWidth() const
{
    RenderObject *prev = m_previous;
    while(prev && !prev->isFlow())
        prev = prev->previousSibling();
    int w;
    if(prev && static_cast<RenderFlow *>(prev)->floatBottom() > prev->height() )
        w = static_cast<RenderFlow *>(prev)->lineWidth( prev->height() );
    else
        w =containingBlockWidth();
    //kdDebug(0) << "renderHR::intrinsicWidth = " << w << endl;
    return w;
}

int RenderHR::intrinsicHeight() const
{
    return size+2;
}
