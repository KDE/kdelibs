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
// -------------------------------------------------------------------------

#include "dom_string.h"

#include "html_miscimpl.h"
#include "dom_node.h"
#include "khtmltags.h"
#include "khtmlattrs.h"
using namespace DOM;

#include <stdio.h>

HTMLBaseFontElementImpl::HTMLBaseFontElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLBaseFontElementImpl::~HTMLBaseFontElementImpl()
{
}

const DOMString HTMLBaseFontElementImpl::nodeName() const
{
    return "BASEFONT";
}

ushort HTMLBaseFontElementImpl::id() const
{
    return ID_BASEFONT;
}

// -------------------------------------------------------------------------

HTMLCollectionImpl::HTMLCollectionImpl(NodeImpl *_base, int _type)
{
    base = _base;
    base->ref();
    type = _type;
}

HTMLCollectionImpl::~HTMLCollectionImpl()
{
    base->deref();
}

unsigned long HTMLCollectionImpl::calcLength(NodeImpl *current) const
{
    unsigned long len = 0;
    while(current)
    {
	if(!current->isTextNode())
	{
	    bool deep = true;
	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(current);
	    switch(type)
	    {
	    case DOC_IMAGES:
		if(e->id() == ID_IMG)
		    len++;
		break;
	    case DOC_FORMS:
		if(e->id() == ID_FORM)
		    len++;
		break;
	    case TABLE_TBODIES:
		if(e->id() == ID_TBODY)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TR_CELLS:
		if(e->id() == ID_TD)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TABLE_ROWS:
	    case TSECTION_ROWS:
		if(e->id() == ID_TR || e->id() == ID_TH)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case SELECT_OPTIONS:
		if(e->id() == ID_OPTION)
		    len++;
		break;
	    case MAP_AREAS:
		if(e->id() == ID_AREA)
		    len++;
		break;
	    case FORM_ELEMENTS:
		switch(e->id())
		{
		case ID_INPUT:
		case ID_BUTTON:
		case ID_SELECT:
		case ID_TEXTAREA:
		case ID_ISINDEX:
		    //case ID_LABEL: // ### does it really belong here???
		    len++;
		    break;
		default:
		    break;
		}
		break;
	    case DOC_APPLETS:   // all OBJECT and APPLET elements
		if(e->id() == ID_OBJECT || e->id() == ID_APPLET)
		    len++;
		break;
	    case DOC_LINKS:     // all A _and_ AREA elements with a value for href
		if(e->id() == ID_A || e->id() == ID_AREA)
		    if(e->getAttribute(ATTR_HREF) != 0)
			len++;
		break;
	    case DOC_ANCHORS:      // all A elements with a value for name
		if(e->id() == ID_A)
		    if(e->getAttribute(ATTR_NAME) != 0)
			len++;
		break;
	    default:
		printf("Error in HTMLCollection, wrong tagId!\n");
	    }
	    if(deep && current->firstChild())
		len += calcLength(current->firstChild());
	}
	current = current->nextSibling();
    }
    return len;
}

// since the collections are to be "live", we have to do the
// calculation every time...
unsigned long HTMLCollectionImpl::length() const
{
    return calcLength(base->firstChild());
}

