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
#include "xml_tokenizer.h"
#include "htmlhashes.h"

#include "html_baseimpl.h"
#include "html_blockimpl.h"
#include "html_documentimpl.h"
#include "html_elementimpl.h"
#include "html_formimpl.h"
#include "html_headimpl.h"
#include "html_imageimpl.h"
#include "html_inlineimpl.h"
#include "html_listimpl.h"
#include "html_miscimpl.h"
#include "html_tableimpl.h"
#include "html_objectimpl.h"

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
    // if tabIndex is -1, items containing tabIndex are skipped.

  kdDebug(6000)<<"HTMLDocumentImpl:findLink: Node: "<<n<<" forward: "<<(forward?"true":"false")<<" tabIndexHint: "<<tabIndexHint<<"\n";

    int maxTabIndex;

    if (forward) maxTabIndex = findHighestTabIndex();
    else maxTabIndex = -1;

    do
    {
	n = findSelectableElement(n, forward);
	// this is alright even for non-tabindex-searches,
	// because DOM::NodeImpl::tabIndex() defaults to -1.
    } while (n && (n->tabIndex()!=tabIndexHint));
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


ElementImpl *HTMLDocumentImpl::createElement( const DOMString &name )
{
    uint id = khtml::getTagID( name.string().lower().latin1(), name.string().length() );

    ElementImpl *n = 0;
    switch(id)
    {
    case ID_HTML:
	n = new HTMLHtmlElementImpl(this);
	break;
    case ID_HEAD:
	n = new HTMLHeadElementImpl(this);
	break;
    case ID_BODY:
	n = new HTMLBodyElementImpl(this);
	break;

// head elements
    case ID_BASE:
	n = new HTMLBaseElementImpl(this);
	break;
    case ID_LINK:
	n = new HTMLLinkElementImpl(this);
	break;
    case ID_META:
	n = new HTMLMetaElementImpl(this);
	break;
    case ID_STYLE:
	n = new HTMLStyleElementImpl(this);
	break;
    case ID_TITLE:
	n = new HTMLTitleElementImpl(this);
	break;

// frames
    case ID_FRAME:
	n = new HTMLFrameElementImpl(this);
	break;
    case ID_FRAMESET:
	n = new HTMLFrameSetElementImpl(this);
	break;
    case ID_IFRAME:
	n = new HTMLIFrameElementImpl(this);
	break;

// form elements
// ### FIXME: we need a way to set form dependency after we have made the form elements
    case ID_FORM:
	    n = new HTMLFormElementImpl(this);
	break;
    case ID_BUTTON:
            n = new HTMLButtonElementImpl(this);
	break;
    case ID_FIELDSET:
            n = new HTMLFieldSetElementImpl(this);
	break;
    case ID_INPUT:
            n = new HTMLInputElementImpl(this);
	break;
    case ID_ISINDEX:
	    n = new HTMLIsIndexElementImpl(this);
	break;
    case ID_LABEL:
            n = new HTMLLabelElementImpl(this);
	break;
    case ID_LEGEND:
            n = new HTMLLegendElementImpl(this);
	break;
    case ID_OPTGROUP:
            n = new HTMLOptGroupElementImpl(this);
	break;
    case ID_OPTION:
            n = new HTMLOptionElementImpl(this);
	break;
    case ID_SELECT:
            n = new HTMLSelectElementImpl(this);
	break;
    case ID_TEXTAREA:
            n = new HTMLTextAreaElementImpl(this);
	break;

// lists
    case ID_DL:
	n = new HTMLDListElementImpl(this);
	break;
    case ID_DD:
	n = new HTMLGenericElementImpl(this, id);
	break;
    case ID_DT:
	n = new HTMLGenericElementImpl(this, id);
	break;
    case ID_UL:
	n = new HTMLUListElementImpl(this);
	break;
    case ID_OL:
	n = new HTMLOListElementImpl(this);
	break;
    case ID_DIR:
	n = new HTMLDirectoryElementImpl(this);
	break;
    case ID_MENU:
	n = new HTMLMenuElementImpl(this);
	break;
    case ID_LI:
	n = new HTMLLIElementImpl(this);
	break;

// formatting elements (block)
    case ID_BLOCKQUOTE:
	n = new HTMLBlockquoteElementImpl(this);
	break;
    case ID_DIV:
	n = new HTMLDivElementImpl(this);
	break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6:
	n = new HTMLHeadingElementImpl(this, id);
	break;
    case ID_HR:
	n = new HTMLHRElementImpl(this);
	break;
    case ID_P:
	n = new HTMLParagraphElementImpl(this);
	break;
    case ID_PRE:
	n = new HTMLPreElementImpl(this);
	break;

// font stuff
    case ID_BASEFONT:
	n = new HTMLBaseFontElementImpl(this);
	break;
    case ID_FONT:
	n = new HTMLFontElementImpl(this);
	break;

// ins/del
    case ID_DEL:
    case ID_INS:
	n = new HTMLModElementImpl(this, id);
	break;

// anchor
    case ID_A:
	n = new HTMLAnchorElementImpl(this);
	break;

// images
    case ID_IMG:
	n = new HTMLImageElementImpl(this);
	break;
    case ID_MAP:
	n = new HTMLMapElementImpl(this);
	/*n = map;*/
	break;
    case ID_AREA:
	n = new HTMLAreaElementImpl(this);
	break;

// objects, applets and scripts
    case ID_APPLET:
	n = new HTMLAppletElementImpl(this);
	break;
    case ID_OBJECT:
	n = new HTMLObjectElementImpl(this);
	break;
    case ID_PARAM:
	n = new HTMLParamElementImpl(this);
	break;
    case ID_SCRIPT:
	n = new HTMLScriptElementImpl(this);
	break;

// tables
    case ID_TABLE:
	n = new HTMLTableElementImpl(this);
	break;
    case ID_CAPTION:
	n = new HTMLTableCaptionElementImpl(this);
	break;
    case ID_COLGROUP:
    case ID_COL:
	n = new HTMLTableColElementImpl(this, id);
	break;
    case ID_TR:
	n = new HTMLTableRowElementImpl(this);
	break;
    case ID_TD:
    case ID_TH:
	n = new HTMLTableCellElementImpl(this, id);
	break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT:
	n = new HTMLTableSectionElementImpl(this, id);
	break;

// inline elements
    case ID_BR:
	n = new HTMLBRElementImpl(this);
	break;
    case ID_Q:
	n = new HTMLQuoteElementImpl(this);
	break;

// elements with no special representation in the DOM

// block:
    case ID_ADDRESS:
    case ID_CENTER:
	n = new HTMLGenericElementImpl(this, id);
	break;
// inline
	// %fontstyle
    case ID_TT:
    case ID_U:
    case ID_B:
    case ID_I:
    case ID_S:
    case ID_STRIKE:
    case ID_BIG:
    case ID_SMALL:

	// %phrase
    case ID_EM:
    case ID_STRONG:
    case ID_DFN:
    case ID_CODE:
    case ID_SAMP:
    case ID_KBD:
    case ID_VAR:
    case ID_CITE:
    case ID_ABBR:
    case ID_ACRONYM:

	// %special
    case ID_SUB:
    case ID_SUP:
    case ID_SPAN:
	n = new HTMLGenericElementImpl(this, id);
	break;

    case ID_BDO:
	break;

// text
    case ID_TEXT:
        kdDebug( 6020 ) << "Use document->createTextNode()" << endl;
	break;

    default:
	// ### don't allow this for normal HTML documents
	n = DocumentImpl::createElement(name);
    }
    return n;
}

ElementImpl *HTMLDocumentImpl::createElementNS ( const DOMString &_namespaceURI, const DOMString &_qualifiedName )
{
    // ### somehow set the namespace for html elements to http://www.w3.org/1999/xhtml ?
    if (_namespaceURI == "http://www.w3.org/1999/xhtml") {
	QString qName = _qualifiedName.string();
	int colonPos = qName.find(':',0);
	return createElement(colonPos ? qName.mid(colonPos+1) : qName);
    }
    else
	return DocumentImpl::createElementNS(_namespaceURI,_qualifiedName);
}

//------------------------------------------------------------------------------


XHTMLDocumentImpl::XHTMLDocumentImpl() : HTMLDocumentImpl()
{
}

XHTMLDocumentImpl::XHTMLDocumentImpl(KHTMLView *v) : HTMLDocumentImpl(v)
{
}

XHTMLDocumentImpl::~XHTMLDocumentImpl()
{
}

Tokenizer *XHTMLDocumentImpl::createTokenizer()
{
    return new XMLTokenizer(this,m_view);
}



#include "html_documentimpl.moc"
