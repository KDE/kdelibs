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
#include "render_root.h"

#include "khtmlview.h"
#include <stdio.h>
using namespace khtml;

RenderRoot::RenderRoot(RenderStyle *style, KHTMLView *view)
    : RenderFlow(style)
{
    m_view = view;
    // try to contrain the width to the views width
    m_minWidth = view->frameWidth();
    m_width = m_minWidth;
    m_maxWidth = m_minWidth;
    setParsing();
}


void RenderRoot::calcWidth()
{
    m_width = m_view->frameWidth();
    if(m_width < m_minWidth) m_width = m_minWidth;
    if(m_maxWidth != m_minWidth) m_maxWidth = m_minWidth;
}

QScrollView *RenderRoot::view()
{
    return m_view;
}

void RenderRoot::absolutePosition(int &xPos, int &yPos)
{
    xPos = yPos = 0;
}


void RenderRoot::repaintRectangle(int x, int y, int w, int h)
{
    x += m_x;
    y += m_y;
//    printf("updating views contents (%d/%d) (%d/%d)\n", x,y,w,h);
    if (m_view) m_view->updateContents(x, y, w, h);
}

void RenderRoot::repaint()
{
    if (m_view) m_view->updateContents(0, 0, m_width, m_height);
}

void RenderRoot::repaintObject(RenderObject *o, int x, int y)
{
    x += m_x;
    y += m_y;

    if (m_view) m_view->paintElement(o, x, y);
}

void RenderRoot::updateSize()
{
//    printf("%s(RenderRoot)::updateSize()\n", renderName());
    //int oldMin = m_minWidth;
    setLayouted(false);
    calcMinMaxWidth();

    int oldHeight = m_height;
    layout(true);	
    if(m_height != oldHeight)
    {
    	printf("resizing %d,%d\n",m_width,m_height);
    	m_view->resizeContents(m_width,m_height);
//	repaint();
    }
}

void RenderRoot::close()
{
    setParsing(false);
    updateSize();
    repaint();
//    printTree();
}
