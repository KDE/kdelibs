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
#include "html_documentimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "khtml_settings.h"
#include "misc/htmlattrs.h"

#include "htmltokenizer.h"
#include "htmlhashes.h"

#include "html_imageimpl.h"

#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kglobalsettings.h>

#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include <stdlib.h>
#include <qstack.h>

template class QStack<DOM::NodeImpl>;

using namespace DOM;
using namespace khtml;


HTMLDocumentImpl::HTMLDocumentImpl() : DocumentImpl()
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor this = " << this << endl;
    bodyElement = 0;
    htmlElement = 0;
}

HTMLDocumentImpl::HTMLDocumentImpl(KHTMLView *v)
    : DocumentImpl(v)
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor2 this = " << this << endl;
    bodyElement = 0;
    htmlElement = 0;

    m_styleSelector = new CSSStyleSelector(this);
}

HTMLDocumentImpl::~HTMLDocumentImpl()
{
}

DOMString HTMLDocumentImpl::referrer() const
{
    // ### should we fix that? I vote against for privacy reasons
    return DOMString();
}

DOMString HTMLDocumentImpl::domain() const
{
    // ### do they want the host or the domain????
    KURL u(url.string());
    return u.host();
}

HTMLElementImpl *HTMLDocumentImpl::body()
{
    if(bodyElement) return bodyElement;
    if(!_first) return 0;
    if(!htmlElement)
        html();
    if(!htmlElement) return 0;
    NodeImpl *test = htmlElement->firstChild();
    while(test && (test->id() != ID_BODY && test->id() != ID_FRAMESET))
        test = test->nextSibling();
    if(!test) return 0;
    bodyElement = static_cast<HTMLElementImpl *>(test);
    return bodyElement;
}

HTMLElementImpl *HTMLDocumentImpl::html()
{
    if (htmlElement)
        return htmlElement;


    NodeImpl *n = _first;
    while (n && n->id() != ID_HTML)
        n = n->nextSibling();
    if (n) {
        htmlElement = static_cast<HTMLElementImpl*>(n);
        return htmlElement;
    }
    else
        return 0;
}


void HTMLDocumentImpl::setBody(const HTMLElement &/*_body*/)
{
    // ###
}

Tokenizer *HTMLDocumentImpl::createTokenizer()
{
    return new HTMLTokenizer(this,m_view);
}

NodeListImpl *HTMLDocumentImpl::getElementsByName( const DOMString &elementName )
{
    return new NameNodeListImpl( documentElement(), elementName );
}

// Please see if there`s a possibility to merge that code
// with the next function and getElementByID().
NodeImpl *HTMLDocumentImpl::findElement( int id )
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
            if(current->id() == id)
                return current;

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

HTMLElementImpl *HTMLDocumentImpl::findSelectableElement(NodeImpl *start, bool forward)
{
    if (!start)
	start = forward?_first:_last;
    if (!start)
	return 0;
    if (forward)
	while(1)
	{
	    if (start->firstChild())
		start = start->firstChild();
	    else if (start->nextSibling())
		start = start->nextSibling();
	    else // find the next sibling of the first parent that has a nextSibling
	    {
		NodeImpl *pa = start;
		while (pa)
		{
		    pa = pa->parentNode();
		    if (!pa)
			return 0;
		    if (pa->nextSibling())
		    {
			start = pa->nextSibling();
			pa = 0;
		    }
		}
	    }
	    if (start->isElementNode() && static_cast<HTMLElementImpl *>(start)->isSelectable())
		return static_cast<HTMLElementImpl*>(start);
	}
    else
	while (1)
	{
	    if (start->lastChild())
		start = start->lastChild();
	    else if (start->previousSibling())
		start = start->previousSibling();
	    else
	    {
		NodeImpl *pa = start;
		while (pa)
		{
		  // find the previous sibling of the first parent that has a prevSibling
		    pa = pa->parentNode();
		    if (!pa)
			return 0;
		    if (pa->previousSibling())
		    {
			start = pa->previousSibling();
			pa = 0;
			break;
		    }
		}
	    }
	    if (start->isElementNode() && static_cast<HTMLElementImpl*>(start)->isSelectable())
		return static_cast<HTMLElementImpl*>(start);
	}
    kdFatal(6000) << "some error in findElement\n";
}


StyleSheetListImpl *HTMLDocumentImpl::styleSheets()
{
    // ### implement for html
    return 0;
}


// --------------------------------------------------------------------------
// not part of the DOM
// --------------------------------------------------------------------------