NodeImpl *HTMLCollectionImpl::getItem(NodeImpl *current, int index, int &len)
{
    while(current)
    {
	if(!current->isTextNode())
	{
	    bool deep = true;
	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(current);
	    switch(type)
	    {
	    case DOC_IMAGES:
		if(e->id() == ID_IMG)
		    len++;
		break;
	    case DOC_FORMS:
		if(e->id() == ID_FORM)
		    len++;
		break;
	    case TABLE_TBODIES:
		if(e->id() == ID_TBODY)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TR_CELLS:
		if(e->id() == ID_TD)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TABLE_ROWS:
	    case TSECTION_ROWS:
		if(e->id() == ID_TR || e->id() == ID_TH)
		    len++;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case SELECT_OPTIONS:
		if(e->id() == ID_OPTION)
		    len++;
		break;
	    case MAP_AREAS:
		if(e->id() == ID_AREA)
		    len++;
		break;
	    case FORM_ELEMENTS:
		switch(e->id())
		{
		case ID_INPUT:
		case ID_BUTTON:
		case ID_SELECT:
		case ID_TEXTAREA:
		case ID_ISINDEX:
		    //case ID_LABEL: // ### does it really belong here???
		    len++;
		    break;
		default:
		    break;
		}
		break;
	    case DOC_APPLETS:   // all OBJECT and APPLET elements
		if(e->id() == ID_OBJECT || e->id() == ID_APPLET)
		    len++;
		break;
	    case DOC_LINKS:     // all A _and_ AREA elements with a value for href
		if(e->id() == ID_A || e->id() == ID_AREA)
		    if(e->getAttribute(ATTR_HREF) != 0)
			len++;
		break;
	    case DOC_ANCHORS:      // all A elements with a value for name
		if(e->id() == ID_A)
		    if(e->getAttribute(ATTR_NAME) != 0)
			len++;
		break;
	    default:
		printf("Error in HTMLCollection, wrong tagId!\n");
	    }
	    if(len == index) return current;
	    NodeImpl *retval;
	    if(deep && current->firstChild())
		retval = getItem(current->firstChild(), index, len);
	    if(retval) return retval;
	}
	current = current->nextSibling();
    }
    return 0;
}

NodeImpl *HTMLCollectionImpl::item( unsigned long index )
{
    int pos = 0;
    return getItem(base->firstChild(), index, pos);
}

NodeImpl *HTMLCollectionImpl::getNamedItem( NodeImpl *current, int attr_id,
					    const DOMString &name )
{


    while(current)
    {
	if(!current->isTextNode())
	{
	    bool deep = true;
	    bool check = false;
	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(current);
	    switch(type)
	    {
	    case DOC_IMAGES:
		if(e->id() == ID_IMG)
		    check = true;
		break;
	    case DOC_FORMS:
		if(e->id() == ID_FORM)
		    check = true;
		break;
	    case TABLE_TBODIES:
		if(e->id() == ID_TBODY)
		    check = true;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TR_CELLS:
		if(e->id() == ID_TD)
		    check = true;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case TABLE_ROWS:
	    case TSECTION_ROWS:
		if(e->id() == ID_TR || e->id() == ID_TH)
		    check = true;
		else if(e->id() == ID_TABLE)
		    deep = false;
		break;
	    case SELECT_OPTIONS:
		if(e->id() == ID_OPTION)
		    check = true;
		break;
	    case MAP_AREAS:
		if(e->id() == ID_AREA)
		    check = true;
		break;
	    case FORM_ELEMENTS:
		switch(e->id())
		{
		case ID_INPUT:
		case ID_BUTTON:
		case ID_SELECT:
		case ID_TEXTAREA:
		case ID_ISINDEX:
		    //case ID_LABEL: // ### does it really belong here???
		    check = true;
		    break;
		default:
		    break;
		}
		break;
	    case DOC_APPLETS:   // all OBJECT and APPLET elements
		if(e->id() == ID_OBJECT || e->id() == ID_APPLET)
		    check = true;
		break;
	    case DOC_LINKS:     // all A _and_ AREA elements with a value for href
		if(e->id() == ID_A || e->id() == ID_AREA)
		    if(e->getAttribute(ATTR_HREF) != 0)
			check = true;
		break;
	    case DOC_ANCHORS:      // all A elements with a value for name
		if(e->id() == ID_A)
		    if(e->getAttribute(ATTR_NAME) != 0)
			check = true;
		break;
	    default:
		printf("Error in HTMLCollection, wrong tagId!\n");
	    }
	    if(check && e->getAttribute(attr_id) == name)
	    {
		printf("found node: %p %p %d\n", e, current, e->id());
		return current;
	    }
	    NodeImpl *retval = 0;
	    if(deep && current->firstChild())
		retval = getNamedItem(current->firstChild(), attr_id, name);
	    if(retval) 
	    {	
		printf("got a return value %p\n", retval);
		return retval;
	    }
	}
	current = current->nextSibling();
    }
    return 0;
}

NodeImpl *HTMLCollectionImpl::namedItem( const DOMString &name )
{
    NodeImpl *n;
    n = getNamedItem(base->firstChild(), ATTR_ID, name);
    if(n) return n;
    return getNamedItem(base->firstChild(), ATTR_NAME, name);
}


