/*
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *               1999 Waldo Bastian (bastian@kde.org)
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
 * $Id$
 */
#ifndef _CSS_BASE_H
#define _CSS_BASE_H

#include "dom/dom_string.h"
#include "dom/dom_misc.h"
#include "xml/dom_nodeimpl.h"
#include "misc/shared.h"
#include <kdemacros.h>
#include <qdatetime.h>
#include <qptrlist.h>

namespace DOM {

    class StyleSheetImpl;
    class MediaList;

    class CSSSelector;
    class CSSProperty;
    class CSSValueImpl;
    class CSSPrimitiveValueImpl;
    class CSSStyleDeclarationImpl;
    class CSSRuleImpl;
    class CSSStyleRuleImpl;

    class DocumentImpl;

// this class represents a selector for a StyleRule
    class CSSSelector
    {
    public:
	CSSSelector()
	    : tagHistory(0), simpleSelector(0), attr(0), tag(0xffff), relation( Descendant ),
	      match( None ), nonCSSHint( false ), pseudoId( 0 ), _pseudoType(PseudoNotParsed)
        {}

	~CSSSelector() {
	    delete tagHistory;
            delete simpleSelector;
	}

	/**
	 * Print debug output for this selector
	 */
	void print();

	/**
	 * Re-create selector text from selector's data
	 */
	DOMString selectorText() const;

	// checks if the 2 selectors (including sub selectors) agree.
	bool operator == ( const CSSSelector &other ) const;

	// tag == -1 means apply to all elements (Selector = *)

	unsigned int specificity() const;

	/* how the attribute value has to match.... Default is Exact */
	enum Match
	{
	    None = 0,
	    Id,
	    Exact,
	    Set,
	    List,
	    Hyphen,
	    Pseudo,
	    Contain,   // css3: E[foo*="bar"]
	    Begin,     // css3: E[foo^="bar"]
	    End        // css3: E[foo$="bar"]
	};

	enum Relation
	{
	    Descendant = 0,
	    Child,
	    Sibling,
            Cousin,
            SubSelector
	};

	enum PseudoType
	{
	    PseudoNotParsed = 0,
	    PseudoOther,
	    PseudoEmpty,
	    PseudoFirstChild,
            PseudoLastChild,
            PseudoOnlyChild,
	    PseudoFirstLine,
	    PseudoFirstLetter,
	    PseudoLink,
	    PseudoVisited,
	    PseudoHover,
	    PseudoFocus,
	    PseudoActive,
            PseudoTarget,
	    PseudoBefore,
	    PseudoAfter,
            PseudoLang,
            PseudoNot,
            PseudoRoot,
            PseudoSelection
	};

	PseudoType pseudoType() const {
            if (_pseudoType == PseudoNotParsed)
                extractPseudoType();
            return _pseudoType;
        }

	mutable DOM::DOMString value;
	CSSSelector *tagHistory;
        CSSSelector* simpleSelector; // Used for :not.
        DOM::NodeImpl::Id attr;
        DOM::NodeImpl::Id tag;

	Relation relation     : 3;
	Match 	 match         : 4;
	bool	nonCSSHint : 1;
	unsigned int pseudoId : 3;
	mutable PseudoType _pseudoType : 5;

    private:
	void extractPseudoType() const;
    };

    // a style class which has a parent (almost all have)
    class StyleBaseImpl : public khtml::TreeShared<StyleBaseImpl>
    {
    public:
	StyleBaseImpl()  { m_parent = 0; hasInlinedDecl = false; strictParsing = true; multiLength = false; }
	StyleBaseImpl(StyleBaseImpl *p) {
	    m_parent = p; hasInlinedDecl = false;
	    strictParsing = (m_parent ? m_parent->useStrictParsing() : true);
	    multiLength = false;
	}

	virtual ~StyleBaseImpl() {}

	// returns the url of the style sheet this object belongs to
        // not const
	KURL baseURL();

	virtual bool isStyleSheet() const { return false; }
	virtual bool isCSSStyleSheet() const { return false; }
	virtual bool isStyleSheetList() const { return false; }
	virtual bool isMediaList() const { return false; }
	virtual bool isRuleList() const { return false; }
	virtual bool isRule() const { return false; }
	virtual bool isStyleRule() const { return false; }
	virtual bool isCharetRule() const { return false; }
	virtual bool isImportRule() const { return false; }
	virtual bool isMediaRule() const { return false; }
	virtual bool isFontFaceRule() const { return false; }
	virtual bool isPageRule() const { return false; }
	virtual bool isUnknownRule() const { return false; }
	virtual bool isStyleDeclaration() const { return false; }
	virtual bool isValue() const { return false; }
	virtual bool isPrimitiveValue() const { return false; }
	virtual bool isValueList() const { return false; }
	virtual bool isValueCustom() const { return false; }

	void setParent(StyleBaseImpl *parent) { m_parent = parent; }

	static void setParsedValue(int propId, const CSSValueImpl *parsedValue,
				   bool important, bool nonCSSHint, QPtrList<CSSProperty> *propList);

	virtual bool parseString(const DOMString &/*cssString*/, bool = false) { return false; }

	virtual void checkLoaded() const;

	void setStrictParsing( bool b ) { strictParsing = b; }
	bool useStrictParsing() const { return strictParsing; }

        // not const
	StyleSheetImpl* stylesheet();

    protected:
	bool hasInlinedDecl : 1;
	bool strictParsing : 1;
	bool multiLength : 1;
    };

    // a style class which has a list of children (StyleSheets for example)
    class StyleListImpl : public StyleBaseImpl
    {
    public:
	StyleListImpl() : StyleBaseImpl() { m_lstChildren = 0; }
	StyleListImpl(StyleBaseImpl *parent) : StyleBaseImpl(parent) { m_lstChildren = 0; }
	virtual ~StyleListImpl();

	unsigned long length() const { return m_lstChildren->count(); }
	StyleBaseImpl *item(unsigned long num) const { return m_lstChildren->at(num); }

	void append(StyleBaseImpl *item) { m_lstChildren->append(item); }

    protected:
	QPtrList<StyleBaseImpl> *m_lstChildren;
    };

    KDE_NO_EXPORT int getPropertyID(const char *tagStr, int len);

}

#endif
