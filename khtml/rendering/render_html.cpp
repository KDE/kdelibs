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
    int h = QMAX(height(), parent()->height());

    //kdDebug(0) << "width = " << w <<endl;

    int bx = _tx - marginLeft();
    int by = _ty - marginTop();
    int bw = QMAX(w + marginLeft() + marginRight() + borderLeft() + borderRight(), parent()->width());
    int bh = QMAX(h + marginTop() + marginBottom() + borderTop() + borderBottom(), parent()->height());

    int my = QMAX(_ty,_y);
    int mh;
    if (_ty<_y)
    	mh= QMAX(0,h-(_y-_ty));
    else
    	mh = QMIN(_h,h);

    //kdDebug() << "my=" << my << " mh=" << mh <<" by=" << by << " height = " << bh << endl;
    
    printBackground(p, c, bg, my, mh, bx, by, bw, bh);

    if(m_style->hasBorder())
	printBorder( p, _tx, _ty, w, h );
}

void RenderHtml::repaint()
{
    RenderObject *cb = containingBlock();
    if(cb != this)
	cb->repaint();
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
