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
#include "misc/loader.h"

#include "htmlparser.h"
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
#include "rendering/render_style.h"
#include "rendering/render_root.h"
#include <stdlib.h>

#include <qstack.h>


using namespace DOM;
using namespace khtml;

template class QStack<DOM::NodeImpl>;

HTMLDocumentImpl::HTMLDocumentImpl() : DocumentImpl()
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor this = " << this << endl;
    parser = 0;
    tokenizer = 0;

    bodyElement = 0;
    htmlElement = 0;

    m_loadingSheet = false;

    m_elemSheet=0;
    m_docLoader = new DocLoader();

    visuallyOrdered = false;
}

HTMLDocumentImpl::HTMLDocumentImpl(KHTMLView *v)
    : DocumentImpl(v)
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl constructor2 this = " << this << endl;
    parser = 0;
    tokenizer = 0;

    bodyElement = 0;
    htmlElement = 0;

    m_styleSelector = new CSSStyleSelector(this);

    m_loadingSheet = false;

    m_elemSheet=0;
    m_docLoader = new DocLoader();

    visuallyOrdered = false;
}

HTMLDocumentImpl::~HTMLDocumentImpl()
{
//    kdDebug( 6090 ) << "HTMLDocumentImpl destructor this = " << this << endl;
    delete m_docLoader;
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

DOMString HTMLDocumentImpl::baseURL() const
{
    if(!view()->part()->baseURL().isEmpty()) return view()->part()->baseURL().url();
    return url;
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

void HTMLDocumentImpl::open(  )
{
    //kdDebug( 6030 ) << "HTMLDocumentImpl::open()" << endl;
    clear();
    parser = new KHTMLParser(m_view, this);
    tokenizer = new HTMLTokenizer(parser, m_view);
    connect(tokenizer,SIGNAL(finishedParsing()),this,SLOT(slotFinishedParsing()));
    tokenizer->begin();
}

void HTMLDocumentImpl::close(  )
{
    if (m_render)
        m_render->close();

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
    write(text);
    write(DOMString("\n"));
}

void HTMLDocumentImpl::finishParsing (  )
{
    if(tokenizer)
        tokenizer->finish();
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


NodeListImpl *HTMLDocumentImpl::getElementsByName( const DOMString &elementName )
{
    return new NameNodeListImpl( documentElement(), elementName );
}

#ifdef __GNUC__
#warning bloatware
#endif
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

void HTMLDocumentImpl::clear()
{
    if(parser) delete parser;
    if(tokenizer) delete tokenizer;
    parser = 0;
    tokenizer = 0;

    // #### clear tree
}

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

void HTMLDocumentImpl::attach(KHTMLView *w)
{
    m_view = w;
    if(!m_styleSelector) createSelector();
    m_render = new RenderRoot(w);
    recalcStyle();

    NodeBaseImpl::attach(w);
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

void HTMLDocumentImpl::setVisuallyOrdered()
{
    visuallyOrdered = true;
    if(!m_style) return;
    m_style->setVisuallyOrdered(true);
}

void HTMLDocumentImpl::createSelector()
{
    applyChanges();
}

// ### this function should not be needed in the long run. The one in
// DocumentImpl should be enough.
void HTMLDocumentImpl::applyChanges(bool,bool force)
{
    if(m_styleSelector) delete m_styleSelector;
    m_styleSelector = new CSSStyleSelector(this);
    if(!m_render) return;

    recalcStyle();

    // a style change can influence the children, so we just go
    // through them and trigger an appplyChanges there too
    NodeImpl *n = _first;
    while(n) {
        n->applyChanges(false,force || changed());
        n = n->nextSibling();
    }

    // force a relayout of this part of the document
    m_render->layout();
    // force a repaint of this part.
    // ### if updateSize() changes any size, it will already force a
    // repaint, so we might do double work here...
    m_render->repaint();
    setChanged(false);
}

void HTMLDocumentImpl::recalcStyle()
{
    QTime qt;
    qt.start();
    if( !m_render ) return;
    if( m_style ) delete m_style;
    m_style = new RenderStyle();
    m_style->setDisplay(BLOCK);
    m_style->setVisuallyOrdered( visuallyOrdered );
    // ### make the font stuff _really_ work!!!!
    const KHTMLSettings *settings = m_view->part()->settings();
    QValueList<int> fs = settings->fontSizes();
    int size = fs[3];
    if(size < settings->minFontSize())
        size = settings->minFontSize();
    QFont f = KGlobalSettings::generalFont();
    f.setFamily(settings->stdFontName());
    f.setPointSize(size);
    //kdDebug() << "HTMLDocumentImpl::attach: setting to charset " << settings->charset() << endl;
    KGlobal::charsets()->setQFont(f, settings->charset());
    m_style->setFont(f);

    m_style->setHtmlHacks(true); // enable html specific rendering tricks
    if(m_render)
	m_render->setStyle(m_style);

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling())
	n->recalcStyle();
    kdDebug( ) << "TIME: recalcStyle() dt=" << qt.elapsed() << endl;
}

void HTMLDocumentImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet)
{
    kdDebug( 6030 ) << "HTMLDocument::setStyleSheet()" << endl;
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString(sheet);
    m_loadingSheet = false;

    createSelector();
}

CSSStyleSheetImpl* HTMLDocumentImpl::elementSheet()
{
    if (!m_elemSheet)
        m_elemSheet = new CSSStyleSheetImpl(this, baseURL());
    return m_elemSheet;
}


void HTMLDocumentImpl::setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep)
{
    static_cast<RenderRoot*>(m_render)
        ->setSelection(s->renderer(),sp,e->renderer(),ep);
}

void HTMLDocumentImpl::clearSelection()
{
    static_cast<RenderRoot*>(m_render)
        ->clearSelection();
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
    emit finishedParsing();
}

bool HTMLDocumentImpl::childAllowed( NodeImpl *newChild )
{
    return (newChild->id() == ID_HTML || newChild->id() == ID_COMMENT);
}

void HTMLDocumentImpl::setReloading()
{
    m_docLoader->reloading = true;
}

void HTMLDocumentImpl::updateRendering()
{
    QListIterator<NodeImpl> it(changedNodes);
    for (; it.current(); ++it) {
	if( it.current()->changed() )
	    it.current()->applyChanges( true, true );
    }
     changedNodes.clear();
}


#include "html_documentimpl.moc"
