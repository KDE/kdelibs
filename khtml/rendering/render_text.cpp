/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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
//#define BIDI_DEBUG

#if 0
// #### for debugging Qt
#define private public
#include <qfontmetrics.h>
#undef private
#include <qtextcodec.h>
#endif

#include "render_text.h"
#include "render_style.h"

#include "misc/loader.h"

#include <qfontmetrics.h>
#include <qfontinfo.h>
#include <qfont.h>
#include <qpainter.h>
#include <qstring.h>
#include <qcolor.h>
#include <qrect.h>
#include <kdebug.h>

#include <assert.h>

#define QT_ALLOC_QCHAR_VEC( N ) (QChar*) new char[ sizeof(QChar)*( N ) ]
#define QT_DELETE_QCHAR_VEC( P ) delete[] ((char*)( P ))

using namespace khtml;
using namespace DOM;


TextSlave::~TextSlave()
{
    if(deleteText)
        QT_DELETE_QCHAR_VEC(m_text);
}


void TextSlave::print( QPainter *p, int _tx, int _ty)
{
    if (!m_text || len <= 0)
        return;

    QConstString s(m_text, len);
    //kdDebug( 6040 ) << "textSlave::printing(" << s.string() << ") at(" << x+_tx << "/" << y+_ty << ")" << endl;
    p->drawText(x + _tx, y + _ty + m_baseline, s.string());
}

void TextSlave::printSelection(QPainter *p, RenderText* rt, int tx, int ty, int startPos, int endPos)
{
    if(startPos > len) return;
    if(startPos < 0) startPos = 0;

    int _len = len;
    int _width = m_width;
    if(endPos > 0 && endPos < len) {
        _len = endPos;
    }
    _len -= startPos;

    QConstString s(m_text+startPos , _len);

    if (_len != len)
        _width = p->fontMetrics().width(s.string());

    int _offset = 0;
    if ( startPos > 0 ) {
        QConstString aStr(m_text, startPos);
        _offset = p->fontMetrics().width(aStr.string());
    }
    QColor c = rt->style()->color();
    p->setPen(QColor(0xff-c.red(),0xff-c.green(),0xff-c.blue()));
    p->fillRect(x + tx + _offset, y + ty, _width, m_height, c);

    ty += m_baseline;

    //kdDebug( 6040 ) << "textSlave::printing(" << s.string() << ") at(" << x+_tx << "/" << y+_ty << ")" << endl;
    p->drawText(x + tx + _offset, y + ty, s.string());


}
// no blink at the moment...
void TextSlave::printDecoration( QPainter *p, int _tx, int _ty, int deco)
{
    _tx += x;
    _ty += y;

    int underlineOffset = m_height/7 + m_baseline;
    if(underlineOffset == m_baseline) underlineOffset++;

    if(deco & UNDERLINE)
        p->drawLine(_tx, _ty + underlineOffset, _tx + m_width, _ty + underlineOffset );
    if(deco & OVERLINE)
        p->drawLine(_tx, _ty, _tx + m_width, _ty );
    if(deco & LINE_THROUGH)
        p->drawLine(_tx, _ty + 2*m_baseline/3, _tx + m_width, _ty + 2*m_baseline/3 );
// ### add BLINK
}

