/*
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

#include "xml/dom_docimpl.h"
#include "misc/loader_client.h"

#include <qmap.h>

class KHTMLView;
class QString;

namespace DOM {

    class HTMLCollection;
    class NodeList;
    class Element;
    class HTMLElement;
    class HTMLElementImpl;
    class DOMString;
    class CSSStyleSheetImpl;
    class HTMLMapElementImpl;

class HTMLDocumentImpl : public DOM::DocumentImpl, public khtml::CachedObjectClient
{
    Q_OBJECT
public:
    HTMLDocumentImpl(DOMImplementationImpl *_implementation, KHTMLView *v = 0);
    ~HTMLDocumentImpl();

    virtual bool isHTMLDocument() const { return true; }

    DOMString referrer() const;
    DOMString domain() const;
    void setDomain( const DOMString &newDomain ); // not part of the DOM
    DOMString lastModified() const;
    DOMString cookie() const;
    void setCookie( const DOMString &);

    HTMLElementImpl *body();
    void setBody(HTMLElementImpl *_body);

    virtual Tokenizer *createTokenizer();

    virtual bool childAllowed( NodeImpl *newChild );

    virtual ElementImpl *createElement ( const DOMString &tagName );

    HTMLMapElementImpl* getMap(const DOMString& url_);

    virtual void determineParseMode( const QString &str );
    virtual void close();

protected:
    HTMLElementImpl *bodyElement;
    HTMLElementImpl *htmlElement;
    friend class HTMLMapElementImpl;
    friend class HTMLImageElementImpl;
    QMap<QString,HTMLMapElementImpl*> mapMap;

protected slots:
    /**
     * Repaints, so that all links get the proper color
     */
    void slotHistoryChanged();
private:
    mutable DOMString m_domain;
};

} //namespace

#endif
