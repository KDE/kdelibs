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
#include "render_root.h"
#include "html/html_baseimpl.h"
#include "xml/dom_docimpl.h"
#include "khtmlview.h"

#include <kglobal.h>
#include <kdebug.h>

using namespace khtml;
using namespace DOM;

RenderBody::RenderBody(HTMLBodyElementImpl* element)
    : RenderFlow(element)
{
    scrollbarsStyled = false;
}

RenderBody::~RenderBody()
{
}

void RenderBody::setStyle(RenderStyle* style)
{
//     qDebug("RenderBody::setStyle()");
    // ignore position: on body
    if (style->htmlHacks() && style->position() != STATIC)
        style->setPosition(STATIC);

    RenderFlow::setStyle(style);
    element()->getDocument()->setTextColor( style->color() );
    scrollbarsStyled = false;
}

void RenderBody::paintBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintDecorations()" << endl;
    QColor c;
    if( parent()->style()->backgroundColor().isValid() )
	c =  style()->backgroundColor();
    CachedImage *bg = 0;
    if( parent()->style()->backgroundImage() )
	bg = style()->backgroundImage();

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    _ty -= borderTopExtra();

    int my = QMAX(_ty,_y);
    int end = QMIN( _y + _h,  _ty + h );
    int mh = end - my;

    paintBackground(p, c, bg, my, mh, _tx, _ty, w, h);

    if(style()->hasBorder())
	paintBorder( p, _tx, _ty, w, h, style() );

}

void RenderBody::repaint()
{
    RenderObject *cb = containingBlock();
    if(cb != this)
	cb->repaint();
}

void RenderBody::layout()
{
    RenderFlow::layout();

    if (!scrollbarsStyled)
    {
        if (root()->view())
        {
            root()->view()->horizontalScrollBar()->setPalette(style()->palette());
            root()->view()->verticalScrollBar()->setPalette(style()->palette());
        }
        scrollbarsStyled=true;
    }
}

int RenderBody::availableHeight() const
{
    int h = RenderFlow::availableHeight();

    if( style()->marginTop().isFixed() )
        h  -= style()->marginTop().value;
    if( style()->marginBottom().isFixed() )
        h -= style()->marginBottom().value;

    return kMax(0, h);
}
