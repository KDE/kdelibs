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

void RenderImage::setPixmap( const QPixmap &p, CachedObject *o, bool *manualUpdate )
{

    if(o != image) {
	RenderReplaced::setPixmap(p, o);
	return;
    }

    if (manualUpdate && *manualUpdate)
    {
//    	kdDebug( 6040 ) << "Image: manualUpdate" << endl;
        updateSize();	
//	repaintRectangle(0, 0, m_width, m_height); //should not be needed!
        return;
    }

    // Image dimensions have been changed, recalculate layout
//    kdDebug( 6040 ) << "Image: setPixmap" << endl;
    if(p.width() != pix.width() || p.height() != pix.height())
    {	
//    	kdDebug( 6040 ) << "Image: newSize " << p.width() << "/" << p.height() << endl;
	pix = p;
	setLayouted(false);
	setMinMaxKnown(false);
//    	kdDebug( 6040 ) << "Image: size " << m_width << "/" << m_height << endl;
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
    	//repaintObject(this, 0, 0);
	repaintRectangle(0, 0, m_width, m_height);
    }
}

void RenderImage::printReplaced(QPainter *p, int _tx, int _ty)
{
    // add offset for relative positioning
    if(isRelPositioned())
	relativePositionOffset(_tx, _ty);

    //kdDebug( 6040 ) << "Image::printObject (" << width() << "/" << height() << ")" << endl;

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    int leftBorder = borderLeft();
    int topBorder = borderTop();


    //kdDebug( 6040 ) << "    contents (" << contentWidth << "/" << contentHeight << ") border=" << borderLeft() << " padding=" << paddingLeft() << endl;

    QRect rect( 0, 0, cWidth, cHeight );

    if ( pix.isNull() )
    {
	QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, Qt::darkGray, Qt::gray,
			      Qt::black, Qt::white );
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
	if ( (cWidth != pix.width() ||
	    cHeight != pix.height() ) &&
	    pix.width() && pix.height() )
	{
	  //kdDebug( 6040 ) << "have to scale: width:" << //   width - border*2 << "<-->" << pix.width() << " height " << //   getHeight() - border << "<-->" << pix.height() << endl;
	    if (resizeCache.isNull())
	    {
		QWMatrix matrix;
		matrix.scale( (float)(cWidth)/pix.width(),
			(float)(cHeight)/pix.height() );
		resizeCache = pix.xForm( matrix );
	    }
	    p->drawPixmap( QPoint( _tx + leftBorder, _ty + topBorder ), resizeCache, rect );
	
	}
	else
	    p->drawPixmap( QPoint( _tx + leftBorder, _ty + topBorder ), pix, rect );
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
//    if(minMaxKnown()) return;
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "Image::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif
//    setMinMaxKnown();

    // contentWidth

    short oldwidth = m_width;

    calcWidth();

    if (oldwidth != m_width)
    	resizeCache.resize(0,0);		

    m_maxWidth = m_minWidth = m_width;

}

void RenderImage::layout()
{
    if(layouted()) return;
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "Image::layout(?) width=" << m_width << ", layouted=" << layouted() << endl;
#endif

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
    return pix.width();
}

int RenderImage::intrinsicHeight() const
{
     return pix.height();
}
