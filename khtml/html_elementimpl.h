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
#ifndef HTML_ELEMENTIMPL_H
#define HTML_ELEMENTIMPL_H

#include <qptrdict.h>
#include <qstring.h>
#include <qfontmetrics.h>

#include "dtd.h"
#include "dom_elementimpl.h"
#include "khtmltags.h"
#include "khtmlstyle.h"


class CSSStyle;

namespace DOM {

class DOMString;

class HTMLElementImpl : public ElementImpl
{
public:
    HTMLElementImpl(DocumentImpl *doc);

    virtual ~HTMLElementImpl();

    // make it pure virtual...
    virtual ushort id() const = 0;

    virtual void setStyle(CSSStyle *style);
    virtual CSSStyle *style() { return _style; };
    virtual const HTMLFont *getFont() { return ::getFont(_style); }

    DOMString getAttribute(int attr_id)
	{
	    int i = attributeMap.find(attr_id);
	    if( i != -1 ) return attributeMap.value(i);
	    //return defaultMap()....
	    return 0;
	}

    virtual HAlign hAlign() { return halign; }
    virtual void print(QPainter *p, int _x, int _y, int _w, int _h, 
		       int _tx, int _ty);

    virtual void calcMinMaxWidth();
    virtual void setAvailableWidth(int w = -1);
    virtual void updateSize();
    virtual void close();

    virtual short getMinWidth() const { return minWidth; }
    virtual short getMaxWidth() const { return maxWidth; }

    virtual bool mouseEvent( int x, int y, int button, MouseEventType, 
			     int _tx, int _ty, DOMString &url);
    virtual void mouseEventHandler( int button, MouseEventType type );

    virtual void getAbsolutePosition(int &xPos, int &yPos); 
    virtual NodeImpl *addChild(NodeImpl *newChild);

    void parseAttribute(Attribute *token);

protected:
    /*
     * the style of the current object. Is also used to render inline children
     * ### not sure, if this is the right way in the long term...
     */
    CSSStyle *_style; 

    HAlign halign;
    /*
     * the minimum width the element needs, to be able to render 
     * it's content without clipping
     */
    short minWidth;
    /* The maximum width the element can fill horizontally
     * ( = the width of the element with line breaking disabled)
     */
    short maxWidth;

    /* The width available to the element
     * If the element need more width (minWidth > avWidth) it tells it's
     * parent
     */
    short availableWidth;
        
};

class HTMLInlineElementImpl : public HTMLElementImpl
{
public:
    HTMLInlineElementImpl(DocumentImpl *doc, ushort i);

    virtual ~HTMLInlineElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const { return _id; };

    // overrides NodeImpl

    virtual void setStyle(CSSStyle *style);

protected:
    ushort _id;
};


class HTMLPositionedElementImpl : public HTMLElementImpl
{
public:
    HTMLPositionedElementImpl(DocumentImpl *doc)
	: HTMLElementImpl(doc)
	{ x = y = width = ascent = descent = 0; }

    virtual ~HTMLPositionedElementImpl() {}

    virtual void setPos( int xPos, int yPos ) { x = xPos, y = yPos; }
    virtual void setXPos( int xPos ) { x = xPos; }
    virtual void setYPos( int yPos ) { y = yPos; }
    virtual void setWidth(int w) { width = w; }
    virtual void setAscent(int a) { ascent = a; }
    virtual void setDescent(int d) { descent = d; }

    virtual int getXPos() const { return x; }
    virtual int getYPos() const { return y; }
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return ascent+descent; }
    virtual int getAscent() const { return ascent; }
    virtual int getDescent() const { return descent; }

    virtual void updateSize();

    virtual bool mouseEvent( int x, int y, int button, MouseEventType, 
			     int _tx, int _ty, DOMString &url);

    virtual void getAbsolutePosition(int &xPos, int &yPos);
    virtual NodeImpl *addChild(NodeImpl *newChild);

protected:
    int x;
    int y;
    short width;
    int ascent;
    int descent;
};


class HTMLParagraphElementImpl;

/**
 * all geometry managing stuff is only in the block elements.
 *
 * Inline elements don't paint themselves, but the whole paragraph
 * gets painted by the surrounding block element. This is, because
 * one needs to know the whole paragraph to calculate bidirectional
 * behaviour of text, so putting the drawing routines in the inline
 * elments is impossible.
 */
class HTMLBlockElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLBlockElementImpl(DocumentImpl *doc);

    virtual ~HTMLBlockElementImpl();

    virtual bool isInline() { return false; }

    // overrides NodeImpl

    virtual void print( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    virtual void printObject( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);

    virtual void calcMinMaxWidth();
    virtual void layout( bool deep = false );

    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void setAvailableWidth(int w = -1);

protected:
    NodeImpl *calcParagraph(NodeImpl *child, bool pre = false);
    void      calcFloating(NodeImpl *child, int elemY);
  
    NodeImpl *aligned;
    NodeImpl *startPar;
    
    inline int getRightMargin(int y);
    inline int getLeftMargin(int y);
    inline int getWidth(int y);
    
    void insertMarginElement(HAlign align, int y, NodeImpl* node);    
    void clearMargins();    
    
    
private:
    
    // rendering helpers;
    class HTMLParagraphClose : public HTMLElementImpl {
    public:
    	HTMLParagraphClose() : HTMLElementImpl(0L) {}
    	    virtual ushort id() const {
    	    return ID_P + ID_CLOSE_TAG;
    	}
    };

    static HTMLParagraphClose pElemClose;    
    
    struct MarginRange {
    	int startY;
	int endY;
	int width;	
	NodeImpl* node;
    };
    
    QList<MarginRange>* leftMargin; 
    QList<MarginRange>* rightMargin; 

};

class HTMLGenericBlockElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLGenericBlockElementImpl(DocumentImpl *doc, ushort tagId)
	: HTMLBlockElementImpl(doc)
	{ _id = tagId; }

    virtual const DOMString nodeName() const;
    virtual ushort id() const { return _id; };
    virtual void setStyle(CSSStyle *style);

protected:
    ushort _id;
};


}; //namespace

#endif