bool HTMLDocumentImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
                                  int, int, DOMString &url,
                                   NodeImpl *&innerNode, long &offset)
{
    bool inside = false;
    NodeImpl *n = firstChild();
    while ( n && n->id() != ID_HTML )
        n = n->nextSibling();
    if ( n )
        inside = n->mouseEvent(_x, _y, button, type, 0, 0, url, innerNode, offset);
    //kdDebug(0) << "documentImpl::mouseEvent " << n->id() << " " << inside << endl;
    return inside;
}

void HTMLDocumentImpl::detach()
{
    // onunload script...
    if(m_view && m_view->part() && m_view->part()->jScriptEnabled() && body()) {
        DOMString script = body()->getAttribute(ATTR_ONUNLOAD);
        if(script.length()) {
            //kdDebug( 6030 ) << "emit executeScript( " << script.string() << " )" << endl;
            m_view->part()->executeScript( Node(this), script.string() );
        }
    }
    kdDebug( 6090 ) << "HTMLDocumentImpl::detach()" << endl;
    m_view = 0;

    DocumentImpl::detach();
}

int HTMLDocumentImpl::findHighestTabIndex()
{
    NodeImpl *n=body();
    NodeImpl *next=0;
    HTMLAreaElementImpl *a;
    int retval=-1;
    int tmpval;
    while(n)
    {
	//find out tabindex of current element, if availiable
	if (n->id()==ID_A)
        {
	    a=static_cast<HTMLAreaElementImpl *>(n);
	    tmpval=a->tabIndex();
	    if (tmpval>retval)
		retval=tmpval;
        }
	//iterate to next element.
	if (n->firstChild())
	    n=n->firstChild();
	else if (n->nextSibling())
	    n=n->nextSibling();
	else
        {
	    next=0;
	    while(!next)
            {
		n=n->parentNode();
		if (!n)
		    return retval;
		next=n->nextSibling();
            }
	    n=next;
        }
    }
    return retval;
}

HTMLElementImpl *HTMLDocumentImpl::findLink(HTMLElementImpl *n, bool forward, int tabIndexHint)
{
    // tabIndexHint is the tabIndex that should be found.
    // if it is not in the document, and direction is forward,
    // tabIndexHint is incremented until maxTabIndex is reached.
    // if direction is backward, tabIndexHint is reduced until -1
    // is encountered.
    // if tabIndex is -1, items containing tabIndex are skipped.

  kdDebug(6000)<<"HTMLDocumentImpl:findLink: Node: "<<n<<" forward: "<<(forward?"true":"false")<<" tabIndexHint: "<<tabIndexHint<<"\n";

    int maxTabIndex;

    if (forward) maxTabIndex = findHighestTabIndex();
    else maxTabIndex = -1;

    do
    {
	do
	{
	    n = findSelectableElement(n, forward);
	    // this is alright even for non-tabindex-searches,
	    // because DOM::NodeImpl::tabIndex() defaults to -1.
	} while (n && (n->tabIndex()!=tabIndexHint));
	if (n)
	    break;
	if (tabIndexHint!=-1)
	{
	    if (forward)
	    {
		tabIndexHint++;
		if (tabIndexHint>maxTabIndex)
		    tabIndexHint=-1;
		n=0;
	    }
	    else
	    {
		tabIndexHint--;
		n=0;
	    }
	    kdDebug(6000)<<"trying next tabIndexHint:"<<tabIndexHint<<"\n";
	}
	// this is not the same as else ... ,
	// since tabIndexHint may have been changed in the block above.
	if (tabIndexHint==-1)
	    break;
    } while(1); // break.

    return n;
}

void HTMLDocumentImpl::slotFinishedParsing()
{
    // onload script...
    if(m_view && m_view->part()->jScriptEnabled() && body()) {
        DOMString script = body()->getAttribute(ATTR_ONLOAD);
        if(script.length()) {
            kdDebug( 6030 ) << "emit executeScript( " << script.string() << " )" << endl;
            m_view->part()->executeScript( Node(this), script.string() );
        }
    }
    if ( !onloadScript.isEmpty() )
	m_view->part()->executeScript( Node(this), onloadScript );
    DocumentImpl::slotFinishedParsing();
}

bool HTMLDocumentImpl::childAllowed( NodeImpl *newChild )
{
    return (newChild->id() == ID_HTML || newChild->id() == ID_COMMENT);
}

#include "html_documentimpl.moc"
