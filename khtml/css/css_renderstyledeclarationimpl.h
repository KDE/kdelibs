/**
 * css_renderstyleimpl.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#ifndef CSS_RENDERSTYLEIMPL_H
#define CSS_RENDERSTYLEIMPL_H

#include "css/css_valueimpl.h"
#include "dom/dom_string.h"

class RenderObject;

namespace DOM {
    class NodeImpl;
}

namespace khtml {

class RenderStyleDeclarationImpl : public DOM::CSSStyleDeclarationImpl
{
public:
    RenderStyleDeclarationImpl( DOM::NodeImpl *node );
    virtual ~RenderStyleDeclarationImpl();

    DOM::DOMString cssText() const;
    void setCssText( DOM::DOMString str );

    DOM::CSSValueImpl *getPropertyCSSValue( int propertyID ) const;
    DOM::DOMString getPropertyValue( int propertyID ) const;
    bool getPropertyPriority( int propertyID ) const;

    DOM::DOMString removeProperty( int propertyID, bool NonCSSHints = false );
    bool setProperty ( int propertyId, const DOM::DOMString &value, bool important = false,
                       bool nonCSSHint = false);
    void setProperty ( int propertyId, int value, bool important = false, bool nonCSSHint = false);
    void setLengthProperty(int id, const DOM::DOMString &value, bool important,
                           bool nonCSSHint = true, bool multiLength = false);

    void setProperty ( const DOM::DOMString &propertyString);
    DOM::DOMString item ( unsigned long index ) const;

protected:
    DOM::CSSProperty property( int id ) const;

protected:
    khtml::RenderObject *m_renderer;
};


}

#endif
