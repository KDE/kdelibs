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
#include "render_body.h"

#include <qpainter.h>

#include <kdebug.h>

using namespace khtml;

RenderBody::RenderBody()
    : RenderFlow()
{
}

RenderBody::~RenderBody()
{
}

void RenderBody::printBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::printDecorations()" << endl;

    QColor c = m_style->backgroundColor();

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();	
    _ty -= borderTopExtra();

    int my = MAX(_ty,_y);
    int mh;
    if (_ty<_y)
    	mh=MAX(0,h-(_y-_ty));
    else
    	mh = MIN(_h,h);

    if( c.isValid() && parent()->style()->backgroundColor().isValid() ) {
	//kdDebug( 6040 ) << "printing bgcolor" << endl;
	p->fillRect(_tx, my, w, mh, c);
    }
    if( m_bgImage && parent()->backgroundImage() ) {
	//kdDebug( 6040 ) << "printing bgimage at " << _tx << "/" << _ty << endl;
	// ### might need to add some correct offsets
	// ### use paddingX/Y
	
	int sx = 0;
	int sy = 0;
	
	if( !m_style->backgroundAttachment() ) {
	    //kdDebug(0) << "fixed background" << endl;
	    QRect r = viewRect();
	    sx = _tx - r.x();
	    sy = _ty - r.y();
	
	} 	

	switch(m_style->backgroundRepeat()) {
	case NO_REPEAT:
	    if(m_bgImage->pixmap().width() < w)
		h = m_bgImage->pixmap().width();
	case REPEAT_X:
	    if(m_bgImage->pixmap().height() < h)
		h = m_bgImage->pixmap().height();
	    break;
	case REPEAT_Y:
	    if(m_bgImage->pixmap().width() < h)
		h = m_bgImage->pixmap().width();
	    break;
	case REPEAT:
	    break;
	}
	p->drawTiledPixmap(_tx, _ty, w, h,m_bgImage->pixmap(), sx, sy);
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

