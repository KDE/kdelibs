/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2005 Zack Rusin <zack@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#define DEBUG_LAYOUT

#include "config.h"
#include "render_canvasimage.h"
#include "render_canvas.h"

#include <qpainter.h>
#include <qdebug.h>

#include "css/csshelper.h"
#include "misc/helper.h"
#include "misc/htmltags.h"
#include "html/html_formimpl.h"
#include "html/html_canvasimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "html/html_documentimpl.h"

#include <math.h>

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------

RenderCanvasImage::RenderCanvasImage(NodeImpl *_node)
    : RenderImage(_node), m_drawingContext(0),
      m_drawnImage(0), m_needsImageUpdate(0)
{
}

RenderCanvasImage::~RenderCanvasImage()
{
    delete m_drawingContext; m_drawingContext = 0;
    delete m_drawnImage; m_drawnImage = 0;
}

void RenderCanvasImage::createDrawingContext()
{
    delete m_drawingContext;
    delete m_drawnImage;

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    if ( !cWidth ) {
        cWidth = 300;
        setWidth( cWidth );
    }
    if ( !cHeight ) {
        cHeight = 200;
        setHeight( cHeight );
    }
    m_drawnImage = new QImage( cWidth, cHeight, QImage::Format_ARGB32_Premultiplied );
    //### clear color is bogus
    m_drawnImage->fill( 0xffffffff );
    m_drawingContext = new QPainter( m_drawnImage );
    m_drawingContext->setRenderHint( QPainter::Antialiasing );
}

QPainter *RenderCanvasImage::drawingContext()
{
    if (!m_drawingContext) {
        document()->updateLayout();
        createDrawingContext();
    }
    if ( !m_drawingContext->isActive() ) {
        //### clear color is bogus
        //m_drawnImage->fill( 0xffffffff );
        m_drawingContext->begin( m_drawnImage );
    }

    return m_drawingContext;
}

void RenderCanvasImage::setNeedsImageUpdate()
{
    m_needsImageUpdate = true;
    repaint();
}


void RenderCanvasImage::updateDrawnImage()
{
    m_drawingContext->end();
}

void RenderCanvasImage::paint(PaintInfo& i, int _tx, int _ty)
{
    int x = _tx + m_x;
    int y = _ty + m_y;

    if (shouldPaintBackgroundOrBorder() && i.phase != PaintActionOutline)
        paintBoxDecorations(i, x, y);

    QPainter* p = i.p;

    if (i.phase == PaintActionOutline && style()->outlineWidth() && style()->visibility() == VISIBLE)
        paintOutline(p, x, y, width(), height(), style());

    if (i.phase != PaintActionForeground && i.phase != PaintActionSelection)
        return;

    //if (!shouldPaintWithinRoot(i))
    //return;

    bool isPrinting = (i.p->device()->devType() == QInternal::Printer);
    bool drawSelectionTint = (selectionState() != SelectionNone) && !isPrinting;
    if (i.phase == PaintActionSelection) {
        if (selectionState() == SelectionNone) {
            return;
        }
        drawSelectionTint = false;
    }

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    if ( !cWidth )  cWidth = 300;
    if ( !cHeight ) cHeight = 200;
    int leftBorder = borderLeft();
    int topBorder = borderTop();
    int leftPad = paddingLeft();
    int topPad = paddingTop();

    x += leftBorder + leftPad;
    y += topBorder + topPad;

    if (m_needsImageUpdate) {
        updateDrawnImage();
        m_needsImageUpdate = false;
    }
    //qDebug()<<"drawing image "<<m_drawnImage;
    if (m_drawnImage) {
        HTMLCanvasElementImpl* i = (element() && element()->id() == ID_CANVAS ) ? static_cast<HTMLCanvasElementImpl*>(element()) : 0;
        p->drawImage( QRectF( x, y, cWidth, cHeight ), *m_drawnImage,
                      QRectF( 0, 0, m_drawnImage->width(), m_drawnImage->height() ) );
    }

    // if (drawSelectionTint) {
//         QBrush brush(selectionColor(p));
//         QRect selRect(selectionRect());
//         p->fillRect(selRect.x(), selRect.y(), selRect.width(), selRect.height(), brush);
//     }
}

void RenderCanvasImage::layout()
{
    KHTMLAssert( needsLayout());
    KHTMLAssert( minMaxKnown() );

    short oldwidth = m_width;
    int oldheight = m_height;

    calcWidth();
    calcHeight();

    if ( m_width != oldwidth || m_height != oldheight ) {
        createDrawingContext();
    }

    // if they are variable width and we calculate a huge height or width, we assume they
    // actually wanted the intrinsic width.
    if ( m_width > 4096 && !style()->width().isFixed() )
	m_width = intrinsicWidth() + paddingLeft() + paddingRight() + borderLeft() + borderRight();
    if ( m_height > 2048 && !style()->height().isFixed() )
	m_height = intrinsicHeight() + paddingTop() + paddingBottom() + borderTop() + borderBottom();

    // limit total size to not run out of memory when doing the xform call.
    if ( ( m_width * m_height > 4096*2048 ) &&
         ( contentWidth() > intrinsicWidth() || contentHeight() > intrinsicHeight() ) ) {
	float scale = ::sqrt( m_width*m_height / ( 4096.*2048. ) );
	m_width = (int) (m_width/scale);
	m_height = (int) (m_height/scale);
    }

    setNeedsLayout(false);
}
