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

#include <kdebug.h>

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
    if(document->parseMode() != DocumentImpl::Strict )
	style()->setFlowAroundFloats(true);
}

// -------------------------------------------------------------------------

HTMLDivElementImpl::HTMLDivElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
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
	addCSSProperty(CSS_PROP_TEXT_ALIGN, v);
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
        if ( strcasecmp( attr->value(), "left") != 0) // _not_ equal
            addCSSProperty(CSS_PROP_MARGIN_LEFT, "auto");
        else
            addCSSProperty(CSS_PROP_MARGIN_LEFT, "1px");
        if( strcasecmp( attr->value(), "right") != 0)
            addCSSProperty(CSS_PROP_MARGIN_RIGHT, "auto");
        else
            addCSSProperty(CSS_PROP_MARGIN_RIGHT, "1px");
        break;
    case ATTR_SIZE:
    {
        DOMString s = attr->value();
        int _s = s.toInt();
        if(_s % 2)
        {
            QString n;
            n.sprintf("%dpx", _s / 2);
            addCSSProperty(CSS_PROP_BORDER_WIDTH, DOMString(n));
            n.sprintf("%dpx", (_s/ 2)+(_s ? 1 : 0));
            addCSSProperty(CSS_PROP_BORDER_TOP_WIDTH, DOMString(n));
        }
        else {
            QString w;
            w.setNum(_s/2);
            addCSSLength(CSS_PROP_BORDER_BOTTOM_WIDTH, DOMString(w));
            addCSSLength(CSS_PROP_BORDER_TOP_WIDTH, DOMString(w));
        }
	break;
    }
    case ATTR_WIDTH:
    {
        // cheap hack to cause linebreaks
        // khtmltests/html/strange_hr.html
        QString vstr = attr->value().string();
        bool ok;
        int v = vstr.toInt(&ok);
        if(ok && !v)
            addCSSLength(CSS_PROP_WIDTH, "1");
        else
            addCSSLength(CSS_PROP_WIDTH, attr->value());
	break;
    }
    case ATTR_NOSHADE:
        addCSSProperty(CSS_PROP_BORDER_STYLE, "solid");
        break;
    case ATTR_COLOR:
        addCSSProperty(CSS_PROP_BORDER_COLOR, attr->value());
        break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLHRElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
    if(document->parseMode() != DocumentImpl::Strict )
        style()->setFlowAroundFloats(true);
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
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value());
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
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value());
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

// ------------------------------------------------------------------------

HTMLLayerElementImpl::HTMLLayerElementImpl(DocumentImpl *doc)
    : HTMLDivElementImpl( doc )
{
    addCSSProperty(CSS_PROP_POSITION, "absolute" );
    fixed = false;
}

HTMLLayerElementImpl::~HTMLLayerElementImpl()
{
}
    
const DOMString HTMLLayerElementImpl::nodeName() const
{
    return "LAYER";
}

ushort HTMLLayerElementImpl::id() const
{
    return ID_LAYER;
}
    

void HTMLLayerElementImpl::parseAttribute(AttrImpl *attr)
{
    int cssprop;
    bool page = false;
    switch(attr->attrId) {
	case ATTR_PAGEX:
	    page = true;
	case ATTR_LEFT:
	    cssprop = CSS_PROP_LEFT;
	    break;
	case ATTR_PAGEY:
	    page = true;
	case ATTR_TOP:
	    cssprop = CSS_PROP_TOP;
	    break;
	case ATTR_WIDTH:
	    cssprop = CSS_PROP_WIDTH;
	    break;
	case ATTR_HEIGHT:
	    cssprop = CSS_PROP_HEIGHT;
	    break;
	case ATTR_Z_INDEX:
	    cssprop = CSS_PROP_Z_INDEX;
	    break;
	case ATTR_VISIBILITY:
	    cssprop = CSS_PROP_VISIBILITY;
	    break;
	default:
	    HTMLDivElementImpl::parseAttribute(attr);
	    return;
    }
    addCSSProperty(cssprop, attr->value());
    if ( !fixed && page ) {
	addCSSProperty(CSS_PROP_POSITION, "fixed");
	fixed = true;
    }
}

