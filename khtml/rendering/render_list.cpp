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
#include "render_list.h"

#include <qpainter.h>
#include <qcolor.h>
#include <qfontmetrics.h>
#include <qnamespace.h>

#include "rendering/render_style.h"

#include <kdebug.h>

using namespace khtml;

static QString toRoman( int number, bool upper )
{
    QString roman;
    QChar ldigits[] = { 'i', 'v', 'x', 'l', 'c', 'd', 'm' };
    QChar udigits[] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    QChar *digits = upper ? udigits : ldigits;
    int i, d = 0;

    do
    {
	int num = number % 10;

	if ( num % 5 < 4 )
	    for ( i = num % 5; i > 0; i-- )
		roman.insert( 0, digits[ d ] );

	if ( num >= 4 && num <= 8)
	    roman.insert( 0, digits[ d+1 ] );

	if ( num == 9 )
	    roman.insert( 0, digits[ d+2 ] );

	if ( num % 5 == 4 )
	    roman.insert( 0, digits[ d ] );

	number /= 10;
	d += 2;
    }
    while ( number );

    return roman;
}

// -------------------------------------------------------------------------

RenderListItem::RenderListItem()
    : RenderFlow()
{
    predefVal = -1;
    m_marker = 0;
}

void RenderListItem::setStyle(RenderStyle *style)
{
    RenderFlow::setStyle(style);
    if(!m_marker) {
	RenderStyle *newStyle = new RenderStyle(style);
	m_marker = new RenderListMarker();
	m_marker->setStyle(newStyle);
	addChild(m_marker);
    } else {
	RenderStyle *newStyle = new RenderStyle(style);
	m_marker->setStyle(newStyle);
    }
}

RenderListItem::~RenderListItem()
{
}

void RenderListItem::calcListValue()
{
    if(predefVal != -1)
	m_marker->val = predefVal;
    else if(!m_previous)
	m_marker->val = 1;
    else
    {
	if(m_previous && m_previous->isListItem())
	{
	    RenderListItem *item = static_cast<RenderListItem *>(m_previous);
	    m_marker->val = item->value() + 1;
	}
	else
	    m_marker->val = 1;
    }
}


void RenderListItem::layout( )
{
    calcListValue();
    m_marker->layout();
    RenderFlow::layout();
}

