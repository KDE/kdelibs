/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2002 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2002 Dirk Mueller (mueller@kde.org)
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

#include "rendering/render_list.h"
#include "rendering/render_root.h"
#include "html/html_listimpl.h"
#include "misc/helper.h"
#include "misc/htmltags.h"

#include <kdebug.h>
#include <kglobal.h>

//#define BOX_DEBUG

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

static QString toLetter( int number, int base ) {
    number--;
    QString letter = (QChar) (base + (number % 24));
    // Add a "'" at the end of the alphabet
    for (int i = 0; i < (number / 24); i++) {
       letter += QString::fromLatin1("'");
    }
    return letter;
}

static QString toHebrew( int number ) {
    const QChar tenDigit[] = {1497, 1499, 1500, 1502, 1504, 1505, 1506, 1508, 1510};

    QString letter;
    if (number>999) {
  	letter = toHebrew(number/1000) + QString::fromLatin1("'");
   	number = number%1000;
    }

    int hunderts = (number/400);
    if (hunderts > 0) {
	for(int i=0; i<hunderts; i++) {
	    letter += QChar(1511 + 3);
	}
    }
    number = number % 400;
    if ((number / 100) != 0) {
        letter += QChar (1511 + (number / 100) -1);
    }
    number = number % 100;
    int tens = number/10;
    if (tens > 0 && !(number == 15 || number == 16)) {
	letter += tenDigit[tens-1];
    }
    if (number == 15 || number == 16) { // special because of religious
	letter += QChar(1487 + 9);       // reasons
    	letter += QChar(1487 + number - 9);
    } else {
        number = number % 10;
        if (number != 0) {
            letter += QChar (1487 + number);
        }
    }
    return letter;
}

// -------------------------------------------------------------------------

RenderListItem::RenderListItem(DOM::NodeImpl* node)
    : RenderFlow(node)
{
    // init RenderObject attributes
    setInline(false);   // our object is not Inline

    predefVal = -1;
    m_marker = 0;
}

void RenderListItem::setStyle(RenderStyle *_style)
{
    RenderFlow::setStyle(_style);

    RenderStyle *newStyle = new RenderStyle();
    newStyle->inheritFrom(style());
    if(newStyle->direction() == LTR)
        newStyle->setFloating(FLEFT);
    else
        newStyle->setFloating(FRIGHT);

    if(!m_marker && style()->listStyleType() != LNONE) {

        m_marker = new RenderListMarker();
        m_marker->setStyle(newStyle);
        insertChildNode( m_marker, firstChild() );
    } else if ( m_marker && style()->listStyleType() == LNONE) {
        m_marker->detach();
        m_marker = 0;
    }
    else if ( m_marker ) {
        m_marker->setStyle(newStyle);
    }
}

RenderListItem::~RenderListItem()
{
}

void RenderListItem::calcListValue()
{
    // only called from the marker so..
    KHTMLAssert(m_marker);

    if(predefVal != -1)
        m_marker->m_value = predefVal;
    else if(!previousSibling())
        m_marker->m_value = 1;
    else {
	RenderObject *o = previousSibling();
	while ( o && (!o->isListItem() || o->style()->listStyleType() == LNONE) )
	    o = o->previousSibling();
        if( o && o->isListItem() && o->style()->listStyleType() != LNONE ) {
            RenderListItem *item = static_cast<RenderListItem *>(o);
            m_marker->m_value = item->value() + 1;
        }
        else if (parent()->element() && parent()->element()->id() == ID_OL)
            m_marker->m_value = static_cast<DOM::HTMLOListElementImpl*>(parent()->element())->start();
        else
            m_marker->m_value = 1;
    }
}

void RenderListItem::layout( )
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );

    if (m_marker && !m_marker->layouted())
        m_marker->layout();
    RenderFlow::layout();

    m_height = kMax ( m_height, int ( lineHeight( true ) ) );
    if (m_marker)
        m_height = kMax( m_height, m_marker->height() );
}

// -----------------------------------------------------------

RenderListMarker::RenderListMarker()
    : RenderBox(0), m_listImage(0), m_value(-1)
{
    // init RenderObject attributes
    setInline(true);   // our object is Inline
    setReplaced(true); // pretend to be replaced
    // val = -1;
    // m_listImage = 0;
}

RenderListMarker::~RenderListMarker()
{
    if(m_listImage)
        m_listImage->deref(this);
}

void RenderListMarker::setStyle(RenderStyle *s)
{
    if ( s && style() && s->listStylePosition() != style()->listStylePosition() ) {
	setLayouted( false );
	setMinMaxKnown( false );
    }
    RenderBox::setStyle(s);

    if ( m_listImage != style()->listStyleImage() ) {
	if(m_listImage)  m_listImage->deref(this);
	m_listImage = style()->listStyleImage();
	if(m_listImage)  m_listImage->ref(this);
    }
}


