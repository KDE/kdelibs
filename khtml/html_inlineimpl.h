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
#ifndef HTML_INLINEIMPL_H
#define HTML_INLINEIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"
#include "html_imageimpl.h"

class CSSStyle;

namespace DOM {

class DOMString;

class HTMLAnchorElementImpl : public HTMLAreaElementImpl
{
public:
    HTMLAnchorElementImpl(DocumentImpl *doc);

    ~HTMLAnchorElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return AStartTag; }
    virtual tagStatus endTag() { return AEndTag; }
    long tabIndex() const;

    void setTabIndex( long );
    void blur (  );
    void focus (  );

    virtual void setStyle(CSSStyle *style);

    virtual void parseAttribute(Attribute *attr);
    virtual bool mouseEvent( int x, int y, int button, MouseEventType,
			     int _tx, int _ty, DOMString &url);

    void getAnchorPosition(int &xPos, int &yPos);
};

// -------------------------------------------------------------------------

class HTMLBRElementImpl : public HTMLElementImpl
{
public:
    HTMLBRElementImpl(DocumentImpl *doc);

    ~HTMLBRElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;
    virtual bool isRendered() { return true; }

    virtual tagStatus startTag() { return BRStartTag; }
    virtual tagStatus endTag() { return BREndTag; }
};

// -------------------------------------------------------------------------

class HTMLFontElementImpl : public HTMLElementImpl
{
public:
    HTMLFontElementImpl(DocumentImpl *doc);

    ~HTMLFontElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FONTStartTag; }
    virtual tagStatus endTag() { return FONTEndTag; }

    virtual void parseAttribute(Attribute *attr);
    virtual void setStyle(CSSStyle *currentStyle);

};

// -------------------------------------------------------------------------

class HTMLIFrameElementImpl : public HTMLElementImpl
{
public:
    HTMLIFrameElementImpl(DocumentImpl *doc);

    ~HTMLIFrameElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return IFRAMEStartTag; }
    virtual tagStatus endTag() { return IFRAMEEndTag; }
};

// -------------------------------------------------------------------------

class HTMLModElementImpl : public HTMLElementImpl
{
public:
    HTMLModElementImpl(DocumentImpl *doc, ushort tagid);

    ~HTMLModElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return INSStartTag; }
    virtual tagStatus endTag() { return INSEndTag; }

protected:
    ushort _id;
};

// -------------------------------------------------------------------------

class DOMString;

class HTMLQuoteElementImpl : public HTMLElementImpl
{
public:
    HTMLQuoteElementImpl(DocumentImpl *doc);

    ~HTMLQuoteElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BLOCKQUOTEStartTag; }
    virtual tagStatus endTag() { return BLOCKQUOTEEndTag; }
};

}; //namespace

#endif
