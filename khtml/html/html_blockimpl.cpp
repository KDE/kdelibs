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
// -------------------------------------------------------------------------
//#define DEBUG
#include "html_blockimpl.h"
#include "html_documentimpl.h"
#include "css/cssstyleselector.h"

#include "css/cssproperties.h"
#include "misc/htmlhashes.h"

using namespace khtml;
using namespace DOM;

HTMLBlockquoteElementImpl::HTMLBlockquoteElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLBlockquoteElementImpl::~HTMLBlockquoteElementImpl()
{
}

const DOMString HTMLBlockquoteElementImpl::nodeName() const
{
    return "BLOCKQUOTE";
}

ushort HTMLBlockquoteElementImpl::id() const
{
    return ID_BLOCKQUOTE;
}

void HTMLBlockquoteElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
    style()->setFlowAroundFloats(true);
}

// -------------------------------------------------------------------------

HTMLDivElementImpl::HTMLDivElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLDivElementImpl::~HTMLDivElementImpl()
{
}

const DOMString HTMLDivElementImpl::nodeName() const
{
    return "DIV";
}

ushort HTMLDivElementImpl::id() const
{
    return ID_DIV;
}

void HTMLDivElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ALIGN:
    {
	DOMString v = attr->value();
	if ( strcasecmp( attr->value(), "center" ) == 0 )
	    v = "konq-center";
	addCSSProperty(CSS_PROP_TEXT_ALIGN, v, false);
	break;
    }
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLHRElementImpl::HTMLHRElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLHRElementImpl::~HTMLHRElementImpl()
{
}

const DOMString HTMLHRElementImpl::nodeName() const
{
    return "HR";
}

ushort HTMLHRElementImpl::id() const
{
    return ID_HR;
}

void HTMLHRElementImpl::parseAttribute(AttrImpl *attr)
{
    switch( attr->attrId )
    {
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
    case ATTR_SIZE:
    {
        DOMString s = attr->value();
        if(s.toInt() < 2)
        {
            addCSSProperty(CSS_PROP_BORDER_WIDTH, "0", false);
            addCSSProperty(CSS_PROP_BORDER_TOP_WIDTH, "1", false);
        }
        else
            addCSSProperty(CSS_PROP_BORDER_WIDTH, s, false);
	break;
    }
    case ATTR_WIDTH:
	addCSSProperty(CSS_PROP_WIDTH, attr->value(), false);
	break;
    case ATTR_NOSHADE:
        addCSSProperty(CSS_PROP_BORDER_STYLE, "solid", false);
        // ###
        addCSSProperty(CSS_PROP_BORDER_COLOR, "black", false);
        break;
    case ATTR_COLOR:
        addCSSProperty(CSS_PROP_BORDER_COLOR, attr->value(), false);
        break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLHRElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
}

// -------------------------------------------------------------------------

HTMLHeadingElementImpl::HTMLHeadingElementImpl(DocumentImpl *doc, ushort _tagid) : HTMLElementImpl(doc)
{
    _id = _tagid;
}

HTMLHeadingElementImpl::~HTMLHeadingElementImpl()
{
}

const DOMString HTMLHeadingElementImpl::nodeName() const
{
    return getTagName(_id);
}


ushort HTMLHeadingElementImpl::id() const
{
    return _id;
}

void HTMLHeadingElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLParagraphElementImpl::HTMLParagraphElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLParagraphElementImpl::~HTMLParagraphElementImpl()
{
}

const DOMString HTMLParagraphElementImpl::nodeName() const
{
    return "P";
}

ushort HTMLParagraphElementImpl::id() const
{
    return ID_P;
}

void HTMLParagraphElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLParagraphElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
}

// -------------------------------------------------------------------------

HTMLPreElementImpl::HTMLPreElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLPreElementImpl::~HTMLPreElementImpl()
{
}

const DOMString HTMLPreElementImpl::nodeName() const
{
    return "PRE";
}

ushort HTMLPreElementImpl::id() const
{
    return ID_PRE;
}

long HTMLPreElementImpl::width() const
{
    // ###
    return 0;
}

void HTMLPreElementImpl::setWidth( long /*w*/ )
{
    // ###
}

