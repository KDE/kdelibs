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

#ifndef HTML_BASEIMPL_H
#define HTML_BASEIMPL_H 1

#include "dtd.h"
#include "html_elementimpl.h"

#define BORDER 5

class KHTMLWidget;

namespace DOM {

class DOMString;

class HTMLBodyElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLBodyElementImpl(DocumentImpl *doc, KHTMLWidget *view = 0);

    ~HTMLBodyElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BODYStartTag; }
    virtual tagStatus endTag() { return BODYEndTag; }

    virtual void parseAttribute(Attribute *);
    	
//    virtual void layout(bool);  // just use BlockElement layout instead


protected:
    KHTMLWidget *view;
};

// -------------------------------------------------------------------------

class HTMLFrameElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLFrameElementImpl(DocumentImpl *doc);

    ~HTMLFrameElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMEStartTag; }
    virtual tagStatus endTag() { return FRAMEEndTag; }
    bool noResize() const;

    void setNoResize( bool );

    virtual void parseAttribute(Attribute *);
    virtual void layout(bool);

    virtual bool isInline() { return false; }
    virtual void attach(KHTMLWidget *w);
    virtual void detach();

protected:
    DOMString url;
    DOMString name;
    KHTMLWidget *view;
    KHTMLWidget *parentWidget;
};

// -------------------------------------------------------------------------

class HTMLFrameSetElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLFrameSetElementImpl(DocumentImpl *doc);

    ~HTMLFrameSetElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMESETStartTag; }
    virtual tagStatus endTag() { return FRAMESETEndTag; }

    virtual void parseAttribute(Attribute *);
    virtual void layout(bool);

    virtual bool isInline() { return false; }
    NodeImpl *addChild(NodeImpl *child);
    void close();
    virtual void attach(KHTMLWidget *w);
    virtual void detach();

protected:
    QList<Length> *rows;
    QList<Length> *cols;
    int *rowHeight;
    int *colWidth;
};

// -------------------------------------------------------------------------

class HTMLHeadElementImpl : public HTMLElementImpl
{
public:
    HTMLHeadElementImpl(DocumentImpl *doc);

    ~HTMLHeadElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual bool isInline() { return false; }

    virtual tagStatus startTag() { return HEADStartTag; }
    virtual tagStatus endTag() { return HEADEndTag; }

    virtual void print(QPainter *, int, int, int, int, int, int) {}
    virtual void layout(bool) {}

};

// -------------------------------------------------------------------------

class HTMLHtmlElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLHtmlElementImpl(DocumentImpl *doc);

    ~HTMLHtmlElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return HTMLStartTag; }
    virtual tagStatus endTag() { return HTMLEndTag; }

    virtual int getWidth() const { return width; }
    virtual NodeImpl *addChild(NodeImpl *child);

    virtual void getAbsolutePosition(int &xPos, int &yPos);
    virtual bool isInline() { return false; }

    virtual void layout(bool);

protected:
    int width;
};

}; //namespace

#endif

