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
    val = -1;
}

RenderListItem::~RenderListItem()
{
}

void RenderListItem::calcListValue()
{
    if(predefVal != -1)
	val = predefVal;
    else if(!m_previous)
	val = 1;
    else
    {
	RenderObject *prev = m_previous;
	while(prev && !prev->isListItem())
	    prev = prev->previousSibling();
	if(prev)
	{
	    RenderListItem *item = static_cast<RenderListItem *>(prev);
	    val = item->value() + 1;
	}
	else
	    val = 1;
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

void RenderListItem::printIcon(QPainter *p, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(LI)::printObject(%d, %d)\n", nodeName().string().ascii(), _tx, _ty);
#endif

    // ### this should scale with the font size in the body... possible?
    int yp = _ty + 4;
    int xp = _tx - 13;

    QColor color( style()->color() );
    p->setPen( QPen( color ) );

    QString item;

    switch(m_style->listStyleType())
    {
    case DISC:
	p->setBrush( QBrush( color ) );
	p->drawEllipse( xp, yp, 7, 7 );
	break;
    case CIRCLE:
	p->setBrush( QBrush( color ) );
	p->drawArc( xp, yp, 7, 7, 0, 16*360 );
	break;
    case SQUARE:
    {
	p->setBrush( QBrush( color ) );
	QCOORD points[] = { xp,yp, xp+7,yp, xp+7,yp+7, xp,yp+7, xp,yp };
	QPointArray a( 5, points );
	p->drawPolyline( a );
	break;
    }
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
    if(item != QString::null)
    p->drawText(_tx-5, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, item);
}

void RenderListItem::printObject(QPainter *p, int _x, int _y,
				    int _w, int _h, int _tx, int _ty)
{
    printIcon(p, _tx, _ty);
    RenderFlow::printObject(p, _x, _y, _w, _h, _tx, _ty);
}

