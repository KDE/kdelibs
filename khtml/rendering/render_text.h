/*
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
#ifndef RENDERTEXT_H
#define RENDERTEXT_H

#include "dom_string.h"
#include "dom_stringimpl.h"
#include "render_object.h"

#include <assert.h>

class QPainter;
class QFontMetrics;

namespace khtml
{
    class RenderText;
    class RenderStyle;

class TextSlave
{
public:
    TextSlave(int _x, int _y, QChar *text, int _len,
	      int height, int baseline, int width, bool _deleteText = false)
	{
	    x = _x;
	    y = _y;
	    m_text = text;
	    len = _len;
	    n = 0;
	    m_height = height;
	    m_baseline = baseline;
	    m_width = width;
	    deleteText = _deleteText;
	}
    ~TextSlave() { if(deleteText) delete [] m_text; }

    void print( QPainter *p, int _tx, int _ty);
    void printDecoration( QPainter *p, int _tx, int _ty, int decoration);
    void printBoxDecorations(QPainter *p, RenderText *parent, int _tx, int _ty, bool begin, bool end);
    bool checkPoint(int _x, int _y, int _tx, int _ty);
    bool checkVerticalPoint(int _y, int _ty, int _h)
{ if((_ty + y > _y + _h) || (_ty + y + m_height < _y)) return false; return true; }
    void printSelection(QPainter *p, int tx, int ty, int startPos, int endPos);


    void setNext(TextSlave *_n) { n = _n; }
    TextSlave *next() { return n; }

    int x;
    int y;
    QChar *m_text;
    int len;

    TextSlave *n;
    unsigned short m_height;
    unsigned short m_baseline;
    unsigned short m_width;

    // this is needed for right to left text. In this case, m_text will point to a QChar array which
    // holds the already reversed string. The slave has to delete this string by itself.
    bool deleteText;
};


class RenderText : public RenderObject
{
public:
    RenderText(DOM::DOMStringImpl *_str);
    virtual ~RenderText();

    virtual const char *renderName() const { return "RenderText"; }

    virtual void setStyle(RenderStyle *style);

    virtual bool isText() const { return true; }
    virtual bool isInline() const { return true; }
    virtual bool isRendered() const { return true; }

    virtual void print( QPainter *, int x, int y, int w, int h,
			int tx, int ty);
    virtual void printObject( QPainter *, int x, int y, int w, int h,
			int tx, int ty);

    void deleteSlaves();

    DOM::DOMString data() const { return str; }
    DOM::DOMStringImpl *string() { return str; }

    virtual void layout(bool /*deep*/ = false) {assert(false);}

    bool checkPoint(int _x, int _y, int _tx, int _ty, int &off);

    virtual unsigned int length() const { return str->l; }
    virtual const QChar *text() const { return str->s; }
    virtual void position(int x, int y, int from, int len, int width, bool reverse);
    virtual unsigned int width( int from, int len) const;

    virtual int height() const;

    // from BiDiObject
    // height of the contents (without paddings, margins and borders)
    virtual int bidiHeight() const;

    // overrides
    virtual void calcMinMaxWidth();
    virtual short minWidth() const { return m_minWidth; }
    virtual short maxWidth() const { return m_maxWidth; }

    virtual int xPos() const;
    virtual int yPos() const;

    virtual short baselineOffset() const;
    virtual short verticalPositionHint() const;

    virtual const QFont &font();

    void setText(DOM::DOMStringImpl *text);

    TextSlave *first() { return m_first; }
    TextSlave *last() { return m_last; }

    virtual SelectionState selectionState() const {return m_selectionState;}
    virtual void setSelectionState(SelectionState s) {m_selectionState = s; }
    virtual void cursorPos(int offset, int &_x, int &_y, int &height);
    virtual void absolutePosition(int &/*xPos*/, int &/*yPos*/);


protected:
    short m_minWidth;
    short m_maxWidth;
    int m_contentHeight;

    SelectionState m_selectionState : 3 ;

    QFontMetrics *fm;
    DOM::DOMStringImpl *str;
    TextSlave *m_first;
    TextSlave *m_last;
};


};
#endif
