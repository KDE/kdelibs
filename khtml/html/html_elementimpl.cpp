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
#include "html_elementimpl.h"

#include "html_documentimpl.h"

#include "htmlhashes.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include "rendering/render_object.h"
#include "rendering/render_replaced.h"
#include "css/css_valueimpl.h"
#include "css_stylesheetimpl.h"
#include "css/cssproperties.h"

#include <stdlib.h> // using atoi

#include <kdebug.h>

using namespace DOM;
using namespace khtml;

HTMLElementImpl::HTMLElementImpl(DocumentImpl *doc) : ElementImpl(doc)
{
    m_styleDecls = 0;
    has_tabindex=false;

    DOMString indexstring = getAttribute(ATTR_TABINDEX);
    if (indexstring.length()) {
	has_tabindex=true;
	tabindex=atoi(indexstring.string().latin1());
    } else {
	has_tabindex=false;
	tabindex=0;
    }
}

HTMLElementImpl::~HTMLElementImpl()
{
//     if ( m_styleDecls ) {
// 	m_styleDecls->setParent( 0 );
// 	m_styleDecls->deref();
//     }
    delete m_styleDecls;
    //kdDebug( 6030 ) << "Element destructor: this=" << nodeName().string() << endl;
}

bool HTMLElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url,
                                  NodeImpl *&innerNode, long &offset)
{
#ifdef EVENT_DEBUG
    kdDebug( 6030 ) << nodeName().string() << "::mouseEvent" << endl;
#endif
    bool inside = false;

    if(!m_render) return false;

    RenderObject *p = m_render->parent();
    while( p && p->isAnonymousBox() ) {
	//kdDebug( 6030 ) << "parent is anonymous!" << endl;
	// we need to add the offset of the anonymous box
	_tx += p->xPos();
	_ty += p->yPos();
	p = p->parent();
    }


    if(!m_render->isInline() || !m_render->firstChild() || m_render->isFloating() ) {
	EPosition pos = m_render->style()->position();
	if ( pos == FIXED || pos == ABSOLUTE ) {
	    m_render->absolutePosition( _tx, _ty );
	    //kdDebug() << "positioned element at " << _tx << "/" << _ty << endl;
	} else {
	    _tx += m_render->xPos();
	    _ty += m_render->yPos();
	}

	inside = true;
	if( (_y < _ty ) || (_y >= _ty + m_render->height() ) ||
	    (_x < _tx ) || (_x >= _tx + m_render->width() ) )
	    inside = false;
	else
	    innerNode = this;
    }

    NodeImpl *child = firstChild();
    while(child != 0) {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, url, innerNode, offset))
	    inside = true;
	child = child->nextSibling();
    }

#ifdef EVENT_DEBUG
    if(inside) kdDebug( 6030 ) << "    --> inside" << endl;
#endif
    // dynamic HTML...
    if(inside || mouseInside()) mouseEventHandler(button, type, inside);

    return inside;
}

void HTMLElementImpl::mouseEventHandler( int /*button*/, MouseEventType type, bool inside )
{
    if(!hasEvents()) return;

    KHTMLView *view = (KHTMLView *) static_cast<HTMLDocumentImpl *>(document)->view();
    if(!view) return;

    int id = 0;
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
	id = ATTR_ONCLICK;
	click = true;
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

    if(id != ATTR_ONCLICK) {
	DOMString script = getAttribute(id);
	if(script.length())
	{
	    //kdDebug( 6030 ) << "emit executeScript( " << script.string() << " )" << endl;
	    view->part()->executeScript( Node( this ), script.string() );
	}
    }

    if(click)
    {
	DOMString script = getAttribute(ATTR_ONCLICK);
	if(script.length())
	{
	    //kdDebug( 6030 ) << "(click) emit executeScript( " << script.string() << " )" << endl;
	    view->part()->executeScript( Node( this ), script.string() );
	}
    }

    if(inside != mouseInside())
    {
	// onmouseover and onmouseout
	int id = ATTR_ONMOUSEOVER;
	if(!inside)
	    id = ATTR_ONMOUSEOUT;
	DOMString script = getAttribute(id);
	if(script.length())
	{
	    //kdDebug( 6030 ) << "emit executeScript( " << script.string() << " )" << endl;
	    if( id == ATTR_ONMOUSEOVER )
		view->part()->scheduleScript( Node( this ), script.string() );
	    else
		view->part()->executeScript( Node( this ), script.string() );
	}
    }
    setMouseInside(inside);
}

