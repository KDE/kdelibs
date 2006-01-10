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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef HTML_ELEMENTIMPL_H
#define HTML_ELEMENTIMPL_H

#include "xml/dom_elementimpl.h"

namespace DOM {

class DOMString;
class CSSStyleDeclarationImpl;
class HTMLFormElementImpl;
class DocumentFragment;

class HTMLElementImpl : public ElementImpl
{
public:
    HTMLElementImpl(DocumentPtr *doc);

    virtual ~HTMLElementImpl();

    virtual bool isHTMLElement() const { return true; }

    virtual bool isInline() const;

    virtual Id id() const = 0;
    virtual DOMString tagName() const;
    virtual DOMString localName() const;
    virtual DOMString namespaceURI() const;

    virtual void parseAttribute(AttributeImpl *token);

    void addCSSLength(int id, const DOMString &value, bool numOnly = false, bool multiLength = false);
    void addCSSProperty(int id, const DOMString &value);
    void addCSSProperty(int id, int value);
    void addHTMLColor( int id, const DOMString &c );
    void removeCSSProperty(int id);

    virtual void recalcStyle( StyleChange );

    DOMString innerHTML() const;
    DOMString innerText() const;
    DocumentFragment createContextualFragment( const DOMString &html );
    void setInnerHTML( const DOMString &html, int& exceptioncode );
    void setInnerText( const DOMString &text, int& exceptioncode );

    virtual DOMString toString() const;

protected:
    // for IMG, OBJECT and APPLET
    void addHTMLAlignment( DOMString alignment );
};

class HTMLGenericElementImpl : public HTMLElementImpl
{
public:
    HTMLGenericElementImpl(DocumentPtr *doc, ushort i);

    virtual ~HTMLGenericElementImpl();

    virtual Id id() const { return _id; };

protected:
    ushort _id;
};

} //namespace

#endif