void TextSlave::printBoxDecorations(QPainter *pt, RenderText *p, int _tx, int _ty, bool begin, bool end)
{
    _tx += x;
    _ty += y - p->paddingTop() - p->borderTop();

    //kdDebug( 6040 ) << "renderBox::printDecorations()" << endl;
    RenderStyle *style = p->style();

    int width = m_width;
    if(begin)
        _tx -= p->paddingLeft() + p->borderLeft();

    QColor c = style->backgroundColor();
    CachedImage *i = style->backgroundImage();
    if(c.isValid() && (!i || i->tiled_pixmap(c).mask()))
        pt->fillRect(_tx, _ty, width, m_height, c);

    if(i)
    {
        // ### might need to add some correct offsets
        // ### use paddingX/Y
        pt->drawTiledPixmap(_tx + p->borderLeft(), _ty + p->borderTop(),
                            m_width + p->paddingLeft() + p->paddingRight(),
                            m_height + p->paddingTop() + p->paddingBottom(), i->tiled_pixmap(c));
    }

    if(style->hasBorder())
    {
        int h = m_height + p->paddingTop() + p->paddingBottom() + p->borderTop() + p->borderBottom();

        if(style->borderTopStyle() != BNONE)
        {
            c = style->borderTopColor();
            if(!c.isValid()) c = style->color();
            p->drawBorder(pt, _tx, _ty, _tx + width, _ty, style->borderTopWidth(),
                       RenderObject::BSTop, c, style->borderTopStyle());
        }
        if(style->borderBottomStyle() != BNONE)
        {
            c = style->borderBottomColor();
            if(!c.isValid()) c = style->color();
            p->drawBorder(pt, _tx, _ty + h, _tx + width, _ty + h, style->borderBottomWidth(),
                       RenderObject::BSBottom, c, style->borderBottomStyle());
        }
        // ### has to be changed for RTL
        if(style->borderLeftStyle() != BNONE && (begin))
        {
            c = style->borderLeftColor();
            if(!c.isValid()) c = style->color();
            p->drawBorder(pt, _tx, _ty, _tx, _ty + h, style->borderLeftWidth(),
                       RenderObject::BSLeft, c, style->borderLeftStyle());
        }
        if(style->borderRightStyle() != BNONE && end)
        {
            c = style->borderRightColor();
            if(!c.isValid()) c = style->color();
            p->drawBorder(pt, _tx + width, _ty, _tx + width, _ty + h, style->borderRightWidth(),
                       RenderObject::BSRight, c, style->borderRightStyle());
        }
    }

#ifdef BIDI_DEBUG
    int h = m_height + p->paddingTop() + p->paddingBottom() + p->borderTop() + p->borderBottom();
    c = QColor("#0000ff");
    p->drawBorder(pt, _tx, _ty, _tx, _ty + h, 1,
                  RenderObject::BSLeft, c, SOLID);
    p->drawBorder(pt, _tx + width, _ty, _tx + width, _ty + h, style->borderRightWidth(),
                  RenderObject::BSRight, c, SOLID);
#endif
}

void TextSlave::printActivation( QPainter *p, int _tx, int _ty)
{
  p->drawRect(_tx+x,_ty+y-1, m_width, m_height+2);
  p->drawRect(_tx+x-1,_ty+y, m_width+2, m_height);
}


bool TextSlave::checkPoint(int _x, int _y, int _tx, int _ty)
{
    if((_ty + y > _y) || (_ty + y + m_height < _y) ||
       (_tx + x > _x) || (_tx + x + m_width < _x))
        return false;
    return true;
}

// -------------------------------------------------------------------------------------

RenderText::RenderText(DOMStringImpl *_str)
    : RenderObject()
{
    // init RenderObject attributes
    m_isText = true;   // our object inherits from RenderText
    m_inline = true;   // our object is Inline

    m_first = 0;
    m_last = 0;
    m_minWidth = -1;
    m_maxWidth = -1;
    str = _str;
    if(str) str->ref();
    fm = 0;

    m_selectionState = SelectionNone;

#ifdef DEBUG_LAYOUT
    QConstString cstr(str->s, str->l);
    kdDebug( 6040 ) << "RenderText::setText '" << (const char *)cstr.string().utf8() << "'" << endl;
#endif
}

void RenderText::setStyle(RenderStyle *style)
{
    RenderObject::setStyle(style);
    delete fm;
    fm = new QFontMetrics(m_style->font());
    m_contentHeight = m_style->lineHeight().width(fm->height());
}

RenderText::~RenderText()
{
    deleteSlaves();
    delete fm; fm = 0;
    if(str) str->deref();
}

