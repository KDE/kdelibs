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

// -------------------------------------------------------------------------
#ifndef HTML_BLOCKIMPL_H
#define HTML_BLOCKIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"

namespace DOM {

class DOMString;

class HTMLBlockquoteElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLBlockquoteElementImpl(DocumentImpl *doc);

    ~HTMLBlockquoteElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BLOCKQUOTEStartTag; }
    virtual tagStatus endTag() { return BLOCKQUOTEEndTag; }

    virtual void setAvailableWidth(int w = -1);
    virtual void setXPos( int xPos );

    //virtual void layout(bool deep = false);
};

// -------------------------------------------------------------------------

class DOMString;

class HTMLDivElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLDivElementImpl(DocumentImpl *doc);

    ~HTMLDivElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return DIVStartTag; }
    virtual tagStatus endTag() { return DIVEndTag; }
    
    void parseAttribute(Attribute *token);
};

// -------------------------------------------------------------------------

class HTMLHRElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLHRElementImpl(DocumentImpl *doc);

    ~HTMLHRElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual void parseAttribute(Attribute *);

    virtual tagStatus startTag() { return HRStartTag; }
    virtual tagStatus endTag() { return HREndTag; }

    bool noShade() const;
    void setNoShade( bool );

    virtual void layout(bool deep = false);
    virtual void print(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);
    virtual void printObject(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);

protected:
    Length length;
    bool shade;
    int size;
};

// -------------------------------------------------------------------------

class HTMLHeadingElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLHeadingElementImpl(DocumentImpl *doc, ushort _tagid);

    ~HTMLHeadingElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;
    
    virtual void layout(bool deep);

    virtual void setStyle(CSSStyle *currentStyle);
    void parseAttribute(Attribute *token);

    virtual tagStatus startTag() { return H1StartTag; }
    virtual tagStatus endTag() { return H1EndTag; }

protected:
    ushort _id;
};

// -------------------------------------------------------------------------

/*
 * were not using HTMLBlockElementImpl as parent class, since a 
 * paragraph should be able to flow around aligned objects. Thus
 * a <p> element has to be inline, and is rendered by 
 * HTMLBlockImpl::calcParagraph
 */
class HTMLParagraphElementImpl : public HTMLElementImpl
{
public:
    HTMLParagraphElementImpl(DocumentImpl *doc);

    ~HTMLParagraphElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return PStartTag; }
    virtual tagStatus endTag() { return PEndTag; }

    virtual bool isRendered() { return true; }

    virtual void parseAttribute(Attribute *);
};

// -------------------------------------------------------------------------

class HTMLPreElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLPreElementImpl(DocumentImpl *doc);

    ~HTMLPreElementImpl();

    long width() const;
    void setWidth( long w );

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    //virtual void layout( bool deep = false );

    virtual tagStatus startTag() { return PREStartTag; }
    virtual tagStatus endTag() { return PREEndTag; }

    virtual void setStyle(CSSStyle *s);
};

}; //namespace
#endif
