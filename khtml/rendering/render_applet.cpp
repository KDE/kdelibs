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
#include "render_applet.h"

#include <qlabel.h>
#include <qscrollview.h>

using namespace khtml;

RenderApplet::RenderApplet(RenderStyle *style, QScrollView *view, QSize size,
                           KURL url, QStringList *args)
	: RenderWidget(style, view)
{
    printf("Create Renderer applet %s \n", url.url().data());
    
    // Put part code here
    m_widget = new QLabel("I'm an applet", view->viewport());
    m_widget->setBackgroundColor(QColor(0, 0, 200));
    //
    
    m_width = size.width();
    m_height = size.height();
    
    m_widget->resize(m_width, m_height);
}

RenderApplet::~RenderApplet()
{
    printf("Delete Renderer applet\n");
}
