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

#include "render_image.h"

#include <qpixmap.h>
#include <qdrawutil.h>
#include <qpalette.h>

#include <kdebug.h>

#include "rendering/render_style.h"

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------

RenderImage::RenderImage()
    : RenderReplaced()
{
    bComplete = true;
    setLayouted(false);
    setParsing(false);
    image = 0;
}

RenderImage::~RenderImage()
{
    if(image) image->deref(this);
}

void RenderImage::setPixmap( const QPixmap &p, const QRect& r, CachedImage *o, bool *manualUpdate )
{
    if(o != image) {
        RenderReplaced::setPixmap(p, r, o);
        return;
    }

    if (manualUpdate && *manualUpdate)
    {
        updateSize();
        return;
    }

    // Image dimensions have been changed, recalculate layout
//    kdDebug( 6040 ) << "Image: setPixmap" << endl;

    if(o->pixmap_size() !=  pixSize)
    {
        //kdDebug( 6040 ) << "Image: newSize " << p.width() << "/" << p.height() << endl;
        pix = p;
        pixSize = o->pixmap_size();
        //kdDebug( 6040 ) << "Image size is now " << pixSize.width() << " " << pixSize.height() << endl;
        setLayouted(false);
        setMinMaxKnown(false);
        //kdDebug( 6040 ) << "m_width: : " << m_width << " height: " << m_height << endl;
        //kdDebug( 6040 ) << "Image: size " << m_width << "/" << m_height << endl;
        // the updateSize() call should trigger a repaint too
        if (manualUpdate) {
           *manualUpdate = true;
        }
        else
        {
            updateSize();
        }
    }
    else
    {
        pix = p;
        repaintRectangle(r.x(), r.y(), r.width(), r.height());
    }
}

void RenderImage::printReplaced(QPainter *p, int _tx, int _ty)
{
    // add offset for relative positioning
    if(isRelPositioned())
        relativePositionOffset(_tx, _ty);

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    int leftBorder = borderLeft();
    int topBorder = borderTop();

    //kdDebug( 6040 ) << "    contents (" << contentWidth << "/" << contentHeight << ") border=" << borderLeft() << " padding=" << paddingLeft() << endl;
    if ( pix.isNull() )
    {
        QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, Qt::darkGray, Qt::gray,
                              Qt::black, Qt::white );
        //qDebug("qDrawShadePanel %d/%d/%d/%d", _tx + leftBorder, _ty + topBorder, cWidth, cHeight);
        qDrawShadePanel( p, _tx + leftBorder, _ty + topBorder, cWidth, cHeight,
                         colorGrp, true, 1 );
        if(!alt.isEmpty())
        {
            QString text = alt.string();
            p->setFont(style()->font());
            p->setPen( style()->color() );
            int ax = _tx + leftBorder + 5;
            int ay = _ty + topBorder + 5;
            int ah = cHeight - 10;
            int aw = cWidth - 10;
            QFontMetrics fm(style()->font());
            if (aw>15 && ah>fm.height())
                p->drawText(ax, ay, aw, ah , Qt::WordBreak, text );
        }
    }
    else
    {
        if ( (cWidth != image->pixmap_size().width() ||  cHeight != image->pixmap_size().height() ) &&
             pix.width() && pix.height() && image->valid_rect().isValid())
        {
//            kdDebug( 6040 ) << "have to scale: " << endl;
//            kdDebug( 6040 ) << "cWidth " << cWidth << " cHeight " << cHeight
//                            << " pw: " << image->pixmap_size().width()
//                             << " ph: " << image->pixmap_size().height() << endl;

            QRect scaledrect(image->valid_rect());
            //scaling does not work if w or h is 1
            if (scaledrect.width()==1) scaledrect.setWidth(2);
            if (scaledrect.height()==1) scaledrect.setHeight(2);
                        
            if (resizeCache.isNull() || image->valid_rect().size() != resizeCache.size())
            {
                QWMatrix matrix;
                matrix.scale( (float)(cWidth)/pix.width(),
                        (float)(cHeight)/pix.height() );
                resizeCache = pix.xForm( matrix );
                scaledrect = matrix.map(scaledrect);
            }
            //qDebug("scaled paint rect %d/%d/%d/%d", scaledrect.x(), scaledrect.y(), scaledrect.width(), scaledrect.height());
            p->drawPixmap( QPoint( _tx + leftBorder, _ty + topBorder ), resizeCache, scaledrect );

        }
        else
        {
            // we might be just about switching images
            // so pix contains the old one (we want to paint), but image->valid_rect is still invalid
            // so use pixSize instead.
            // ### maybe no progressive loading for the second image ?
            QRect rect(image->valid_rect().isValid() ? image->valid_rect() : QRect(0, 0, pixSize.width(), pixSize.height())) ;
            //qDebug("normal paint rect %d/%d/%d/%d", rect.x(), rect.y(), rect.width(), rect.height());
            p->drawPixmap( QPoint( _tx + leftBorder, _ty + topBorder ), pix, rect );
        }
    }
    if (hasKeyboardFocus!=DOM::ActivationOff)
      {
        if (hasKeyboardFocus==DOM::ActivationPassive)
          p->setPen(QColor("green"));
        else
          p->setPen(QColor("blue"));
        p->drawRect( _tx + leftBorder, _ty + topBorder, cWidth, cHeight);
        p->drawRect( _tx + leftBorder+1, _ty + topBorder+1, cWidth-2, cHeight-2);
      }
}

void RenderImage::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "Image::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif
    short oldwidth = m_width;

    calcWidth();

    if (oldwidth != m_width)
        resizeCache.resize(0,0);

    m_maxWidth = m_minWidth = m_width;
    setMinMaxKnown();
}

void RenderImage::layout()
{
    if(layouted())
    {
#ifdef DEBUG_LAYOUT
        qDebug("should not be called");
#endif
        return;
    }

    calcMinMaxWidth();
    calcHeight();

    setLayouted();
}

void RenderImage::setImageUrl(DOMString url, DOMString baseUrl, DocLoader *docLoader)
{
    if(image) image->deref(this);
    image = docLoader->requestImage(url, baseUrl);
    image->ref(this);
}

void RenderImage::setAlt(DOM::DOMString text)
{
    alt = text;
}

short RenderImage::baselineOffset() const
{
    switch(m_style->verticalAlign())
    {
    case BASELINE:
    case SUB:
    case SUPER:
    case BOTTOM:
        return contentHeight();
    case TOP:
        return 0;
    case TEXT_TOP:
        return QFontMetrics(m_style->font()).ascent();
    case MIDDLE:
        return contentHeight()/2;
    case TEXT_BOTTOM:
        return contentHeight()-QFontMetrics(m_style->font()).descent();
    }
    return 0;
}

int RenderImage::bidiHeight() const
{
    return height();
}


short RenderImage::intrinsicWidth() const
{
    return pixSize.width();
}

int RenderImage::intrinsicHeight() const
{
    return pixSize.height();
}
