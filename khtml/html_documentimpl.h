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

#ifndef HTML_DOCUMENTIMPL_H
#define HTML_DOCUMENTIMPL_H

#include "dtd.h"
#include "dom_docimpl.h"

#include "html_baseimpl.h"

class KHTMLParser;
class HTMLTokenizer;
class KHTMLWidget;
class KHTMLCache;
class HTMLImageRequester;

namespace DOM {

class HTMLCollection;
class NodeList;
class Element;
class HTMLElement;

class DOMString;

class HTMLDocumentImpl : public DocumentImpl
{
public:
    HTMLDocumentImpl();
    HTMLDocumentImpl(KHTMLWidget *v, KHTMLCache *c);

    ~HTMLDocumentImpl();
    DOMString referrer() const;
    DOMString domain() const;
    DOMString URL() const;
    HTMLElementImpl *body();

    virtual bool isInline() { return false; }

    void open (  );
    void close (  );
    void write ( const DOMString &text );
    void write ( const QString &text );
    void writeln ( const DOMString &text );
    ElementImpl *getElementById ( const DOMString &elementId );
    NodeList getElementsByName ( const DOMString &elementName );

    // for KHTML
    virtual DOMString requestImage(HTMLImageRequester *, DOMString );

    // oeverrides NodeImpl
    virtual NodeImpl *addChild(NodeImpl *newChild);
    virtual void setAvailableWidth(int w = -1) { 
    	if(w != -1) width = w; 
	if(bodyElement) bodyElement->setAvailableWidth(w);
    }

    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    virtual void layout( bool deep = false);
    virtual bool mouseEvent( int x, int y, int button,
			     DOM::NodeImpl::MouseEventType,
			     int _tx, int _ty, DOMString &url);

    virtual void getAbsolutePosition(int &xPos, int &yPos)
	{ xPos = yPos = 0; };

    /** forces the redrawing of the object */
    virtual void print(NodeImpl *e, bool recursive=false);
    virtual void updateSize();

    const KHTMLWidget *HTMLWidget() { return view; }

    virtual void attach(KHTMLWidget *w);
    virtual void detach();

protected:
    void clear();

    KHTMLWidget *view;
    KHTMLParser *parser;
    HTMLTokenizer *tokenizer;
    KHTMLCache *cache;

    HTMLElementImpl *bodyElement;

    int width;
    int height;
};

}; //namespace

#endif