void RenderText::deleteSlaves()
{
    // delete all slaves
    TextSlave *s = m_first;
    while(s)
    {
        TextSlave *next = s->next();
        delete s;
        s = next;
    }
    m_first = m_last = 0;
}

bool RenderText::checkPoint(int _x, int _y, int _tx, int _ty, int &offset)
{
    TextSlave *s = m_first;
    while(s)
    {
        if( s->checkPoint(_x, _y, _tx, _ty) )
        {
            // now we need to get the exact position
            int delta = _x - _tx - s->x;
            int pos = 0;
            while(pos < s->len)
            {
                // ### this will produce wrong results for RTL text!!!
                int w = fm->width(*(s->m_text+pos));
                int w2 = w/2;
                w = w - w2;
                delta -= w2;
                if(delta <= 0) break;
                pos++;
                delta -= w;
            }
            offset = s->m_text - m_first->m_text + pos;
            //kdDebug( 6040 ) << " Text  --> inside at position " << offset << endl;

            return true;
        }
        // ### this might be wrong, if combining chars are used ( eg arabic )
        s=s->next();
    }
    return false;
}

void RenderText::cursorPos(int offset, int &_x, int &_y, int &height)
{
  if (!m_first) {
    _x = _y = height = -1;
    return;
  }

  _x = 0;
  TextSlave *s = m_first;
  int off = s->len;

  while(offset > off && s->next())
  {
      s=s->next();
      off = s->m_text - m_first->m_text + s->len;
  }   // we are now in the correct text slave

  int pos = (offset > off ? s->len : s->len - (off - offset ));
  _y = s->y;
  height = s->m_height;

  QString tekst(s->m_text, s->len);
  _x = s->x + (fm->boundingRect(tekst, pos)).right();
  if(pos)
      _x += fm->rightBearing( *(s->m_text + pos - 1 ) );

  int absx, absy;
  absolutePosition(absx,absy);
  if (absx == -1) {
    // we don't know out absolute position, and there is not point returning
    // just a relative one
    _x = _y = -1;
  }
  else {
    _x += absx;
    _y += absy;
  }

}

void RenderText::absolutePosition(int &xPos, int &yPos, bool)
{
    if(m_parent) {
        m_parent->absolutePosition(xPos, yPos, false);
        if ( m_first ) {
            xPos += m_first->x;
            yPos += m_first->y;
        }
    } else
        xPos = yPos = -1;
}

void RenderText::posOfChar(int chr, int &x, int &y)
{
    if (!m_parent)
    {
       x = -1;
       y = -1;
       return;
    }
    m_parent->absolutePosition( x, y, false );
    if( chr > (int) str->l )
	chr = str->l;

    TextSlave *s = m_first;
    TextSlave *last = s;
    QChar *ch = str->s + chr;
    while ( s && ch >= s->m_text )
    {
        last = s;
	s = s->next();
    }

    x += last->x;
    y += last->y;
}

