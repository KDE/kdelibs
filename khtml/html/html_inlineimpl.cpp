/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *	     (C) 2000 Simon Hausmann <hausmann@kde.org>
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

#include <qfontinfo.h>
#include <qstack.h>
#include <qregexp.h>

#include <kdebug.h>

#include "khtmlview.h"
#include "khtml_part.h"

#include "dom_textimpl.h"
#include "dom_string.h"
#include "html_inline.h"
#include "html_inlineimpl.h"
using namespace DOM;

#include "htmlhashes.h"
#include "css/cssproperties.h"
#include "css/cssstyleselector.h"

#include "rendering/render_br.h"
#include "rendering/render_frames.h"

using namespace khtml;

HTMLAnchorElementImpl::HTMLAnchorElementImpl(DocumentImpl *doc)
: HTMLAreaElementImpl(doc)
{
}

HTMLAnchorElementImpl::~HTMLAnchorElementImpl()
{
}

const DOMString HTMLAnchorElementImpl::nodeName() const
{
    return "A";
}

ushort HTMLAnchorElementImpl::id() const
{
    return ID_A;
}

long HTMLAnchorElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLAnchorElementImpl::setTabIndex( long  )
{
}

void HTMLAnchorElementImpl::blur(  )
{
}

void HTMLAnchorElementImpl::focus(  )
{
}

bool HTMLAnchorElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &_url,
                                        NodeImpl *&innerNode, long &offset)
{
    bool inside = false;

    if(m_render->isInline() && m_render->parent()->isAnonymousBox())
    {
	//kdDebug(300) << "parent is anonymous!" << endl;
	// we need to add the offset of the anonymous box
	_tx += m_render->parent()->xPos();
	_ty += m_render->parent()->yPos();
    }

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, _url, innerNode, offset))
	{
	    inside = true;
	    break;
	}
	child = child->nextSibling();
    }
    //kdDebug(300) << "Anchor::mouseEvent inside=" << inside << endl;

    if(inside)
    {
	// set the url
	if(_url != 0) kdDebug(300) << "Error in Anchor::mouseEvent, nested url's!" << endl;
	if(target && href)
	{
	    DOMString s = DOMString("target://") + DOMString(target) + DOMString("/#") + DOMString(href);
	    _url = s;
	}
	else
	    _url = href;
    }
	
    // dynamic HTML...
    if(inside || mouseInside()) mouseEventHandler(button, type, inside);
	
    return inside;
}

void HTMLAnchorElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_HREF:
    default:
	HTMLAreaElementImpl::parseAttribute(attr);
    }
}

// I don't like this way of implementing the method, but I didn't find any
// other way. Lars
void HTMLAnchorElementImpl::getAnchorPosition(int &xPos, int &yPos)
{
    if(_parent)
    {
	_parent->renderer()->absolutePosition( xPos, yPos );
	// now we need to get the postion of this element. As it's
	// not positioned, we use the first child which is positioned
	NodeImpl *current = firstChild();
	QStack<NodeImpl> nodeStack;
	bool found = false;
	while(1)
	{
	    if(!current)
	    {
		if(nodeStack.isEmpty()) break;
		current = nodeStack.pop();
	    }
	    else
	    {
		RenderObject *o = current->renderer();
		if( o && (o->isReplaced() || o->isText() || !o->isInline()) ) {
		    found = true;
		    break;
		}
	    }	

	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
	if(found)
	{
	    RenderObject *o = current->renderer();
	    if(o)
	    {
		xPos += o->xPos();
		yPos += o->yPos();
	    }
	}
	else
	{
	  // we take the position of the parent.
	  return;
	}
    }
    else
	xPos = yPos = -1;
}
// -------------------------------------------------------------------------

HTMLBRElementImpl::HTMLBRElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLBRElementImpl::~HTMLBRElementImpl()
{
}

const DOMString HTMLBRElementImpl::nodeName() const
{
    return "BR";
}

ushort HTMLBRElementImpl::id() const
{
    return ID_BR;
}

void HTMLBRElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_CLEAR:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    addCSSProperty(CSS_PROP_CLEAR, "left", false);
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    addCSSProperty(CSS_PROP_CLEAR, "right", false);
	else if ( strcasecmp( attr->value(), "all" ) == 0 )
	    addCSSProperty(CSS_PROP_CLEAR, "both", false);
	break;
    default:
    	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLBRElementImpl::attach(KHTMLView *)
{
    //kdDebug(300) << "HTMLBRElementImpl::attach" << endl;
    m_style = document->styleSelector()->styleForElement(this);
    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	m_render = new RenderBR();
	m_render->setStyle(m_style);
	m_render->ref();
	r->addChild(m_render);	
    }
}

