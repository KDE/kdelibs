/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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

//#include "dtd.h"
#include "dom_docimpl.h"
#include "misc/loader_client.h"

//#include "html_baseimpl.h"

class KHTMLView;

namespace DOM {

    class StyleSheetImpl;
    class HTMLCollection;
    class NodeList;
    class Element;
    class HTMLElement;
    class HTMLElementImpl;
    class DOMString;
    class StyleSheetListImpl;
    class CSSStyleSheetImpl;
    
class HTMLDocumentImpl : public DocumentImpl, public khtml::CachedObjectClient
{
    Q_OBJECT
public:
    HTMLDocumentImpl();
    HTMLDocumentImpl(KHTMLView *v);

    ~HTMLDocumentImpl();

    virtual bool isHTMLDocument() const { return true; }

    DOMString referrer() const;
    DOMString domain() const;

    HTMLElementImpl *body();
    HTMLElementImpl *html();
    void setBody(const HTMLElement &_body);

    virtual Tokenizer *createTokenizer();
    NodeListImpl *getElementsByName ( const DOMString &elementName );

    virtual StyleSheetListImpl *styleSheets();

    // internal
    NodeImpl *findElement( int id );
    HTMLElementImpl *findSelectableElement( NodeImpl *start, bool forward = true);

    HTMLElementImpl *findLink(HTMLElementImpl *start, bool forward, int tabIndexHint=-1);

    int findHighestTabIndex();

    // oeverrides NodeImpl
    virtual bool mouseEvent( int x, int y, int button,
			     DOM::NodeImpl::MouseEventType,
			     int _tx, int _ty, DOMString &url,
                             NodeImpl *& innerNode, long &offset);

    virtual void detach();

    virtual bool childAllowed( NodeImpl *newChild );

    void setOnload( const QString &script ) { onloadScript = script; }
    void setOnunload( const QString &script ) { onUnloadScript = script; }

signals:
    void finishedParsing();

public slots:
    void slotFinishedParsing();

protected:
    HTMLElementImpl *bodyElement;
    HTMLElementImpl *htmlElement;

    QString onloadScript;
    QString onUnloadScript;

};

}; //namespace

#endif