void RenderText::printObject( QPainter *p, int /*x*/, int y, int /*w*/, int h,
                      int tx, int ty)
{
    //kdDebug( 6040 ) << "Text::printObject(long)" << endl;

    TextSlave *s = m_first;

    //kdDebug( 6040 ) << "Text::printObject(2)" << endl;

    bool start = true;
#ifndef BIDI_DEBUG
    if(m_printSpecial && m_parent->isInline())
#endif
    {
        bool breakallowed = false;
        while(s)
        {
            bool end = false;
            if(!s->next()) end = true;
            if(s->checkVerticalPoint(y, ty, h))
            {
                breakallowed = true;
                s->printBoxDecorations(p, this, tx, ty, start, end);
            }
            else if (breakallowed)
                break;

            s=s->next();
            start = false;
        }
        s = m_first;
    }

    p->setFont( m_style->font() );
    //kdDebug( 6040 ) << "charset used: " << m_style->font().charSet() << endl;
#if 0
    kdDebug( 6040 ) << "charset used: " << m_style->font().charSet() << " mapper: " << fm->mapper()->name() << endl;
#endif

    // ### as QPainter::drawText only honors the pen color, we can avoid
    // updating the pen if the text color is still correct - this saves
    // quite some time
    if(m_style->color() != p->pen().color())
        p->setPen( m_style->color() );

    // as the textslaves are ordered from top to bottom
    // as soon as we find one that is "below" our
    // printing range, we can quit
    // ### better if textslaves are placed in a double linked list
    bool breakallowed = false;
    while(s)
    {
        if(s->checkVerticalPoint(y, ty, h))
        {
            breakallowed = true;
            s->print(p, tx, ty);
        }
        else if(breakallowed)
            break;

        s=s->next();
    }

    int d = m_style->textDecoration();
    if(d != TDNONE && hasKeyboardFocus==DOM::ActivationOff)
    {
        p->setPen( m_style->textDecorationColor() );
        s = m_first;
        bool breakallowed = false;
        while(s)
        {
            if(s->checkVerticalPoint(y, ty, h))
            {
                breakallowed = true;
                s->printDecoration(p, tx, ty, d);
            }
            else if(breakallowed)
                break;

            s=s->next();
        }
    }


    s = m_first;
    if (selectionState() != SelectionNone)
    {
        int endPos, startPos;
        bool breakAtEnd = false;
        if (selectionState()==SelectionInside)
        {
            startPos = 0;
            endPos = -1;
        }
        else
        {
            selectionStartEnd(startPos,endPos);
            breakAtEnd = true;
            if(selectionState() == SelectionStart) {
                endPos = -1;
                breakAtEnd = false;
            }
            else if(selectionState() == SelectionEnd)
                startPos = 0;
//          kdDebug( 6040 ) << "selectionstartend start=" << startPos << " end=" << endPos << endl;
        }

        breakallowed = false;
        while(s && endPos)
        {
            if(s->checkVerticalPoint(y, ty, h))
            {
                breakallowed = true;
                s->printSelection(p, this, tx, ty, startPos, endPos);
            }
            else if(breakallowed)
                break;

            int diff;
            if(s->next())
                diff = s->next()->m_text - s->m_text;
            else
                diff = s->len;
            endPos -= diff;
            startPos -= diff;
            if(breakAtEnd && endPos < 0) break;
            s=s->next();
        }
    }
    if (hasKeyboardFocus!=DOM::ActivationOff)
    {
        bool clip = p->hasClipping();
	p->setClipping(false);
	p->setRasterOp(Qt::XorROP);
        if (hasKeyboardFocus==DOM::ActivationPassive)
          p->setPen(QColor("white"));
        else
          p->setPen(QColor("blue"));

        breakallowed = false;
        while(s)
        {
            if(s->checkVerticalPoint(y, ty, h))
            {
                breakallowed = true;
                s->printActivation(p, tx, ty);
            }
            else if(breakallowed)
                break;

            int diff;
            if(s->next())
                diff = s->next()->m_text - s->m_text;
            else
                diff = s->len;
            s=s->next();
        }
	p->setRasterOp(Qt::CopyROP);
	p->setClipping(clip);
    }
#ifdef BIDI_DEBUG
    p->setPen(QPen(QColor("#00CC00"), 1, Qt::DashLine));
    p->setBrush( Qt::NoBrush );
    p->drawRect(tx + boundingRect.x(), ty + boundingRect.y(), boundingRect.width(), boundingRect.height());
#endif
}

void RenderText::print( QPainter *p, int x, int y, int w, int h,
                      int tx, int ty)
{
    if ( !m_visible )
        return;

    printObject(p, x, y, w, h, tx, ty);
}