void HTMLElementImpl::parseAttribute(AttrImpl *attr)
{
  DOMString indexstring;
    switch( attr->attrId )
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
	// ### we need to remove old style info in case there was any!
	// ### the inline sheet ay contain more than 1 property!
	// stylesheet info
	setHasStyle();
	addCSSProperty(attr->value());
	break;
    case ATTR_TITLE:
	// additional title for the element, may be displayed as tooltip
	setHasTooltip();
	break;
    case ATTR_TABINDEX:
        indexstring=getAttribute(ATTR_TABINDEX);
        if (indexstring.length()) {
	  has_tabindex=true;
	  tabindex=atoi(indexstring.string().latin1());
	} else {
	  has_tabindex=false;
	  tabindex=0;
	}
	has_tabindex=true;
	break;
// i18n attributes
    case ATTR_LANG:
	break;
    case ATTR_DIR:
	addCSSProperty(CSS_PROP_DIRECTION, attr->value(), false );
	break;
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

static inline CSSStyleDeclarationImpl *createDecl( DocumentImpl *document )
{
    HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
    CSSStyleDeclarationImpl *m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->setParent(doc->elementSheet());
    m_styleDecls->ref();
    return m_styleDecls;
}

void HTMLElementImpl::addCSSProperty( const DOMString &property, const DOMString &value, bool important, bool nonCSSHint)
{
    if(!m_styleDecls) m_styleDecls = createDecl( document );
    m_styleDecls->setProperty(property, value, important, nonCSSHint);
}

void HTMLElementImpl::addCSSProperty(int id, const DOMString &value, bool important, bool nonCSSHint)
{
    if(!m_styleDecls) m_styleDecls = createDecl( document );
    m_styleDecls->setProperty(id, value, important, nonCSSHint);
}

void HTMLElementImpl::addCSSLength(int id, const DOMString &value, bool important, bool nonCSSHint)
{
    if(!m_styleDecls) m_styleDecls = createDecl( document );
    m_styleDecls->setLengthProperty(id, value, important, nonCSSHint);
}

void HTMLElementImpl::addCSSProperty(const DOMString &property)
{
    if(!m_styleDecls) m_styleDecls = createDecl( document );
    m_styleDecls->setProperty(property);
}

void HTMLElementImpl::removeCSSProperty(int id)
{
    if(!m_styleDecls)
	return;
    HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
    m_styleDecls->setParent(doc->elementSheet());
    m_styleDecls->removeProperty(id);
}

void HTMLElementImpl::removeCSSProperty( const DOMString &id )
{
    if(!m_styleDecls)
	return;
    HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
    m_styleDecls->setParent(doc->elementSheet());
    m_styleDecls->removeProperty(id);
}

DOMString HTMLElementImpl::getCSSProperty( const DOM::DOMString &prop )
{
    if(!m_styleDecls)
	return 0;
    return m_styleDecls->getPropertyValue( prop );
}

short HTMLElementImpl::tabIndex() const
{
  if (has_tabindex)
    return tabindex;
  else
    return -1;
}

void HTMLElementImpl::setTabIndex( short _tabindex )
{
  has_tabindex=true;
  tabindex=_tabindex;
}

bool HTMLElementImpl::isSelectable()
{
  switch(id())
    {
    case ID_A:
    case ID_INPUT:
    case ID_TEXTAREA:
    case ID_BUTTON:
    case ID_SELECT:
      if (!renderer())
      {
	kdDebug(6000)<<"isSelectable: no renderer for "<<getTagName(id()).string()<<"\n";
	  return false;
      }
      if (!renderer()->isReplaced())
	  return true;
      if (!static_cast<RenderWidget*>(renderer())->isWidget())
	  return true;
      return static_cast<RenderWidget*>(renderer())->m_widget->isEnabled();
    default:
      return false;
    };
}

// I don't like this way of implementing the method, but I didn't find any
// other way. Lars
void HTMLElementImpl::getAnchorPosition(int &xPos, int &yPos)
{
    if (!m_render)
    {
	kdDebug(6000) << "HTMLElementImpl::getAnchorPosition: no rendering object.\n";
	return;
    }
    if (m_render->containingBlock())
	m_render->containingBlock()->absolutePosition( xPos, yPos );
    else
	m_render->absolutePosition(xPos, yPos);

    RenderObject *o = m_render;
    // find the next text/image after the anchor, to get a position
    while(o) {
	if(o->firstChild())
	    o = o->firstChild();
	else if(o->nextSibling())
	    o = o->nextSibling();
	else {
	    RenderObject *next = 0;
	    while(!next) {
		o = o->parent();
		if(!o) return;
		next = o->nextSibling();
	    }
	    o = next;
	}
	if(o->isText() || o->isReplaced()) {
	    xPos += o->xPos();
	    yPos += o->yPos();
	    return;
	}
    }
}

void HTMLElementImpl::getAnchorBounds(int &xPos, int &yPos)
{
    if (!m_render)
    {
	kdDebug(6000) << "HTMLElementImpl::getAnchorBounds: no rendering object.\n";
	return;
    }

    RenderObject *myRenderer;
    if (m_render->containingBlock())
    {
	myRenderer=m_render->containingBlock();
    }
    else
    {
	myRenderer=m_render;
    }

    myRenderer->absolutePosition( xPos, yPos );

    RenderObject *o = m_render;
    // find the next text/image after the anchor, to get a position
    while(o) {
	if(o->firstChild())
	    o = o->firstChild();
	else if(o->nextSibling())
	    o = o->nextSibling();
	else {
	    RenderObject *next = 0;
	    while(!next) {
		o = o->parent();
		if(!o) return;
		next = o->nextSibling();
	    }
	    o = next;
	}
	if(o->isText() || o->isReplaced()) {
	    xPos += o->xPos()+o->width();
	    yPos += o->yPos()+o->height();
	    return;
	}
    }
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



