/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2002 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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
 */

#include "rendering/render_list.h"
#include "rendering/render_canvas.h"
#include "html/html_listimpl.h"
#include "misc/helper.h"
#include "misc/htmltags.h"
#include "misc/loader.h"
#include "xml/dom_docimpl.h"

#include <kdebug.h>
#include <kglobal.h>
#include <qvaluelist.h>

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
    QValueList<QChar> letters;
    while(number > 0) {
        number--; // number 0 is letter a
        QChar letter = (QChar) (base + (number % 26));
        letters.prepend(letter);
        number /= 26;
    }
    QString str;
    str.setLength(letters.size());
    int i=0;
    while(!letters.isEmpty()) {
        str[i++] = letters.front();
        letters.pop_front();
    }
    return str;
}
/*
static QString toLetter( int number, int base ) {
    number--;
    QString letter = (QChar) (base + (number % 26));
    // Add a "'" at the end of the alphabet
    for (int i = 0; i < (number / 26); i++) {
       letter += '\'';
    }
    return letter;
}*/

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

static QString toAlphabetic( int number, int base, QChar* alphabet ) {
    QValueList<QChar> letters;
    while(number > 0) {
        number--; // number 0 is letter 1
        QChar letter = alphabet[number % base];
        letters.prepend(letter);
        number /= base;
    }
    QString str;
    str.setLength(letters.size());
    int i=0;
    while(!letters.isEmpty()) {
        str[i++] = letters.front();
        letters.pop_front();
    }
    return str;
}

static QString toHiragana( int number ) {
    static QChar hiragana[48] = {0x3042, 0x3044, 0x3046, 0x3048, 0x304A, 0x304B, 0x304D,
                                 0x304F, 0x3051, 0x3053, 0x3055, 0x3057, 0x3059, 0x305B, 0x305D,
                                 0x305F, 0x3061, 0x3064, 0x3066, 0x3068, 0x306A, 0x306B,
                                 0x306C, 0x306D, 0x306E, 0x306F, 0x3072, 0x3075, 0x3078,
                                 0x307B, 0x307E, 0x307F, 0x3080, 0x3081, 0x3082, 0x3084, 0x3086,
                                 0x3088, 0x3089, 0x308A, 0x308B, 0x308C, 0x308D, 0x308F,
                                 0x3090, 0x3091, 0x9092, 0x3093};
    return toAlphabetic( number, 48, hiragana );
}

static QString toHiraganaIroha( int number ) {
    static QChar hiragana[47] = {0x3044, 0x308D, 0x306F, 0x306B, 0x307B, 0x3078, 0x3068,
                                 0x3061, 0x308A, 0x306C, 0x308B, 0x3092, 0x308F, 0x304B,
                                 0x3088, 0x305F, 0x308C, 0x305D, 0x3064, 0x306D, 0x306A,
                                 0x3089, 0x3080, 0x3046, 0x3090, 0x306E, 0x304A, 0x304F, 0x3084,
                                 0x307E, 0x3051, 0x3075, 0x3053, 0x3048, 0x3066, 0x3042, 0x3055,
                                 0x304D, 0x3086, 0x3081, 0x307F, 0x3057, 0x3091, 0x3072, 0x3082,
                                 0x305B, 0x3059 };
    return toAlphabetic( number, 47, hiragana );
}

static QString toKatakana( int number ) {
    static QChar katakana[48] = {0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD,
                                 0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB,
                                 0x30BD, 0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA,
                                 0x30CB, 0x30CC, 0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5,
                                 0x30D8, 0x30DB, 0x30DE, 0x30DF, 0x30E0, 0x30E1, 0x30E2,
                                 0x30E4, 0x30E6, 0x30E8, 0x30E9, 0x30EA, 0x30EB, 0x30EC,
                                 0x30ED, 0x30EF, 0x30F0, 0x30F1, 0x90F2, 0x30F3};
    return toAlphabetic( number, 48, katakana );
}

