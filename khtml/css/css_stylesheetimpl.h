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
#ifndef _CSS_css_stylesheetimpl_h_
#define _CSS_css_stylesheetimpl_h_

#include <qlist.h>

#include <dom_string.h>

#include "cssparser.h"

namespace DOM {

class StyleSheet;
class CSSStyleSheet;
class MediaListImpl;
class CSSRuleImpl;
class CSSRuleList;
class CSSStyleRuleImpl;
class CSSValueImpl;
class NodeImpl;
    
class StyleSheetImpl : public StyleListImpl
{
public:
    StyleSheetImpl(DOM::NodeImpl *ownerNode);
    StyleSheetImpl(StyleSheetImpl *parentSheet);

    virtual ~StyleSheetImpl();

    virtual bool isStyleSheet() { return true; }
    virtual bool deleteMe();

    DOM::DOMString type() const;

    bool disabled() const;
    void setDisabled( bool );

    DOM::NodeImpl *ownerNode() const;
    StyleSheetImpl *parentStyleSheet() const;
    DOM::DOMString href() const;
    DOM::DOMString title() const;
    MediaListImpl *media() const;

protected:
    DOM::NodeImpl *m_parentNode;
    bool m_disabled;
    DOM::DOMString m_strType;
    DOM::DOMString m_strHref;
    DOM::DOMString m_strTitle;
    MediaListImpl *m_media;
};

class CSSStyleSheetImpl : public StyleSheetImpl
{
public:
    CSSStyleSheetImpl(DOM::NodeImpl *parentNode);
    CSSStyleSheetImpl(CSSStyleSheetImpl *parentSheet);

    virtual ~CSSStyleSheetImpl();

    virtual bool isCSSStyleSheet() { return true; }

    CSSRuleImpl *ownerRule() const;
    CSSRuleList cssRules();
    unsigned long insertRule ( const DOM::DOMString &rule, unsigned long index );
    void deleteRule ( unsigned long index );

    virtual bool parseString( const DOMString &string );
};

// ----------------------------------------------------------------------------

class StyleSheetListImpl : public StyleListImpl
{
public:
    StyleSheetListImpl(StyleSheetImpl *sheet);

    virtual ~StyleSheetListImpl();

    virtual bool isStyleSheetList() { return true; }

    unsigned long length() const;
    StyleSheetImpl *item ( unsigned long index );
};

// ----------------------------------------------------------------------------

class MediaListImpl : public StyleBaseImpl
{
public:
    MediaListImpl(CSSStyleSheetImpl *parentSheet);
    MediaListImpl(CSSRuleImpl *parentRule);

    virtual ~MediaListImpl();

    virtual bool isMediaList() { return true; }

    CSSStyleSheetImpl *parentStyleSheet() const;
    CSSRuleImpl *parentRule() const;
    unsigned long length() const;
    DOM::DOMString item ( unsigned long index );
    void del ( const DOM::DOMString &oldMedium );
    void append ( const DOM::DOMString &newMedium );

    DOM::DOMString cssText() const;
    void setCssText(DOM::DOMString str);

protected:
    QList<DOMString> m_lstMedia;
};


}; // namespace

#endif
