/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *	     (C) 2000 Simon Hausmann <hausmann@kde.org>
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
#include <qscrollview.h>

class KHTMLView;

namespace khtml {
    class RenderFrame;
    class RenderPartObject;
}

namespace DOM {

class DOMString;
    class CSSStyleSheetImpl;

class HTMLBodyElementImpl : public HTMLElementImpl
{
public:
    HTMLBodyElementImpl(DocumentImpl *doc);
    ~HTMLBodyElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BODYStartTag; }
    virtual tagStatus endTag() { return BODYEndTag; }

    virtual void parseAttribute(AttrImpl *);
    void attach(KHTMLView *w);

    CSSStyleSheetImpl *sheet() const { return m_style; }
protected:
    CSSStyleSheetImpl *m_style;
};

// -------------------------------------------------------------------------

class HTMLFrameElementImpl : public HTMLElementImpl
{
    friend class khtml::RenderFrame;
    friend class khtml::RenderPartObject;
public:
    HTMLFrameElementImpl(DocumentImpl *doc);

    ~HTMLFrameElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMEStartTag; }
    virtual tagStatus endTag() { return FRAMEEndTag; }

    virtual void parseAttribute(AttrImpl *);
    virtual void attach(KHTMLView *w);
    virtual void detach();

    bool noResize() { return noresize; }

protected:
    DOMString url;
    DOMString name;
    KHTMLView *view;
    KHTMLView *parentWidget;

    bool frameBorder;
    int marginWidth;
    int marginHeight;
    bool noresize;
    QScrollView::ScrollBarMode scrolling;
};

// -------------------------------------------------------------------------

class HTMLFrameSetElementImpl : public HTMLElementImpl
{
public:
    HTMLFrameSetElementImpl(DocumentImpl *doc);

    ~HTMLFrameSetElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMESETStartTag; }
    virtual tagStatus endTag() { return FRAMESETEndTag; }

    virtual void parseAttribute(AttrImpl *);
    virtual NodeImpl *addChild(NodeImpl *child);
    virtual void attach(KHTMLView *w);

    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType type,
		     int _tx, int _ty, DOMString &url,
                             NodeImpl *&innerNode, long &offset);

    bool frameBorder() { return frameborder; }
    bool noResize() { return noresize; }

    int totalRows() const { return m_totalRows; }
    int totalCols() const { return m_totalCols; }
    int border() const { return m_border; }

protected:
    int m_totalRows;
    int m_totalCols;

    QList<khtml::Length> *m_rows;
    QList<khtml::Length> *m_cols;

    // mozilla and others use this in the frameset, although it's not standard html4
    bool frameborder;
    int m_border;
    bool noresize;

    bool m_resizing;  // is the user resizing currently

    KHTMLView *view;
};

// -------------------------------------------------------------------------

class HTMLHeadElementImpl : public HTMLElementImpl
{
public:
    HTMLHeadElementImpl(DocumentImpl *doc);

    ~HTMLHeadElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return HEADStartTag; }
    virtual tagStatus endTag() { return HEADEndTag; }
};

// -------------------------------------------------------------------------

class HTMLHtmlElementImpl : public HTMLElementImpl
{
public:
    HTMLHtmlElementImpl(DocumentImpl *doc);

    ~HTMLHtmlElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return HTMLStartTag; }
    virtual tagStatus endTag() { return HTMLEndTag; }

};


// -------------------------------------------------------------------------

class HTMLIFrameElementImpl : public HTMLFrameElementImpl
{
public:
    HTMLIFrameElementImpl(DocumentImpl *doc);

    ~HTMLIFrameElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return IFRAMEStartTag; }
    virtual tagStatus endTag() { return IFRAMEEndTag; }

    virtual void parseAttribute(AttrImpl *attr);
    virtual void attach(KHTMLView *w);
};


}; //namespace

#endif