void RenderText::calcMinMaxWidth()
{
    //kdDebug( 6040 ) << "Text::calcMinMaxWidth(): known=" << minMaxKnown() << endl;

    if(minMaxKnown()) return;

    // ### calc Min and Max width...
    m_minWidth = 0;
    m_maxWidth = 0;

    int currMinWidth = 0;
    int currMaxWidth = 0;

    int len = str->l;
    for(int i = 0; i < len; i++)
    {
        int wordlen = 0;
        char c;
        do {
            wordlen++;
        } while( !(isBreakable( str->s+i+wordlen )) && i+wordlen < len); // && c != '-'
        if(i+wordlen < len) wordlen--;
        if (wordlen)
        {
            int w = fm->width(QConstString(str->s+i, wordlen).string());
            currMinWidth += w;
            currMaxWidth += w;
        }
        if(i+wordlen < len)
        {
            if ( c == '\n' )
            {
                assert(c == '\n');
                if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
                currMinWidth = 0;
                if(currMaxWidth > m_maxWidth) m_maxWidth = currMaxWidth;
                currMaxWidth = 0;
            }
            else
            {
                if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
                currMinWidth = 0;
                currMaxWidth += fm->width( *(str->s+i+wordlen) );
            }
            /* else if( c == '-')
            {
                currMinWidth += minus_width;
                if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
                currMinWidth = 0;
                currMaxWidth += minus_width;
            }*/
        }
        i += wordlen;
    }
    if(currMinWidth > m_minWidth) m_minWidth = currMinWidth;
    currMinWidth = 0;
    if(currMaxWidth > m_maxWidth) m_maxWidth = currMaxWidth;
    currMaxWidth = 0;
    setMinMaxKnown();
}

int RenderText::xPos() const
{
    if (m_first)
        return m_first->x;
    else
        return 0;
}

int RenderText::yPos() const
{
    if (m_first)
        return m_first->y;
    else
        return 0;
}

const QFont &RenderText::font()
{
    return parent()->style()->font();
}

void RenderText::setText(DOMStringImpl *text)
{
    if(str) str->deref();
    str = text;
    if(str) str->ref();

    setLayouted(false);
    containingBlock()->setLayouted(false);
    containingBlock()->layout();
#ifdef DEBUG_LAYOUT
    QConstString cstr(str->s, str->l);
    kdDebug( 6040 ) << "RenderText::setText '" << (const char *)cstr.string().utf8() << "'" << endl;
#endif
}

int RenderText::height() const
{
    return m_contentHeight
        + m_style->borderTopWidth() + m_style->borderBottomWidth();
   // ### padding is relative to the _width_ of the containing block
    //+ m_style->paddingTop() + m_style->paddingBottom()
 }

int RenderText::bidiHeight() const
{
    return m_contentHeight;
}

short RenderText::baselineOffset() const
{
    return (m_contentHeight - fm->height())/2 + fm->ascent();
}

short RenderText::verticalPositionHint() const
{
    return (m_contentHeight - fm->height())/2 + fm->ascent();
}

