/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#include "render_replaced.h"

#include <qscrollview.h>

using namespace khtml;

void RenderReplaced::print( QPainter *p, int _x, int _y, int _w, int _h,
			    int _tx, int _ty)
{
   _tx += m_x;
   _ty += m_y;

//   if((_ty > _y + _h) || (_ty + m_height < _y)) return;

   if(m_printSpecial) printBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);
   printReplaced(p, _tx, _ty);
}


RenderWidget::RenderWidget(QScrollView *view)
	: RenderReplaced()
{
    m_widget = 0;
    m_view = view;
}


RenderWidget::~RenderWidget()
{
    delete m_widget;
}

void RenderWidget::setStyle(RenderStyle *style)
{
    RenderReplaced::setStyle(style);
    if(m_widget) m_widget->setFont(m_style->font());
}

void RenderWidget::printReplaced(QPainter *, int _tx, int _ty)
{
    if(!(m_widget && m_view)) return;

    // add offset for relative positioning
    if(isRelPositioned())
	relativePositionOffset(_tx, _ty);

    m_view->addChild(m_widget, _tx, _ty);
    	
    if(!m_widget->isVisible())
        m_widget->show();
}

short RenderWidget::verticalPositionHint() const
{
    switch(vAlign())
    {
    case BASELINE:
	//kdDebug(300) << "aligned to baseline" << endl;
	return (contentHeight() - QFontMetrics(m_style->font()).descent());
    case SUB:
	// ###
    case SUPER:
	// ###
    case TOP:
	return PositionTop;
    case TEXT_TOP:
	return QFontMetrics(m_style->font()).ascent();
    case MIDDLE:
	return -QFontMetrics(m_style->font()).width('x')/2;
    case BOTTOM:
	return PositionBottom;
    case TEXT_BOTTOM:
	return QFontMetrics(m_style->font()).descent();
    }
    return 0;
}
