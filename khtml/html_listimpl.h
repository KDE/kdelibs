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
#ifndef HTML_LISTIMPL_H
#define HTML_LISTIMPL_H

/*
 * we ignore the deprecated compact attribute. Netscape does so too...
 */

#include "dtd.h"
#include "html_elementimpl.h"

#include "khtmllayout.h"

namespace DOM
{

class DOMString;

class HTMLUListElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLUListElementImpl(DocumentImpl *doc);

    ~HTMLUListElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return ULStartTag; }
    virtual tagStatus endTag() { return ULEndTag; }

    bool compact() const { return false; }
    void setCompact( bool ) {};

    virtual void parseAttribute(Attribute *);
    virtual void layout(bool deep = false);
    virtual void print(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);
    virtual void printObject(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);

    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void setAvailableWidth(int w = -1);

    ListType type() { return _type; }
    void setType(ListType t) { _type = t; }
protected:
    ListType _type;
};

// -------------------------------------------------------------------------

class HTMLDirectoryElementImpl : public HTMLUListElementImpl
{
public:
    HTMLDirectoryElementImpl(DocumentImpl *doc);

    ~HTMLDirectoryElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return DIRStartTag; }
    virtual tagStatus endTag() { return DIREndTag; }
};

// -------------------------------------------------------------------------

class HTMLMenuElementImpl : public HTMLUListElementImpl
{
public:
    HTMLMenuElementImpl(DocumentImpl *doc);

    ~HTMLMenuElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return MENUStartTag; }
    virtual tagStatus endTag() { return MENUEndTag; }
};

// -------------------------------------------------------------------------

class HTMLOListElementImpl : public HTMLUListElementImpl
{
public:
    HTMLOListElementImpl(DocumentImpl *doc);

    ~HTMLOListElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return OLStartTag; }
    virtual tagStatus endTag() { return OLEndTag; }

    long start() const;
    void setStart( long );

    virtual void layout(bool deep = false);
};

// -------------------------------------------------------------------------

class HTMLLIElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLLIElementImpl(DocumentImpl *doc);

    ~HTMLLIElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return LIStartTag; }
    virtual tagStatus endTag() { return LIEndTag; }

    long value() const { return val; }
    void setValue( long v ) { predefVal = v; }
    int calcListValue( long v );
    void setType(ListType _t) { t = _t; }

    virtual void print(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);
    virtual void printObject(QPainter *p, int x, int y, int w, int h, 
		       int xoff, int yoff);
    void printIcon(QPainter *p, int _tx, int _ty);

protected:
    long int predefVal;
    long int val;
    ListType t;
};

// -------------------------------------------------------------------------

class HTMLDListElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLDListElementImpl(DocumentImpl *doc);

    ~HTMLDListElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return DLStartTag; }
    virtual tagStatus endTag() { return DLEndTag; }

    bool compact() const { return false; }
    void setCompact( bool ) { }

    virtual void layout(bool deep = false);
};

}; //namespace

#endif