static QString toKatakanaIroha( int number ) {
    static QChar katakana[47] = {0x30A4, 0x30ED, 0x30CF, 0x30CB, 0x30DB, 0x30D8, 0x30C8,
                                 0x30C1, 0x30EA, 0x30CC, 0x30EB, 0x30F2, 0x30EF, 0x30AB,
                                 0x30E8, 0x30BF, 0x30EC, 0x30ED, 0x30C4, 0x30CD, 0x30CA,
                                 0x30E9, 0x30E0, 0x30A6, 0x30F0, 0x30CE, 0x30AA, 0x30AF,
                                 0x30E4, 0x30DE, 0x30B1, 0x30D5, 0x30B3, 0x30A8, 0x30C6,
                                 0x30A2, 0x30B5, 0x30AD, 0x30E6, 0x30E1, 0x30DF, 0x30B7,
                                 0x30F1, 0x30D2, 0x30E2, 0x30BB, 0x90B9};
    return toAlphabetic( number, 47, katakana );
}

static QString toNumeric( int number, int base ) {
    QString letter = QString::number(number);
    for(unsigned int i = 0; i < letter.length(); i++) {
        letter[i] = QChar(letter[i].digitValue()+base);
    }
    return letter;
}

inline static QString toArabicIndic( int number ) {
    return toNumeric(number, 0x660);
}

inline static QString toPersianUrdu( int number ) {
    return toNumeric(number, 0x6F0);
}

// -------------------------------------------------------------------------

RenderListItem::RenderListItem(DOM::NodeImpl* node)
    : RenderBlock(node)
{
    // init RenderObject attributes
    setInline(false);   // our object is not Inline

    predefVal = -1;
    m_marker = 0;
    m_insideList = false;
    m_deleteMarker = false;
}

void RenderListItem::setStyle(RenderStyle *_style)
{
    RenderBlock::setStyle(_style);

    RenderStyle *newStyle = new RenderStyle();
    newStyle->ref();

    newStyle->inheritFrom(style());

    if(!m_marker && style()->listStyleType() != LNONE) {
        m_marker = new (renderArena()) RenderListMarker(element()->getDocument());
        m_marker->setStyle(newStyle);
        m_marker->setListItem( this );
        m_deleteMarker = true;
    } else if ( m_marker && style()->listStyleType() == LNONE) {
        m_marker->detach();
        m_marker = 0;
    }
    else if ( m_marker ) {
        m_marker->setStyle(newStyle);
    }

    newStyle->deref();
}

void RenderListItem::detach()
{
    if ( m_marker && m_deleteMarker )
        m_marker->detach();
    RenderBlock::detach();
}

static RenderObject* getParentOfFirstLineBox(RenderObject* curr, RenderObject* marker)
{
    RenderObject* firstChild = curr->firstChild();
    if (!firstChild)
        return 0;

    for (RenderObject* currChild = firstChild;
         currChild; currChild = currChild->nextSibling()) {
        if (currChild == marker)
            continue;

        if (currChild->isInline())
            return curr;

        if (currChild->isFloating() || currChild->isPositioned())
            continue;

        if (currChild->isTable() || !currChild->isRenderBlock())
            break;

        if (currChild->style()->htmlHacks() && currChild->element() &&
            (currChild->element()->id() == ID_UL || currChild->element()->id() == ID_OL))
            break;

        RenderObject* lineBox = getParentOfFirstLineBox(currChild, marker);
        if (lineBox)
            return lineBox;
    }

    return 0;
}


void RenderListItem::updateMarkerLocation()
{
    // Sanity check the location of our marker.
    if (m_marker) {
        RenderObject* markerPar = m_marker->parent();
        RenderObject* lineBoxParent = getParentOfFirstLineBox(this, m_marker);
        if (!lineBoxParent) {
            // If the marker is currently contained inside an anonymous box,
            // then we are the only item in that anonymous box (since no line box
            // parent was found).  It's ok to just leave the marker where it is
            // in this case.
            if (markerPar && markerPar->isAnonymous())
                lineBoxParent = markerPar;
            else
                lineBoxParent = this;
        }
        if (markerPar != lineBoxParent)
        {
            if (markerPar)
                markerPar->removeChild(m_marker);
            if (!lineBoxParent)
                lineBoxParent = this;
            lineBoxParent->addChild(m_marker, lineBoxParent->firstChild());
            m_deleteMarker = false;
            if (!m_marker->minMaxKnown())
                m_marker->calcMinMaxWidth();
            recalcMinMaxWidths();
        }
    }
}

void RenderListItem::calcMinMaxWidth()
{
    // Make sure our marker is in the correct location.
    updateMarkerLocation();
    if (!minMaxKnown())
        RenderBlock::calcMinMaxWidth();
}

