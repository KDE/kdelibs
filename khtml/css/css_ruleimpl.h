/*
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002 Apple Computer, Inc.
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
 */
#ifndef _CSS_css_ruleimpl_h_
#define _CSS_css_ruleimpl_h_

#include "dom/dom_string.h"
#include "dom/css_rule.h"
#include "css/css_base.h"
#include "misc/loader_client.h"
#include "misc/shared.h"

namespace khtml {
    class CachedCSSStyleSheet;
}

namespace DOM {

class CSSRule;
class CSSStyleSheet;
class CSSStyleSheetImpl;
class CSSStyleDeclarationImpl;
class MediaListImpl;

class CSSRuleImpl : public StyleBaseImpl
{
public:
    CSSRuleImpl(StyleBaseImpl *parent)
        : StyleBaseImpl(parent), m_type(CSSRule::UNKNOWN_RULE) {}

    virtual bool isRule() const { return true; }
    unsigned short type() const { return m_type; }

    CSSStyleSheetImpl *parentStyleSheet() const;
    CSSRuleImpl *parentRule() const;

    DOM::DOMString cssText() const;
    void setCssText(DOM::DOMString str);
    virtual void init() {}

protected:
    CSSRule::RuleType m_type;
};


class CSSCharsetRuleImpl : public CSSRuleImpl
{
public:
    CSSCharsetRuleImpl(StyleBaseImpl *parent)
        : CSSRuleImpl(parent) { m_type = CSSRule::CHARSET_RULE; }

    virtual bool isCharsetRule() const { return true; }

    DOMString encoding() const { return m_encoding; }
    void setEncoding(DOMString _encoding) { m_encoding = _encoding; }

protected:
    DOMString m_encoding;
};


class CSSFontFaceRuleImpl : public CSSRuleImpl
{
public:
    CSSFontFaceRuleImpl(StyleBaseImpl *parent);

    virtual ~CSSFontFaceRuleImpl();

    CSSStyleDeclarationImpl *style() const { return m_style; }

    virtual bool isFontFaceRule() const { return true; }

protected:
    CSSStyleDeclarationImpl *m_style;
};


class CSSImportRuleImpl : public khtml::CachedObjectClient, public CSSRuleImpl
{
public:
    CSSImportRuleImpl( StyleBaseImpl *parent, const DOM::DOMString &href,
                       const DOM::DOMString &media );
    CSSImportRuleImpl( StyleBaseImpl *parent, const DOM::DOMString &href,
                       MediaListImpl *media );

    virtual ~CSSImportRuleImpl();

    DOM::DOMString href() const { return m_strHref; }
    MediaListImpl *media() const { return m_lstMedia; }
    CSSStyleSheetImpl *styleSheet() const { return m_styleSheet; }

    virtual bool isImportRule() const { return true; }

    // from CachedObjectClient
    virtual void setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet);
    virtual void error(int err, const QString &text);

    bool isLoading();
    virtual void init();

protected:
    DOMString m_strHref;
    MediaListImpl *m_lstMedia;
    CSSStyleSheetImpl *m_styleSheet;
    khtml::CachedCSSStyleSheet *m_cachedSheet;
    bool m_loading;
    bool m_done;
};

class MediaList;

class CSSRuleListImpl : public khtml::Shared<CSSRuleListImpl>
{
public:
    CSSRuleListImpl() {}

    ~CSSRuleListImpl();

    unsigned long length() const { return m_lstCSSRules.count(); }
    CSSRuleImpl *item ( unsigned long index ) { return m_lstCSSRules.at( index ); }


    /* not part of the DOM */
    unsigned long insertRule ( CSSRuleImpl *rule, unsigned long index );
    void deleteRule ( unsigned long index );

    void append( CSSRuleImpl *rule ) { m_lstCSSRules.append( rule ); }
protected:
    QPtrList<CSSRuleImpl> m_lstCSSRules;
};

class CSSMediaRuleImpl : public CSSRuleImpl
{
public:
    CSSMediaRuleImpl( StyleBaseImpl *parent );
    CSSMediaRuleImpl( StyleBaseImpl *parent, const DOM::DOMString &media );
    CSSMediaRuleImpl( StyleBaseImpl *parent, MediaListImpl *mediaList, CSSRuleListImpl *ruleList );

    virtual ~CSSMediaRuleImpl();

    MediaListImpl *media() const { return m_lstMedia; }
    CSSRuleListImpl *cssRules() { return m_lstCSSRules; }

    unsigned long insertRule ( const DOM::DOMString &rule, unsigned long index );
    void deleteRule ( unsigned long index ) { m_lstCSSRules->deleteRule( index ); }

    virtual bool isMediaRule() const { return true; }

    /* Not part of the DOM */
    unsigned long append( CSSRuleImpl *rule );
protected:
    MediaListImpl *m_lstMedia;
    CSSRuleListImpl *m_lstCSSRules;
};


class CSSPageRuleImpl : public CSSRuleImpl
{
public:
    CSSPageRuleImpl(StyleBaseImpl *parent);

    virtual ~CSSPageRuleImpl();

    CSSStyleDeclarationImpl *style() const { return m_style; }

    virtual bool isPageRule() const { return true; }

    DOM::DOMString selectorText() const;
    void setSelectorText(DOM::DOMString str);

protected:
    CSSStyleDeclarationImpl *m_style;
};


class CSSStyleRuleImpl : public CSSRuleImpl
{
public:
    CSSStyleRuleImpl(StyleBaseImpl *parent);

    virtual ~CSSStyleRuleImpl();

    CSSStyleDeclarationImpl *style() const { return m_style; }

    virtual bool isStyleRule() const { return true; }

    DOM::DOMString selectorText() const;
    void setSelectorText(DOM::DOMString str);

    virtual bool parseString( const DOMString &string, bool = false );

    void setSelector( QPtrList<CSSSelector> *selector) { m_selector = selector; }
    void setDeclaration( CSSStyleDeclarationImpl *style);

    QPtrList<CSSSelector> *selector() { return m_selector; }
    CSSStyleDeclarationImpl *declaration() { return m_style; }

    void setNonCSSHints();

protected:
    CSSStyleDeclarationImpl *m_style;
    QPtrList<CSSSelector> *m_selector;
};


class CSSUnknownRuleImpl : public CSSRuleImpl
{
public:
    CSSUnknownRuleImpl(StyleBaseImpl *parent) : CSSRuleImpl(parent) {}

    virtual bool isUnknownRule() const { return true; }
};


} // namespace

#endif
