/**
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef _CSS_cssstyleselector_h_
#define _CSS_cssstyleselector_h_

#include <qlist.h>

#include "rendering/render_style.h"

class KHTMLSettings;

namespace DOM {
    class ElementImpl;
    class DocumentImpl;
    class HTMLDocumentImpl;
    class StyleSheetImpl;
    class CSSStyleRuleImpl;
    class CSSStyleSheetImpl;
    class CSSSelector;
    class CSSStyleDeclarationImpl;
    class CSSProperty;
}

namespace khtml
{
    class CSSStyleSelectorList;
    class CSSOrderedRule;
    class RenderStyle;

    // independent of classes. Applies on styleDeclaration to the RenderStyle style
    void applyRule(khtml::RenderStyle *style, DOM::CSSProperty *prop,
		   DOM::ElementImpl *e);

    /**
     * this class selects a RenderStyle for a given Element based on the
     * collection of styleshets it contains. This is just a vrtual base class
     * for specific implementations of the Selector. At the moment only CSSStyleSelector
     * exists, but someone may wish to implement XSL...
     */
    class StyleSelector
    {
    public:
	StyleSelector() {};
	virtual ~StyleSelector() {};
	
	virtual RenderStyle *styleForElement(DOM::ElementImpl *e) = 0;
    };


    /**
     * the StyleSelector implementation for CSS.
     */
    class CSSStyleSelector : public StyleSelector
    {
    public:
	/**
	 * creates a new StyleSelector for a XML Document.
	 * goes through all StyleSheets defined in the document and
	 * creates a list of rules it needs to apply to objects
	 *
	 * Basically the same as the constructor for a HTMLDocument,
	 * but the defaultStyle (which is html only) will not be used.
	 */
	CSSStyleSelector(DOM::DocumentImpl *doc);
	/**
	 * does the same for an HTML document. Initializes the default style
	 * for html.
	 */
	CSSStyleSelector(DOM::HTMLDocumentImpl *doc);
	/**
	 * same as above but for a single stylesheet.
	 */
	CSSStyleSelector(DOM::StyleSheetImpl *sheet);

	virtual ~CSSStyleSelector();
	
	void addSheet(DOM::StyleSheetImpl *sheet);
	
	static void loadDefaultStyle(const KHTMLSettings *s = 0);
	static void setUserStyle(DOM::StyleSheetImpl *sheet);
	static void clear();
	
	virtual RenderStyle *styleForElement(DOM::ElementImpl *e);
	
    protected:
	static DOM::CSSStyleSheetImpl *defaultSheet;
	static CSSStyleSelectorList *defaultStyle;
	static DOM::CSSStyleSheetImpl *userSheet;
	static CSSStyleSelectorList *userStyle;
	CSSStyleSelectorList *authorStyle;
    };

    /*
     * List of properties that get applied to the Element. We need to collect them first
     * and then apply them one by one, because we have to change the apply order.
     * Some properties depend on other one already being applied (for example all properties spezifying
     * some length need to have already the correct font size. Same applies to color
     *
     * While sorting them, we have to take care not to mix up the original order.
     */
    class CSSOrderedProperty
    {
    public:
	CSSOrderedProperty(DOM::CSSProperty *_prop, int _priority, RenderStyle::PseudoId _pid)
	{
	    prop = _prop;
	    priority = _priority;
            pseudoId = _pid;
	}
	
	DOM::CSSProperty *prop;
	int priority;
        RenderStyle::PseudoId pseudoId;        
    };

    /*
     * This is the list we will collect all properties we need to apply in.
     * It will get sorted once before applying.
     */
    class CSSOrderedPropertyList : public QList<CSSOrderedProperty>
    {
    public:
	CSSOrderedPropertyList();
	
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
	void append(DOM::CSSStyleDeclarationImpl *decl, int offset, int important);
	void append(DOM::CSSProperty *prop, int priority);
    };

    class CSSOrderedRule
    {
    public:
	CSSOrderedRule(DOM::CSSStyleRuleImpl *r, DOM::CSSSelector *s, int _index);
	~CSSOrderedRule();

	/* checks if the complete selector (which can be build up from a few CSSSelector's
	    with given relationships matches the given Element */
	bool checkSelector(DOM::ElementImpl *e);

	/* checks if the selector matches the given Element */
	bool checkOneSelector(DOM::CSSSelector *selector, DOM::ElementImpl *e);
	
	DOM::CSSSelector *selector;
	DOM::CSSStyleRuleImpl *rule;
	int index;
    };

    class CSSStyleSelectorList : public QList<CSSOrderedRule>
    {
    public:
	CSSStyleSelectorList();
	virtual ~CSSStyleSelectorList();
	
	virtual int compareItems(QCollection::Item i1, QCollection::Item i2);

	void append(DOM::StyleSheetImpl *sheet);
	void append(DOM::CSSStyleRuleImpl *rule);

	/**
	 * collects the rules that would apply to the style object. The important variable adds to
	 * the priority of the CSSOrderedProperty appended.
	 * This is needed to get the sorting order correct.
	 * offset is used to get the ordering between default user and author style sheets right
	 */
	void collect(khtml::CSSOrderedPropertyList *propsToApply, DOM::ElementImpl *e,
		     int offset = 0x0, int important = 0x0 );

    };

};
#endif
