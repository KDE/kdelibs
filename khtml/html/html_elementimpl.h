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
#ifndef HTML_ELEMENTIMPL_H
#define HTML_ELEMENTIMPL_H

#include "dom_elementimpl.h"

namespace DOM {

class DOMString;
class CSSStyleDeclarationImpl;

class HTMLElementImpl : public ElementImpl
{
public:
    HTMLElementImpl(DocumentImpl *doc);

    virtual ~HTMLElementImpl();

    virtual bool isHTMLElement() const { return true; }

    // make it pure virtual...
    virtual ushort id() const = 0;

    virtual void mouseEventHandler( int button, MouseEventType type, bool inside );

    virtual void parseAttribute(AttrImpl *token);

    virtual DOM::CSSStyleDeclarationImpl *styleRules() { return m_styleDecls; }

    void addCSSProperty( const DOMString &property, const DOMString &value, bool important = false , bool nonCSSHint = true);
    void addCSSLength(int id, const DOMString &value, bool important = false, bool nonCSSHint = true);
    void addCSSProperty(int id, const DOMString &value, bool important = false, bool nonCSSHint = true);
    void addCSSProperty(const DOMString &property);
    DOMString getCSSProperty( const DOM::DOMString &prop );
    void removeCSSProperty(int id);
    void removeCSSProperty(const DOMString &id);

    void getAnchorPosition(int &xPos, int &yPos);
    void getAnchorBounds(int &xPos, int &yPos);

    virtual short tabIndex() const;
    virtual void setTabIndex( short );

    /**
     * returns true if this node may receive the keyboard focus,
     * false otherwise.
     */
    virtual bool isSelectable();

    virtual void blur(){};
    virtual void focus(){};

protected:
    DOM::CSSStyleDeclarationImpl *m_styleDecls;

private:
    bool has_tabindex;
    short tabindex;
};

class HTMLGenericElementImpl : public HTMLElementImpl
{
public:
    HTMLGenericElementImpl(DocumentImpl *doc, ushort i);

    virtual ~HTMLGenericElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const { return _id; };

protected:
    ushort _id;
};

}; //namespace

#endif