void RenderListItem::print(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << nodeName().string() << "(LI)::print()" << endl;
#endif
    RenderFlow::print(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderListItem::printObject(QPainter *p, int _x, int _y,
				    int _w, int _h, int _tx, int _ty)
{
    // ### this should scale with the font size in the body... possible?
    //m_marker->printIcon(p, _tx, _ty);
    RenderFlow::printObject(p, _x, _y, _w, _h, _tx, _ty);
}

// -----------------------------------------------------------

RenderListMarker::RenderListMarker()
    : RenderBox()
{
    val = -1;
    listImage = 0;
}

RenderListMarker::~RenderListMarker()
{
    if(listImage)
	listImage->deref(this);
    delete m_style;
}

void RenderListMarker::setStyle(RenderStyle *s)
{
    RenderBox::setStyle(s);

    if(listImage)
	listImage->deref(this);
    listImage = m_style->listStyleImage();
    if(listImage)
	listImage->ref(this);
}


void RenderListMarker::print(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty)
{
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderListMarker::printObject(QPainter *p, int, int,
				    int, int, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT

        kdDebug( 6040 ) << nodeName().string() << "(ListMarker)::printObject(" << _tx << ", " << _ty << ")" << endl;
#endif
    p->setFont(m_style->font());
    p->setPen(m_style->color());
    QFontMetrics fm = p->fontMetrics();
    int offset = fm.ascent()*2/3;

#ifdef BOX_DEBUG
    p->setPen( QColor("red") );
    QCOORD points[] = { _tx,_ty, _tx+offset,_ty, _tx+offset,_ty+offset, _tx,_ty+offset, _tx,_ty };
    QPointArray a( 5, points );
    p->drawPolyline( a );
#endif

    int xoff = 0;
    int yoff = fm.ascent() - offset;

    if(m_style->listStylePosition() != INSIDE) {
	xoff = -7 - offset;
	if(m_style->direction() == RTL)
	    xoff = -xoff + m_parent->width();
    }

    if ( listImage ) {
	p->drawPixmap( QPoint( _tx + xoff, _ty ), listImage->pixmap());
 	return;
    }

    QColor color( style()->color() );
    p->setPen( QPen( color ) );

    switch(m_style->listStyleType()) {
    case DISC:
	p->setBrush( QBrush( color ) );
	p->drawEllipse( _tx + xoff, _ty + yoff, offset, offset );
	return;
    case CIRCLE:
	p->setBrush( QBrush( color ) );
	p->drawArc( _tx + xoff, _ty + yoff, offset, offset, 0, 16*360 );
	return;
    case SQUARE:
    {
	int xp = _tx + xoff;
	int yp = _ty + fm.ascent() - offset + 1;
	p->setBrush( QBrush( color ) );
	QCOORD points[] = { xp,yp, xp+offset,yp, xp+offset,yp+offset, xp,yp+offset, xp,yp };
	QPointArray a( 5, points );
	p->drawPolyline( a );
	return;
    }
    default:
	if(item != QString::null) {
	    //_ty += fm.ascent() - fm.height()/2 + 1;
	    if(m_style->listStylePosition() == INSIDE) {
		if(m_style->direction() == LTR)
		    p->drawText(_tx, _ty, 0, 0, Qt::AlignLeft|Qt::DontClip, item);
		else
		    p->drawText(_tx, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, item);
	    } else {
		if(m_style->direction() == LTR)
		    p->drawText(_tx-offset/2, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, item);
		else
		    p->drawText(_tx+offset/2 + m_parent->width(), _ty, 0, 0, Qt::AlignLeft|Qt::DontClip, item);
	    }
	}
    }
}

void RenderListMarker::layout()
{	
    calcMinMaxWidth();

    if(listImage) {
	m_height = listImage->pixmap().height();
	return;
    }

    m_height = QFontMetrics(m_style->font()).height();
}

void RenderListMarker::setPixmap( const QPixmap &p, const QRect& r, CachedImage *o, bool *manualUpdate )
{
    if (manualUpdate && *manualUpdate)
    {
        updateSize();	
        repaintRectangle(0, 0, m_width, m_height); //should not be needed!
        return;
    }

    if(o != listImage)
	RenderBox::setPixmap(p, r, o);

    if(m_width != listImage->pixmap_size().width() || m_height != listImage->pixmap_size().height())
    {	
	setLayouted(false);
	setMinMaxKnown(false);
	layout();
	// the updateSize() call should trigger a repaint too
        if (manualUpdate)
        {
            *manualUpdate = true;
        }
        else
        {
            updateSize();	
	    repaintRectangle(0, 0, m_width, m_height); //should not be needed!
        }
    }
    else
    {
	repaintRectangle(0, 0, m_width, m_height);
    }
}
	
void RenderListMarker::calcMinMaxWidth()
{
    m_width = 0;

    if(listImage) {
	if(m_style->listStylePosition() == INSIDE)
	    m_width = listImage->pixmap().width();
	return;
    }

    switch(m_style->listStyleType())
    {
    case DISC:
    case CIRCLE:
    case SQUARE:
	if(m_style->listStylePosition() == INSIDE) {
	    QFontMetrics fm(m_style->font());
	    m_width = fm.ascent();
	}
	else
	    m_width = 0;
	goto end;
    case HEBREW:
    case ARMENIAN:
    case GEORGIAN:
    case CJK_IDEOGRAPHIC:
    case HIRAGANA:
    case KATAKANA:
    case HIRAGANA_IROHA:
    case KATAKANA_IROHA:
    case DECIMAL_LEADING_ZERO:
	// ### unsupported, we use decimal instead
    case LDECIMAL:
	item.sprintf( "%2ld", val );
	item += '.';
	break;
    case LOWER_ROMAN:
	item = toRoman( val, false );
	item += '.';
	break;
    case UPPER_ROMAN:
	item = toRoman( val, true );
	item += '.';
	break;
    case LOWER_GREEK:
	// ### unsupported, use low-alpha instead
    case LOWER_ALPHA:
    case LOWER_LATIN:
	item = (QChar) ((int)('a' + val - 1));
	item += '.';
	break;
    case UPPER_ALPHA:
    case UPPER_LATIN:
	item = (QChar) ((int)('A' + val - 1));
	item += '.';
	break;
    case LNONE:
	break;
    }
    if(m_style->listStylePosition() != INSIDE)
	m_width = 0;
    else {
	QFontMetrics fm(m_style->font());
	m_width = fm.width(item);
    }
 end:

    m_minWidth = m_width;
    m_maxWidth = m_width;
}

short RenderListMarker::baselineOffset() const
{
    return QFontMetrics(m_style->font()).ascent();
}

short RenderListMarker::verticalPositionHint() const
{
    return QFontMetrics(m_style->font()).ascent();
}
