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
#include <qnamespace.h>

#include "rendering/render_style.h"

#include <stdio.h>

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

RenderListItem::RenderListItem(RenderStyle *style)
    : RenderFlow(style)
{
    predefVal = -1;
    
    RenderStyle *newStyle = new RenderStyle(style);
    m_marker = new RenderListMarker(newStyle);
    addChild(m_marker);
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
	RenderObject *prev = m_previous;
	while(prev && !prev->isListItem())
	    prev = prev->previousSibling();
	if(prev)
	{
	    RenderListItem *item = static_cast<RenderListItem *>(prev);
	    m_marker->val = item->value() + 1;
	}
	else
	    m_marker->val = 1;
    }
}


void RenderListItem::layout( bool deep )
{
    calcListValue();
    RenderFlow::layout(deep);
}

void RenderListItem::print(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(LI)::print()\n", nodeName().string().ascii());
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

RenderListMarker::RenderListMarker( RenderStyle *style )
    : RenderBox(style)
{
    val = -1;
}

void RenderListMarker::print(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty)
{
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderListMarker::printObject(QPainter *p, int _x, int _y,
				    int _w, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(ListMarker)::printObject(%d, %d)\n", nodeName().string().ascii(), _tx, _ty);
#endif
    
    

    QColor color( style()->color() );
    p->setPen( QPen( color ) );

    switch(m_style->listStyleType())
    {
    case DISC:
	p->setBrush( QBrush( color ) );
	p->drawEllipse( _tx - 13, _ty + 4, 7, 7 );
	return;
    case CIRCLE:
	p->setBrush( QBrush( color ) );
	p->drawArc( _tx -  13, _ty + 4, 7, 7, 0, 16*360 );
	return;
    case SQUARE:
    {
	int xp = _tx+13;
	int yp = _ty -4;
	p->setBrush( QBrush( color ) );
	QCOORD points[] = { xp,yp, xp+7,yp, xp+7,yp+7, xp,yp+7, xp,yp };
	QPointArray a( 5, points );
	p->drawPolyline( a );
	return;
    }
    default:
	if(item != QString::null)
	    p->drawText(_tx-5, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, item);
    }
}

void RenderListMarker::layout(bool)
{	
    calcMinMaxWidth();
    m_height = QFontMetrics(m_style->font()).height();
}

	       
	       
void RenderListMarker::calcMinMaxWidth()
{
    m_width = 0;

    if(m_style->listStylePosition() != INSIDE)
	goto end;
    
    switch(m_style->listStyleType())
    {
    case DISC:
    case CIRCLE:
    case SQUARE:
	if(m_style->listStylePosition() == INSIDE)
	    m_width = 10;
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
	item += ".";
	break;
    case LOWER_ROMAN:
	item = toRoman( val, false );
	item += ".";
	break;
    case UPPER_ROMAN:
	item = toRoman( val, true );
	item += ".";
	break;
    case LOWER_GREEK:
	// ### unsupported, use low-alpha instead
    case LOWER_ALPHA:
    case LOWER_LATIN:
	item = (QChar) ((int)('a' + val - 1));
	item += ".";
	break;
    case UPPER_ALPHA:
    case UPPER_LATIN:
	item = (QChar) ((int)('A' + val - 1));
	item += ".";
	break;
    case LNONE:
	break;
    }
    {
	QFontMetrics fm(m_style->font());
	m_width = fm.width(item);
    }
 end:
    m_minWidth = m_width;
    m_maxWidth = m_width;
}    