void RenderListMarker::print(QPainter *p, int _x, int _y, int _w, int _h,
                             int _tx, int _ty)
{
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderListMarker::printObject(QPainter *p, int, int _y,
                                    int, int _h, int _tx, int _ty)
{
    if (style()->visibility() != VISIBLE) return;

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << nodeName().string() << "(ListMarker)::printObject(" << _tx << ", " << _ty << ")" << endl;
#endif
    p->setFont(style()->font());
    const QFontMetrics fm = p->fontMetrics();
    int offset = fm.ascent()*2/3;

    bool isPrinting = (p->device()->devType() == QInternal::Printer);
    if (isPrinting)
    {
        if (_ty < _y)
        {
            // This has been printed already we suppose.
            return;
        }
        if (_ty + m_height + paddingBottom() + borderBottom() >= _y+_h)
        {
            RenderRoot *rootObj = root();
            if (_ty < rootObj->truncatedAt())
                rootObj->setTruncatedAt(_ty);
            // Let's print this on the next page.
            return;
        }
    }


    int xoff = m_x;
    int yoff = fm.ascent() - offset;


    if ( m_listImage && !m_listImage->isErrorImage()) {
	if ( style()->listStylePosition() != INSIDE ) {
	    if ( style()->direction() == LTR )
		xoff = - m_listImage->pixmap().width();
	    else
		xoff = parent()->width();
	}
	p->drawPixmap( QPoint( _tx + xoff, _ty ), m_listImage->pixmap());
        return;
    }

    if(style()->listStylePosition() != INSIDE) {
	if(style()->direction() == RTL)
	    xoff = 7 + parent()->width();
	else
	    xoff = -7 - offset;

    }

#ifdef BOX_DEBUG
    p->setPen( Qt::red );
    p->drawRect( _tx + xoff, _ty + yoff, offset, offset );
#endif

    const QColor color( style()->color() );
    p->setPen( color );

    switch(style()->listStyleType()) {
    case DISC:
        p->setBrush( color );
        p->drawEllipse( _tx + xoff, _ty + (3 * yoff)/2, (offset>>1)+1, (offset>>1)+1 );
        return;
    case CIRCLE:
        p->setBrush( Qt::NoBrush );
        p->drawEllipse( _tx + xoff, _ty + (3 * yoff)/2, (offset>>1)+1, (offset>>1)+1 );
        return;
    case SQUARE:
        p->setBrush( color );
        p->drawRect( _tx + xoff, _ty + (3 * yoff)/2, (offset>>1)+1, (offset>>1)+1 );
        return;
    case LNONE:
        return;
    default:
        if (m_item != QString::null) {
       	    //_ty += fm.ascent() - fm.height()/2 + 1;
            if(style()->listStylePosition() == INSIDE) {
            	if(style()->direction() == LTR)
        	    p->drawText(_tx, _ty, 0, 0, Qt::AlignLeft|Qt::DontClip, m_item);
            	else
            	    p->drawText(_tx, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, m_item);
            } else {
                if(style()->direction() == LTR)
            	    p->drawText(_tx-offset/2, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, m_item);
            	else
            	    p->drawText(_tx+offset/2 + parent()->width(), _ty, 0, 0, Qt::AlignLeft|Qt::DontClip, m_item);
	    }
        }
    }
}

void RenderListMarker::layout()
{
    KHTMLAssert( !layouted() );
    // ### KHTMLAssert( minMaxKnown() );
    if (m_listImage)
        m_height = m_listImage->pixmap().height();
    else
        m_height = style()->fontMetrics().ascent();

    if ( !minMaxKnown() )
	calcMinMaxWidth();
    setLayouted();
}

void RenderListMarker::setPixmap( const QPixmap &p, const QRect& r, CachedImage *o)
{
    if(o != m_listImage) {
        RenderBox::setPixmap(p, r, o);
        return;
    }

    if(m_width != m_listImage->pixmap_size().width() || m_height != m_listImage->pixmap_size().height())
    {
        setLayouted(false);
        setMinMaxKnown(false);
    }
    else
        repaintRectangle(0, 0, m_width, m_height);
}

void RenderListMarker::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    m_width = 0;

    if(m_listImage) {
        if(style()->listStylePosition() == INSIDE)
            m_width = m_listImage->pixmap().width();
	setMinMaxKnown();
        return;
    }

    if (m_value < 0) { // not yet calculated
        RenderObject* p = parent();
        while (p->isAnonymousBox())
            p = p->parent();
        static_cast<RenderListItem*>(p)->calcListValue();
    }

    const QFontMetrics &fm = style()->fontMetrics();

    switch(style()->listStyleType())
    {
    case DISC:
    case CIRCLE:
    case SQUARE:
        if(style()->listStylePosition() == INSIDE)
            m_width = fm.ascent();
    	goto end;
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
        m_item.sprintf( "%2ld", m_value );
        break;
    case LOWER_ROMAN:
        m_item = toRoman( m_value, false );
        break;
    case UPPER_ROMAN:
        m_item = toRoman( m_value, true );
        break;
    case LOWER_GREEK:
     {
    	int number = m_value - 1;
      	int l = (number % 24);

	if (l>16) {l++;} // Skip GREEK SMALL LETTER FINAL SIGMA

   	m_item = QChar(945 + l);
    	for (int i = 0; i < (number / 24); i++) {
       	    m_item += QString::fromLatin1("'");
    	}
	break;
     }
    case HEBREW:
     	m_item = toHebrew( m_value );
	break;
    case LOWER_ALPHA:
    case LOWER_LATIN:
        m_item = toLetter( m_value, 'a' );
        break;
    case UPPER_ALPHA:
    case UPPER_LATIN:
        m_item = toLetter( m_value, 'A' );
        break;
    case LNONE:
        break;
    }
    m_item += QString::fromLatin1(". ");

    if(style()->listStylePosition() == INSIDE)
	m_width = fm.width(m_item);

end:

    m_minWidth = m_width;
    m_maxWidth = m_width;

    setMinMaxKnown();
}

short RenderListMarker::verticalPositionHint( bool ) const
{
    return 0;
}

void RenderListMarker::calcWidth()
{
    RenderBox::calcWidth();
}

#undef BOX_DEBUG