void RenderListItem::layout( )
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    updateMarkerLocation();
    RenderBlock::layout();
}

void RenderListItem::calcListValue()
{
    // only called from the marker so..
    KHTMLAssert(m_marker);

    if(predefVal != -1)
        m_marker->m_value = predefVal;
    else {
	RenderObject *o = previousSibling();
	while ( o && (!o->isListItem() || o->style()->listStyleType() == LNONE) )
	    o = o->previousSibling();
        if( o && o->isListItem() && o->style()->listStyleType() != LNONE ) {
            RenderListItem *item = static_cast<RenderListItem *>(o);
            m_marker->m_value = item->m_marker->m_value + 1;
        }
        else {
            RenderObject* o = parent();
            while ( o && o->isAnonymous() )
                o = o->parent();

            if (o->element() && o->element()->id() == ID_OL)
                m_marker->m_value = static_cast<DOM::HTMLOListElementImpl*>(o->element())->start();
            else
                m_marker->m_value = 1;
        }
    }
}

void RenderListItem::calcListTotal()
{
    // only called from the marker so..
    KHTMLAssert(m_marker);

    // first see if our siblings have already counted the total
    RenderObject *o = previousSibling();
    while ( o && (!o->isListItem() || o->style()->listStyleType() == LNONE) )
        o = o->previousSibling();
    if( o && o->isListItem() && o->style()->listStyleType() != LNONE ) {
        RenderListItem *item = static_cast<RenderListItem *>(o);
        m_marker->m_total = item->m_marker->m_total;
    }
    else {
        // count total number of sibling items
        const RenderObject* o = parent();
        while ( o && o->isAnonymous() )
            o = o->parent();

        unsigned int count;
        if (o->element() && o->element()->id() == ID_OL)
            count = static_cast<DOM::HTMLOListElementImpl*>(o->element())->start() - 1;
        else
            count = 0;

        o = o->firstChild();
        while (o) {
            if (o->isListItem()) count++;
            o = o->nextSibling();
        }
        m_marker->m_total = count;
    }
}

// -----------------------------------------------------------

RenderListMarker::RenderListMarker(DOM::DocumentImpl* node)
    : RenderBox(node), m_listImage(0), m_value(-1), m_total(-1)
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
    if ( s && style() && s->listStylePosition() != style()->listStylePosition() )
        setNeedsLayoutAndMinMaxRecalc();

    RenderBox::setStyle(s);

    if ( m_listImage != style()->listStyleImage() ) {
	if(m_listImage)  m_listImage->deref(this);
	m_listImage = style()->listStyleImage();
	if(m_listImage)  m_listImage->ref(this);
    }
}