void RenderText::position(int x, int y, int from, int len, int width, bool reverse)
{
    // ### should not be needed!!!
    if(len == 0) return;

    QChar *ch;
    bool deleteChar = false;
#if QT_VERSION < 221
    // Qt still uses the old BiDi code with 8859-6/8...
    if((reverse && (( !m_style->visuallyOrdered() &&
                      font().charSet() != QFont::ISO_8859_8 &&
                      font().charSet() != QFont::ISO_8859_6)
                    ||
                    ( m_style->visuallyOrdered() &&
                      ( font().charSet() == QFont::ISO_8859_8 ||
                        font().charSet() == QFont::ISO_8859_6))
        )))
    {
        deleteChar = true;
        // reverse String
        QString aStr = QConstString(str->s+from, len).string();
#ifdef DEBUG_LAYOUT
        kdDebug( 6040 ) << "reversing '" << (const char *)aStr.utf8() << "' len=" << aStr.length() << " oldlen=" << len << endl;
#endif
        len = aStr.length();
        ch = QT_ALLOC_QCHAR_VEC(len);
        int half =  len/2;
        const QChar *s = aStr.unicode();
        for(int i = 0; i <= half; i++)
        {
            ch[len-1-i] = s[i];
            ch[i] = s[len-1-i];
            if(ch[i].mirrored() && !m_style->visuallyOrdered())
                ch[i] = ch[i].mirroredChar();
            if(ch[len-1-i].mirrored() && !m_style->visuallyOrdered() && i != len-1-i)
                ch[len-1-i] = ch[len-1-i].mirroredChar();
        }
    }
    else if ( reverse && !m_style->visuallyOrdered() &&
              (font().charSet() == QFont::ISO_8859_8 || font().charSet() == QFont::ISO_8859_6 ) ) {
        deleteChar = true;
        QString aStr = QConstString(str->s+from, len).string();
        len = aStr.length();
        ch = QT_ALLOC_QCHAR_VEC(len);
        const QChar *s = aStr.unicode();
        for( int i = 0; i < len; i++ ) {
            if( s[i].mirrored() )
                ch[i] = s[i].mirroredChar();
            else
                ch[i] = s[i];
        }
    }
    else
        ch = str->s+from;
#else
    if ( reverse && !m_style->visuallyOrdered() ) {
        deleteChar = true;
        // reverse String
        QString aStr = QConstString(str->s+from, len).string();
#ifdef DEBUG_LAYOUT
        kdDebug( 6040 ) << "reversing '" << (const char *)aStr.utf8() << "' len=" << aStr.length() << " oldlen=" << len << endl;
#endif
        len = aStr.length();
        ch = QT_ALLOC_QCHAR_VEC(len);
        int half =  len/2;
        const QChar *s = aStr.unicode();
        for(int i = 0; i <= half; i++)
        {
            ch[len-1-i] = s[i];
            ch[i] = s[len-1-i];
            if(ch[i].mirrored() && !m_style->visuallyOrdered())
                ch[i] = ch[i].mirroredChar();
            if(ch[len-1-i].mirrored() && !m_style->visuallyOrdered() && i != len-1-i)
                ch[len-1-i] = ch[len-1-i].mirroredChar();
        }
    }
    else
        ch = str->s+from;
#endif
    // ### margins and RTL
    if(from == 0 && m_parent->isInline() && m_parent->firstChild()==this)
    {
        x += paddingLeft() + borderLeft() + marginLeft();
        width -= marginLeft();
    }

    if(from + len == int(str->l) && m_parent->isInline() && m_parent->lastChild()==this)
        width -= marginRight();

#ifdef DEBUG_LAYOUT
    QConstString cstr(ch, len);
    kdDebug( 6040 ) << "setting slave text to '" << (const char *)cstr.string().utf8() << "' len=" << len << " width=" << width << " at (" << x << "/" << y << ")" << " height=" << bidiHeight() << " fontHeight=" << fm->height() << " ascent =" << fm->ascent() << endl;
#endif

    TextSlave *s = new TextSlave(x, y, ch, len,
                                 bidiHeight(), baselineOffset(), width, deleteChar);

    if(!m_first)
        m_first = m_last = s;
    else
    {
        m_last->setNext(s);
        m_last = s;
    }
}

unsigned int RenderText::width( int from, int len) const
{
    if(!str->s) return 0;

    int w;
    if( len == 1)
        w = fm->width( *(str->s+from) );
    else
        w = fm->width(QConstString(str->s+from, len).string());

    // ### add margins and support for RTL

    if(m_parent->isInline())
    {
        if(from == 0 && m_parent->firstChild() == static_cast<const RenderObject*>(this))
            w += borderLeft() + paddingLeft() + marginLeft();
        if(from + len == int(str->l) &&
           m_parent->lastChild() == static_cast<const RenderObject*>(this))
            w += borderRight() + paddingRight() +marginRight();;
    }

    //kdDebug( 6040 ) << "RenderText::width(" << from << ", " << len << ") = " << w << endl;
    return w;
}

void RenderText::repaint()
{
    RenderObject *cb = containingBlock();
    if(cb != this)
        cb->repaint();
}

bool RenderText::isFixedWidthFont() const
{
    return QFontInfo(m_style->font()).fixedPitch();
}

#undef BIDI_DEBUG