// -------------------------------------------------------------------------

HTMLFontElementImpl::HTMLFontElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLFontElementImpl::~HTMLFontElementImpl()
{
}

const DOMString HTMLFontElementImpl::nodeName() const
{
    return "FONT";
}

ushort HTMLFontElementImpl::id() const
{
    return ID_FONT;
}

void HTMLFontElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SIZE:
    	{
	DOMString s = attr->value();
	if(s != 0)
	{
            int num = s.toInt();
            if ( *s.unicode() == '+')
	    {
		// ### Won't work like that!!!! larger will only get applied once
	    	for (;num>0;num--)		
		    addCSSProperty(CSS_PROP_FONT_SIZE, "larger", false);
	    }
	    else if ( *s.unicode() == '-')
	    {
	    	for (;num<0;num++)		
		    addCSSProperty(CSS_PROP_FONT_SIZE, "smaller", false);
	    }        	
            else
	    {
        	switch (num)
		{
		    // size = 3 is the normal size according to html specs
		    case 1:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "x-small", false);
			break;
		    case 2:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "small", false);
		    	break;
		    case 3:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "medium", false);
		    	break;
		    case 4:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "large", false);
		    	break;
		    case 5:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "x-large", false);
		    	break;
		    case 6:
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "xx-large", false);
		    	break;
		    case 7:
			// ###
		    	addCSSProperty(CSS_PROP_FONT_SIZE, "xx-large", false);
		    	break;
		    default:		    	
		    	break;
		
		}
		
	    }
	}
	
	break;
    	}
    case ATTR_COLOR:
	addCSSProperty(CSS_PROP_COLOR, attr->value(), false);
	// HTML4 compatibility hack
	addCSSProperty(CSS_PROP_TEXT_DECORATION_COLOR, attr->value(), false);
	break;
    case ATTR_FACE:
	addCSSProperty(CSS_PROP_FONT_FAMILY, attr->value(), false);
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLIFrameElementImpl::HTMLIFrameElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLIFrameElementImpl::~HTMLIFrameElementImpl()
{
}

const DOMString HTMLIFrameElementImpl::nodeName() const
{
    return "IFRAME";
}

ushort HTMLIFrameElementImpl::id() const
{
    return ID_IFRAME;
}

void HTMLIFrameElementImpl::parseAttribute(Attribute *attr )
{
  switch (  attr->id )
  {
    case ATTR_SRC:
      url = attr->value();
      break;
    case ATTR_NAME:
      name = attr->value();
      break;
    case ATTR_WIDTH:
      addCSSLength( CSS_PROP_WIDTH, attr->value(), false );
      break;
    case ATTR_HEIGHT:
      addCSSLength( CSS_PROP_HEIGHT, attr->value(), false );
      break;
    // ### MORE ATTRIBUTES
    default:
      HTMLElementImpl::parseAttribute( attr );
  }
}

void HTMLIFrameElementImpl::attach(KHTMLView *w)
{
  m_style = document->styleSelector()->styleForElement( this );

  khtml::RenderObject *r = _parent->renderer();

  if ( !r )
    return;

  khtml::RenderPartObject *renderFrame = new khtml::RenderPartObject( w );
  m_render = renderFrame;
  m_render->setStyle(m_style);
  m_render->ref();
  r->addChild( m_render );

  // we need a unique name for every frame in the frameset. Hope that's unique enough.
  if(name.isEmpty())
  {
    QString tmp;
    tmp.sprintf("0x%p", this);
    name = DOMString(tmp) + url;
    kdDebug(300) << "creating frame name: " << name.string() << endl;
  }

  w->part()->requestFrame( renderFrame, url.string(), name.string() );

  NodeBaseImpl::attach( w );
}

// -------------------------------------------------------------------------

HTMLModElementImpl::HTMLModElementImpl(DocumentImpl *doc, ushort tagid) : HTMLElementImpl(doc)
{
    _id = tagid;
}

HTMLModElementImpl::~HTMLModElementImpl()
{
}

const DOMString HTMLModElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLModElementImpl::id() const
{
    return _id;
}

// -------------------------------------------------------------------------

HTMLQuoteElementImpl::HTMLQuoteElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLQuoteElementImpl::~HTMLQuoteElementImpl()
{
}

const DOMString HTMLQuoteElementImpl::nodeName() const
{
    return "Q";
}

ushort HTMLQuoteElementImpl::id() const
{
    return ID_Q;
}

