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
#include "css_stylesheetimpl.h"

#include "css_stylesheet.h"
#include "css_rule.h"
#include "css_ruleimpl.h"
#include "css_valueimpl.h"
#include "css_parser.h"

#include "dom_string.h"
#include "dom_exception.h"
using namespace DOM;

// --------------------------------------------------------------------------------

StyleSheetImpl::StyleSheetImpl(StyleSheetImpl *parentSheet)
    : StyleListImpl(parentSheet)
{
    m_disabled = false;
    m_media = 0;
}


StyleSheetImpl::StyleSheetImpl(DOM::NodeImpl *parentNode)
    : StyleListImpl()
{
    m_parentNode = parentNode;
    m_disabled = false;
    m_media = 0;
}

StyleSheetImpl::~StyleSheetImpl()
{
    m_media->deref();
}

bool StyleSheetImpl::deleteMe()
{
    if(!m_parent && !m_parentNode && _ref <= 0) return true;
    return false;
}

DOMString StyleSheetImpl::type() const
{
    return m_strType;
}

bool StyleSheetImpl::disabled() const
{
    return m_disabled;
}

void StyleSheetImpl::setDisabled( bool disabled )
{
    m_disabled = disabled;
}

NodeImpl *StyleSheetImpl::ownerNode() const
{
    return m_parentNode;
}

StyleSheetImpl *StyleSheetImpl::parentStyleSheet() const
{
    if( !m_parent ) return 0;
    if( m_parent->isStyleSheet() ) return static_cast<StyleSheetImpl *>(m_parent);
    return 0;
}

DOMString StyleSheetImpl::href() const
{
    return m_strHref;
}

DOMString StyleSheetImpl::title() const
{
    return m_strTitle;
}

MediaListImpl *StyleSheetImpl::media() const
{
    return m_media;
}

// -----------------------------------------------------------------------


CSSStyleSheetImpl::CSSStyleSheetImpl(CSSStyleSheetImpl *parentSheet)
    : StyleSheetImpl(parentSheet)
{
    m_strType = "text/css";
    m_lstChildren = new QList<StyleBaseImpl>;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode)
    : StyleSheetImpl(parentNode)
{
    m_strType = "text/css";
    m_lstChildren = new QList<StyleBaseImpl>;
}

CSSStyleSheetImpl::~CSSStyleSheetImpl()
{
    // m_lstChildren is deleted in StyleListImpl
}

CSSRuleImpl *CSSStyleSheetImpl::ownerRule() const
{
    if( !m_parent ) return 0;
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

CSSRuleList CSSStyleSheetImpl::cssRules()
{
    return this;
}

unsigned long CSSStyleSheetImpl::insertRule( const DOMString &rule, unsigned long index )
{
    if(index > m_lstChildren->count())
	throw DOMException(DOMException::INDEX_SIZE_ERR);

    const QChar *curP = rule.unicode();
    const QChar *endP = rule.unicode()+rule.length();
    CSSRuleImpl *r = parseRule(curP, endP);

    if(!r) throw DOMException(DOMException::SYNTAX_ERR);

    // ###
    // HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the specified index e.g. if an
    //@import rule is inserted after a standard rule set or other at-rule.
    m_lstChildren->insert(index, r);
}

void CSSStyleSheetImpl::deleteRule( unsigned long index )
{
    StyleBaseImpl *b = m_lstChildren->take(index);
    if(!b) throw DOMException(DOMException::INDEX_SIZE_ERR);
    b->deref();
}

bool
CSSStyleSheetImpl::parseString(const DOMString &string)
{
    const QChar *curP = string.unicode();
    const QChar *endP = string.unicode()+string.length()-1;

    printf("parsing sheet, len=%d, last char=%d\n", string.length(), endP->unicode());

    while (curP && (curP < endP))
    {
	CSSRuleImpl *rule = parseRule(curP, endP);
	if(rule) m_lstChildren->append(rule);
    }
    return true;
}

// ---------------------------------------------------------------------------------------------

StyleSheetListImpl::StyleSheetListImpl(StyleSheetImpl *parentSheet)
    : StyleListImpl(parentSheet)
{
}

StyleSheetListImpl::~StyleSheetListImpl()
{
}

unsigned long StyleSheetListImpl::length() const
{
    //###
}

StyleSheetImpl *StyleSheetListImpl::item( unsigned long index )
{
    //###
}


// --------------------------------------------------------------------------------------------

MediaListImpl::MediaListImpl(CSSStyleSheetImpl *parentSheet)
    : StyleBaseImpl(parentSheet)
{
    m_lstMedia.setAutoDelete(true);
}

MediaListImpl::MediaListImpl(CSSRuleImpl *parentRule)
    : StyleBaseImpl(parentRule)
{
    m_lstMedia.setAutoDelete(true);
}

MediaListImpl::~MediaListImpl()
{
}

CSSStyleSheetImpl *MediaListImpl::parentStyleSheet() const
{
    if( m_parent->isCSSStyleSheet() ) return static_cast<CSSStyleSheetImpl *>(m_parent);
    return 0;
}

CSSRuleImpl *MediaListImpl::parentRule() const
{
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

unsigned long MediaListImpl::length() const
{
    return m_lstMedia.count();
}

DOMString MediaListImpl::item( unsigned long index )
{
    return *m_lstMedia.at(index);
}

void MediaListImpl::del( const DOMString &oldMedium )
{
    int i;
    for(i = 0; i < m_lstMedia.count(); i++)
	if( *(m_lstMedia.at(i)) == oldMedium )
	{
	    m_lstMedia.remove(i);
	    return;
	}
}	

void MediaListImpl::append( const DOMString &newMedium )
{
    DOMString *str = new DOMString(newMedium);
    m_lstMedia.append(str);
}

DOM::DOMString MediaListImpl::cssText() const
{
    // ###
}

void MediaListImpl::setCssText(DOM::DOMString str)
{
    // ###
}


