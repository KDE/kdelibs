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

#include <qstack.h>

#include "khtmlparser.h"
#include "khtmltokenizer.h"
#include "khtmlfont.h"
#include "khtmldata.h"
#include "khtmlio.h"
#include "khtml.h"
#include "khtmlattrs.h"

#include "html_misc.h"
#include "dom_node.h"
#include "dom_element.h"
#include "dom_exception.h"
#include "html_element.h"
#include "html_elementimpl.h"
#include "html_baseimpl.h"
#include "dom_string.h"
#include "html_imageimpl.h"

#include "html_documentimpl.h"

#include <stdio.h>
#include <kurl.h>

using namespace DOM;

HTMLDocumentImpl::HTMLDocumentImpl() : DocumentImpl()
{
    printf("HTMLDocumentImpl constructor this = %p\n",this);
    view = 0;
    parser = 0;
    tokenizer = 0;

    bodyElement = 0;

    width = 0;
    height = 0;

    if (!pFontManager)
	pFontManager = new HTMLFontManager();
    if (!pSettings)
	pSettings = new HTMLSettings();

}

HTMLDocumentImpl::HTMLDocumentImpl(KHTMLWidget *v, KHTMLCache *c)
    : DocumentImpl()
{
    printf("HTMLDocumentImpl constructor2 this = %p\n",this);
    view = v;
    parser = 0;
    tokenizer = 0;
    cache  = c;

    width = view->_width;
    printf("document: setting width to %d\n", width);
    height = view->height();

    bodyElement = 0;

    if (!pFontManager)
	pFontManager = new HTMLFontManager();
    if (!pSettings)
	pSettings = new HTMLSettings();
}

HTMLDocumentImpl::~HTMLDocumentImpl()
{
    printf("HTMLDocumentImpl destructor this = %p\n",this);
}

DOMString HTMLDocumentImpl::referrer() const
{
    // ###
}

DOMString HTMLDocumentImpl::domain() const
{
    // ###
}

DOMString HTMLDocumentImpl::URL() const
{
    // ### FIXME
    return 0;
}

HTMLElementImpl *HTMLDocumentImpl::body()
{
    if(bodyElement) return bodyElement;
    if(!_first) return 0;
    NodeImpl *test = _first->firstChild();
    if(!test) return 0;
    while(test && (test->id() != ID_BODY && test->id() != ID_FRAMESET))
	test = test->nextSibling();
    bodyElement = static_cast<HTMLElementImpl *>(test);
    return bodyElement;
}

void HTMLDocumentImpl::open(  )
{
    printf("HTMLDocumentImpl::open()\n");
    clear();
    parser = new KHTMLParser(view, this);
    tokenizer = new HTMLTokenizer(parser);
    tokenizer->begin();
}

void HTMLDocumentImpl::close(  )
{
    if(parser) delete parser;
    parser = 0;
    if(tokenizer) delete tokenizer;
    tokenizer = 0;
}

void HTMLDocumentImpl::write( const DOMString &text )
{
    if(tokenizer)
	tokenizer->write(text.string());
}

void HTMLDocumentImpl::write( const QString &text )
{
    if(tokenizer)
	tokenizer->write(text);
}

void HTMLDocumentImpl::writeln( const DOMString &text )
{
    // ??? is this correct
    write(text);
}

ElementImpl *HTMLDocumentImpl::getElementById( const DOMString &elementId )
{
    QStack<NodeImpl> nodeStack;
    NodeImpl *current = _first;

    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    if(current->isElementNode())
	    {
		ElementImpl *e = static_cast<ElementImpl *>(current);
		if(e->getAttribute(ATTR_ID) == elementId)
		    return e;
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
    }

    return 0;
}


NodeList HTMLDocumentImpl::getElementsByName( const DOMString &elementName )
{
    // ###
}


// --------------------------------------------------------------------------
// not part of the DOM
// --------------------------------------------------------------------------

void HTMLDocumentImpl::clear()
{
    if(parser) delete parser;
    if(tokenizer) delete tokenizer;
    parser = 0;
    tokenizer = 0;

    // #### clear tree
}


DOMString HTMLDocumentImpl::requestImage(HTMLImageRequester *n, DOMString url)
{
    KURL u(view->url(), url.string());

    cache->requestImage(n, u.url());

    return u.url();
}

NodeImpl *HTMLDocumentImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("Document::addChild( %s )\n", newChild->nodeName().string().ascii());
#endif

    // short check for consistency with DTD
    if(newChild->id() != ID_HTML)
    {
	printf("AddChild failed! id=#document, child->id=%d\n", newChild->id());
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    }

    if(_first)
    {
	printf("AddChild failed! id=#document, child->id=%d. Already have a HTML element!\n", newChild->id());
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    }

    // just add it...
    newChild->setParent(this);
    newChild->setAvailableWidth(width);
    _first = _last = newChild;
    return newChild;
}

void HTMLDocumentImpl::layout( bool deep )
{
#ifdef DEBUG_LAYOUT
    printf("%s(Document)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    height = 0;

    if(_first)
    {

	_first->setPos(0, 0);
	_first->setDescent(view->height());
	setAvailableWidth(width);
	if(deep)
	    _first->layout(deep);
	height = _first->getHeight();
    }
}

void HTMLDocumentImpl::setAvailableWidth(int w) {
    if(w != -1) width = w;
    if(bodyElement)
    {
	bodyElement->calcMinMaxWidth();
    	int tw = width > bodyElement->getMinWidth() ? width :
	    bodyElement->getMinWidth();
    	bodyElement->setAvailableWidth(tw);
//    	printf("Doc:setAvailableWidth %d %d\n",width, tw);
    }
}


bool HTMLDocumentImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int, int, DOMString &url)
{
    if(body())
      {
	return bodyElement->mouseEvent(_x, _y, button, type, 0, 0, url);
      }
    return false;
}

void HTMLDocumentImpl::print(NodeImpl *e, bool recursive)
{
    view->paintElement(e, recursive);
}

void HTMLDocumentImpl::updateSize()
{
    if(body())
    {
    	int oldw = width;
	int oldh = height;
	
    	layout(true);		
	
	if(width != oldw || height != oldh)
	{
	    if(view)
	    {
		view->resizeContents(width, height);
		// ### schedule layout!
	    }
	}
	if (view)	
	   view->viewport()->repaint(true);
    }
}

void HTMLDocumentImpl::attach(KHTMLWidget *w)
{
    view = w;
    cache = view->cache;

    NodeBaseImpl::attach(w);
}

void HTMLDocumentImpl::detach()
{
    view = 0;
    cache = 0;

    NodeBaseImpl::detach();
}