void RenderListMarker::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
    if (paintInfo.phase != PaintActionForeground)
        return;

    if (style()->visibility() != VISIBLE)  return;

    _tx += m_x;
    _ty += m_y;

    if((_ty > paintInfo.r.bottom()) || (_ty + m_height <= paintInfo.r.top()))
        return;

    if(shouldPaintBackgroundOrBorder())
        paintBoxDecorations(paintInfo, _tx, _ty);

    QPainter* p = paintInfo.p;
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << nodeName().string() << "(ListMarker)::paintObject(" << _tx << ", " << _ty << ")" << endl;
#endif
    p->setFont(style()->font());
    const QFontMetrics fm = p->fontMetrics();
    int offset = fm.ascent()*2/3;


    // The marker needs to adjust its tx, for the case where it's an outside marker.
    RenderObject* listItem = 0;
    int leftLineOffset = 0;
    int rightLineOffset = 0;
    if (!listPositionInside()) {
        listItem = this;
        int yOffset = 0;
        int xOffset = 0;
        while (listItem && listItem != m_listItem) {
            yOffset += listItem->yPos();
            xOffset += listItem->xPos();
            listItem = listItem->parent();
        }

        // Now that we have our xoffset within the listbox, we need to adjust ourselves by the delta
        // between our current xoffset and our desired position (which is just outside the border box
        // of the list item).
        if (style()->direction() == LTR) {
            leftLineOffset = m_listItem->leftRelOffset(yOffset, m_listItem->leftOffset(yOffset));
            _tx -= (xOffset - leftLineOffset) + m_listItem->paddingLeft() + m_listItem->borderLeft();
        }
        else {
            rightLineOffset = m_listItem->rightRelOffset(yOffset, m_listItem->rightOffset(yOffset));
            _tx += (rightLineOffset-xOffset) + m_listItem->paddingRight() + m_listItem->borderRight();
        }
    }


    bool isPrinting = (paintInfo.p->device()->devType() == QInternal::Printer);
    if (isPrinting)
    {
        if (_ty < paintInfo.r.y())
        {
            // This has been painted already we suppose.
            return;
        }
        if (_ty + m_height + paddingBottom() + borderBottom() > paintInfo.r.bottom())
        {
            RenderCanvas *rootObj = canvas();
            if (_ty < rootObj->truncatedAt())
                rootObj->setBestTruncatedAt(_ty, this);
            // Let's paint this on the next page.
            return;
        }
    }


    int xoff = 0;
    int yoff = fm.ascent() - offset;

    if (!listPositionInside())
        if (listItem->style()->direction() == LTR)
            xoff = -7 - offset;
        else
            xoff = offset;

    if ( m_listImage && !m_listImage->isErrorImage()) {
	if ( !listPositionInside() ) {
            if (style()->direction() == LTR)
                xoff -= m_listImage->pixmap().width() - fm.ascent()*1/3;
            else
                xoff -= fm.ascent()*1/3;
	}

	p->drawPixmap( QPoint( _tx + xoff, _ty ), m_listImage->pixmap());
        return;
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
        if (!m_item.isNull()) {
            if(listPositionInside()) {
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
    KHTMLAssert( needsLayout() );

    if ( !minMaxKnown() )
        calcMinMaxWidth();

    setNeedsLayout(false);
}

void RenderListMarker::setPixmap( const QPixmap &p, const QRect& r, CachedImage *o)
{
    if(o != m_listImage) {
        RenderBox::setPixmap(p, r, o);
        return;
    }

    if(m_width != m_listImage->pixmap_size().width() || m_height != m_listImage->pixmap_size().height())
        setNeedsLayoutAndMinMaxRecalc();
    else
        repaintRectangle(0, 0, m_width, m_height);
}

void RenderListMarker::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    m_width = 0;

    if(m_listImage) {
        if (listPositionInside())
            m_width = m_listImage->pixmap().width() + 5;
        m_height = m_listImage->pixmap().height();
        m_minWidth = m_maxWidth = m_width;
        setMinMaxKnown();
        return;
    }

    if (m_value < 0)
        m_listItem->calcListValue();

    const QFontMetrics &fm = style()->fontMetrics();
    m_height = fm.ascent();

    switch(style()->listStyleType())
    {
    case DISC:
    case CIRCLE:
    case SQUARE:
        if(listPositionInside())
            m_width = fm.ascent();
    	goto end;
    case ARMENIAN:
    case GEORGIAN:
    case CJK_IDEOGRAPHIC:
    case HIRAGANA:
    case KATAKANA:
    case HIRAGANA_IROHA:
    case KATAKANA_IROHA:
        // ### unsupported, we use decimal instead
    case LDECIMAL:
        m_item.setNum ( m_value );
        break;
    case DECIMAL_LEADING_ZERO: {
        if (m_total < 0)
            m_listItem->calcListTotal();
        int decimals = 2;
        int t = m_total/100;
        while (t>0) {
            t = t/10;
            decimals++;
        }
        decimals = kMax(decimals, 2);
        QString num = QString::number(m_value);
        m_item.fill('0',decimals-num.length());
        m_item.append(num);
        break;
    }
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
    case HIRAGANA:
        m_item = toHiragana( m_value );
        break;
    case HIRAGANA_IROHA:
        m_item = toHiraganaIroha( m_value );
        break;
    case KATAKANA:
        m_item = toKatakana( m_value );
        break;
    case KATAKANA_IROHA:
        m_item = toKatakanaIroha( m_value );
        break;
    case LNONE:
        break;
    }
    m_item += QString::fromLatin1(". ");

    if(listPositionInside())
	m_width = fm.width(m_item);

end:

    m_minWidth = m_width;
    m_maxWidth = m_width;

    setMinMaxKnown();
}

short RenderListMarker::lineHeight(bool /*b*/) const
{
    return height();
}

short RenderListMarker::baselinePosition(bool /*b*/) const
{
    return height();
}

void RenderListMarker::calcWidth()
{
    RenderBox::calcWidth();
}

#undef BOX_DEBUG
