/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
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
#include "rendering/render_body.h"
#include "rendering/render_canvas.h"
#include "html/html_baseimpl.h"
#include "xml/dom_docimpl.h"
#include "khtmlview.h"

#include <kglobal.h>
#include <kdebug.h>

using namespace khtml;
using namespace DOM;

RenderBody::RenderBody(HTMLBodyElementImpl* element)
    : RenderBlock(element)
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

    RenderBlock::setStyle(style);
    element()->getDocument()->setTextColor( style->color() );
    scrollbarsStyled = false;
}

void RenderBody::paintBoxDecorations(PaintInfo& paintInfo, int _tx, int _ty)
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

    int my = kMax(_ty, paintInfo.r.y());
    int end = kMin( paintInfo.r.y()+paintInfo.r.height(), _ty + h );
    int mh = end - my;

    paintBackground(paintInfo.p, c, bg, my, mh, _tx, _ty, w, h);

    if(style()->hasBorder())
	paintBorder( paintInfo.p, _tx, _ty, w, h, style() );

}

void RenderBody::repaint(bool immediate)
{
    RenderObject *cb = containingBlock();
    if(cb)
	cb->repaint(immediate);
}

void RenderBody::layout()
{
    RenderBlock::layout();

    if (!scrollbarsStyled)
    {
	RenderCanvas* canvas = this->canvas();
        if (canvas->view())
        {
            canvas->view()->horizontalScrollBar()->setPalette(style()->palette());
            canvas->view()->verticalScrollBar()->setPalette(style()->palette());
        }
        scrollbarsStyled=true;
    }
}

int RenderBody::availableHeight() const
{
    int h = RenderBlock::availableHeight();

    if( style()->marginTop().isFixed() )
        h  -= style()->marginTop().value();
    if( style()->marginBottom().isFixed() )
        h -= style()->marginBottom().value();

    return kMax(0, h);
}
