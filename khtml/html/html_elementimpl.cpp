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
//#define DEBUG_LAYOUT
//#define PAR_DEBUG
//#define EVENT_DEBUG

#include "dom_string.h"

#include <qpainter.h>
#include <qfontmetrics.h>
#include <qstack.h>
#include <qlist.h>

#include "html_elementimpl.h"
#include "html_inlineimpl.h"
#include "html_blockimpl.h"
#include "html_imageimpl.h"
#include "html_documentimpl.h"
#include "dom_node.h"
#include "dom_textimpl.h"
#include "dom_stringimpl.h"
#include "dom_exception.h"

#include "htmlhashes.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include "rendering/render_style.h"
#include "rendering/render_object.h"
#include "css/css_valueimpl.h"
#include "css/cssproperties.h"

#include <stdio.h>
#include <assert.h>

template class QList<DOM::NodeImpl>;

using namespace DOM;
using namespace khtml;

static inline int MAX(int a, int b)
{
    return a > b ? a : b;
}

static inline int MIN(int a, int b)
{
    return a < b ? a : b;
}

HTMLElementImpl::HTMLElementImpl(DocumentImpl *doc) : ElementImpl(doc)
{
    m_styleDecls = 0;
}

HTMLElementImpl::~HTMLElementImpl()
{
    delete m_styleDecls;
    //printf("Element destructor: this=%s\n", nodeName().string().ascii());
}

bool HTMLElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url,
                                  NodeImpl *&innerNode, long &offset)
{
#ifdef EVENT_DEBUG
    printf("%s::mouseEvent\n", nodeName().string().ascii());
#endif
    bool inside = false;

    if(!m_render) return false;

    if(!m_render->isInline() || m_render->isFloating())
    {
	_tx += m_render->xPos();
	_ty += m_render->yPos();

	inside = true;
	innerNode = this;
        if( (_y < _ty ) || (_y > _ty + m_render->height() ) ||
	    (_x < _tx ) || (_x > _tx + m_render->width() ) )
        {
            inside = false;
	    //printf("not inside the block element!\n");
        }


    }
    else if(m_render->isInline() && m_render->parent()->isAnonymousBox())
    {
	//printf("parent is anonymous!\n");
	// we need to add the offset of the anonymous box
	_tx += m_render->parent()->xPos();
	_ty += m_render->parent()->yPos();
    }

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, url, innerNode, offset))
	{
	    inside = true;
	    break;
	}
	child = child->nextSibling();
    }

#ifdef EVENT_DEBUG
    if(inside) printf("    --> inside\n");
#endif
    // dynamic HTML...
    if(inside || mouseInside()) mouseEventHandler(button, type, inside);

    return inside;
}

void HTMLElementImpl::mouseEventHandler( int /*button*/, MouseEventType type, bool inside )
{
    if(!hasEvents()) return;

    KHTMLView *htmlwidget = (KHTMLView *) static_cast<HTMLDocumentImpl *>(document)->HTMLWidget();
    if(!htmlwidget) return;

    int id;
    bool click = false;

    switch(type)
    {
    case MousePress:
	id = ATTR_ONMOUSEDOWN;
	setPressed();
	break;
    case MouseRelease:
	id = ATTR_ONMOUSEUP;
	if(pressed()) click = true;
	setPressed(false);
	break;
    case MouseClick:
	// handled differently at the moment
	break;
    case MouseDblClick:
	id = ATTR_ONDBLCLICK;
	break;
    case MouseMove:
	id = ATTR_ONMOUSEMOVE;
	break;
    default:
	break;
    }

    DOMString script = getAttribute(id);
    if(script.length())
    {
	printf("emit executeScript( %s )\n", script.string().ascii());
	htmlwidget->part()->executeScript( script.string() );
    }

    if(click)
    {
	script = getAttribute(ATTR_ONCLICK);
	if(script.length())
	{
	    htmlwidget->part()->executeScript( script.string() );
	    printf("emit executeScript( %s )\n", script.string().ascii());
	}
    }

    if(inside != mouseInside())
    {
	// onmouseover and onmouseout
	int id = ATTR_ONMOUSEOVER;
	if(!inside)
	    id = ATTR_ONMOUSEOUT;
	script = getAttribute(ATTR_ONCLICK);
	if(script.length())
	{
	    htmlwidget->part()->executeScript( script.string() );
	    printf("emit executeScript( %s )\n", script.string().ascii());
	}
    }
    setMouseInside(inside);
}

void HTMLElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
// the core attributes...
    case ATTR_ID:
	// unique id
	setHasID();
	break;
    case ATTR_CLASS:
	// class
	setHasClass();
	break;
    case ATTR_STYLE:
	// stylesheet info
	setHasStyle();
	addCSSProperty(attr->value());
	break;
    case ATTR_TITLE:
	// additional title for the element, may be displayed as tooltip
	setHasTooltip();
	break;
// i18n attributes
    case ATTR_LANG:
	// language info
	break;
    case ATTR_DIR:
	// BiDi info
	break;
// standard events
    case ATTR_ONCLICK:
    case ATTR_ONDBLCLICK:
    case ATTR_ONMOUSEDOWN:
    case ATTR_ONMOUSEMOVE:
    case ATTR_ONMOUSEOUT:
    case ATTR_ONMOUSEOVER:
    case ATTR_ONMOUSEUP:
    case ATTR_ONKEYDOWN:
    case ATTR_ONKEYPRESS:
    case ATTR_ONKEYUP:
	setHasEvents();
	break;
// other misc attributes
    default:
	break;
    }
}

void HTMLElementImpl::addCSSProperty(int id, const DOMString &value, bool important)
{
    if(!m_styleDecls) m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->setProperty(id, value, important);
}

void HTMLElementImpl::addCSSLength(int id, const DOMString &value, bool important)
{
    if(!m_styleDecls) m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->setLengthProperty(id, value, important);
}

void HTMLElementImpl::addCSSProperty(const DOMString &property)
{
    if(!m_styleDecls) m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->setProperty(property);
}

// -------------------------------------------------------------------------
HTMLGenericElementImpl::HTMLGenericElementImpl(DocumentImpl *doc, ushort i)
    : HTMLElementImpl(doc)
{
    _id = i;
}

HTMLGenericElementImpl::~HTMLGenericElementImpl()
{
}

const DOMString HTMLGenericElementImpl::nodeName() const
{
    return getTagName(_id);
}



