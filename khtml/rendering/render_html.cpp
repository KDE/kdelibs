/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
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
#include "render_html.h"

#include <qpainter.h>

#include <kdebug.h>

using namespace khtml;

RenderHtml::RenderHtml()
    : RenderFlow()
{
}

RenderHtml::~RenderHtml()
{
}

void RenderHtml::setStyle(RenderStyle *style)
{
    RenderFlow::setStyle(style);
    m_printSpecial = true;
}

void RenderHtml::print(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    //kdDebug(0) << "html:print " << _tx << "/" << _ty << endl;
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderHtml::printBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::printDecorations()" << _tx << "/" << _ty << endl;

    QColor c = m_style->backgroundColor();
    CachedImage *bg = m_bgImage;

    if( firstChild() ) {
	if( !c.isValid() )
	    c = firstChild()->style()->backgroundColor();
	if( !bg )
	    bg = firstChild()->backgroundImage();
    }

    int w = width();
    int h = height();

    //kdDebug(0) << "width = " << w <<endl;

    int bx = _tx - marginLeft();
    int by = _ty - marginTop();
    int bw = QMAX(w + marginLeft() + marginRight() + borderLeft() + borderRight(), parent()->width());
    int bh = QMAX(h + marginTop() + marginBottom() + borderTop() + borderBottom(), parent()->height());

    if(c.isValid()) {
	p->fillRect(bx, by, bw, bh, c);
    }

    if(bg) {
	// kdDebug( 6040 ) << "printing bgimage at " << bx << "/" << by << " " << bw << "/" << bh << endl;
	// ### might need to add some correct offsets
	// ### use paddingX/Y
	
	int sx = 0;
	int sy = 0;
	
	if( !m_style->backgroundAttachment() ) {
	    //kdDebug(0) << "fixed background" << endl;
	    QRect r = viewRect();
	    sx = bx - r.x();
	    sy = by - r.y();
	
	} 	

	switch(m_style->backgroundRepeat()) {
	case NO_REPEAT:
            bw = QMIN(bg->pixmap_size().width(), bw);
            /* nobreak */
	case REPEAT_X:
            bh = QMIN(bg->pixmap_size().height(), bh);
	    break;
	case REPEAT_Y:
            bw = QMIN(bg->pixmap_size().width(), bw);
	    break;
	case REPEAT:
	    break;
	}
	p->drawTiledPixmap(bx, by, bw, bh,bg->pixmap(), sx, sy);
    }

    if(m_style->hasBorder())
    {
	if(m_style->borderTopStyle() != BNONE)
	{
	    c = m_style->borderTopColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx + w, _ty, m_style->borderTopWidth(),
		       BSTop, c, m_style->borderTopStyle());
	}
	if(m_style->borderBottomStyle() != BNONE)
	{
	    c = m_style->borderBottomColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty + h, _tx + w, _ty + h, m_style->borderBottomWidth(),
		       BSBottom, c, m_style->borderBottomStyle());
	}
	if(m_style->borderLeftStyle() != BNONE)
	{
	    c = m_style->borderLeftColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx, _ty + h, m_style->borderLeftWidth(),
		       BSLeft, c, m_style->borderLeftStyle());
	}
	if(m_style->borderRightStyle() != BNONE)
	{
	    c = m_style->borderRightColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx + w, _ty, _tx + w, _ty + h, m_style->borderRightWidth(),
		       BSRight, c, m_style->borderRightStyle());
	}
    }
}

void RenderHtml::repaint()
{
    repaintContainingBlock();
#if 0    
    int bx = - marginLeft();
    int by = - marginTop();
    int bw = m_width + marginLeft() + marginRight() + borderLeft() + borderRight();
    int bh = m_height + marginTop() + marginBottom() + borderTop() + borderBottom();
    repaintRectangle(bx, by, bw, bh);
#endif
}

void RenderHtml::layout()
{
    RenderFlow::layout();

    //kdDebug(0) << renderName() << " height = " << m_height << endl;
    int lp = lowestPosition();
    if( m_height < lp )
	m_height = lp;

    //kdDebug(0) << "docHeight = " << m_height << endl;
}
