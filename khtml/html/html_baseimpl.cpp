/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org))
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
#include "html_baseimpl.h"

#include "html_documentimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"

#include "rendering/render_frames.h"
#include "rendering/render_html.h"
#include "rendering/render_body.h"
#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssproperties.h"
#include "misc/loader.h"
#include "misc/htmlhashes.h"
#include "dom/dom_string.h"

#include <kurl.h>
#include <kdebug.h>

using namespace DOM;
using namespace khtml;

HTMLBodyElementImpl::HTMLBodyElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    m_styleSheet = 0;
}

HTMLBodyElementImpl::~HTMLBodyElementImpl()
{
}

const DOMString HTMLBodyElementImpl::nodeName() const
{
    return "BODY";
}

ushort HTMLBodyElementImpl::id() const
{
    return ID_BODY;
}

void HTMLBodyElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {

    case ATTR_BACKGROUND:
    {
	KURL u = khtml::Cache::completeURL(attr->value(), static_cast<HTMLDocumentImpl *>(document)->baseURL());
	bgImage = u.url();
	addCSSProperty(CSS_PROP_BACKGROUND_IMAGE, u.url(), false);
	break;
    }
    case ATTR_MARGINWIDTH:
    {
	QString str;
	str += attr->value().string() + "px";
        addCSSProperty(CSS_PROP_PADDING_LEFT, str, false);
        addCSSProperty(CSS_PROP_PADDING_RIGHT, str, false);
	break;	
    }
    case ATTR_MARGINHEIGHT:
    {
	QString str;
	str += attr->value().string() + "px";
        addCSSProperty(CSS_PROP_PADDING_TOP, str, false);
        addCSSProperty(CSS_PROP_PADDING_BOTTOM, str, false);
	break;
    }
    case ATTR_BGCOLOR:
	bgColor = attr->value();
	addCSSProperty(CSS_PROP_BACKGROUND_COLOR, attr->value(), false);
	break;
    case ATTR_TEXT:
	addCSSProperty(CSS_PROP_COLOR, attr->value(), false);
	break;
    case ATTR_LINK:
    {
	kdDebug( 6030 ) << "ATTR_LINK" << endl;
	if(!m_styleSheet) m_styleSheet = new CSSStyleSheetImpl(this);
	QString aStr = "a[href] { color: " + attr->value().string() + "; }";
	m_styleSheet->parseString(aStr);
	m_styleSheet->setNonCSSHints();
	break;
    }
    case ATTR_VLINK:
	// ### has to be added as stylesheet
    case ATTR_ALINK:
      break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}


void HTMLBodyElementImpl::attach(KHTMLView *w)
{
    bool recalc = false;

    if(w->marginWidth() != -1) {
	QString str;
	str.sprintf("%dpx",w->marginWidth());
	addCSSProperty(CSS_PROP_PADDING_LEFT, str, false);
        addCSSProperty(CSS_PROP_PADDING_RIGHT, str, false);	
    }
    if(w->marginHeight() != -1) {
	QString str;
	str.sprintf("%dpx",w->marginHeight());
	addCSSProperty(CSS_PROP_PADDING_TOP, str, false);
        addCSSProperty(CSS_PROP_PADDING_BOTTOM, str, false);
    }

    m_style = document->styleSelector()->styleForElement( this );
    khtml::RenderObject *r = _parent->renderer();

    if ( !r )
      return;

    m_render = new khtml::RenderBody();
    m_render->setStyle(m_style);
    r->addChild( m_render, _next ? _next->renderer() : 0 );

    NodeBaseImpl::attach( w );
}

// -------------------------------------------------------------------------

HTMLFrameElementImpl::HTMLFrameElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    view = 0;
    parentWidget = 0;

    frameBorder = true;
    marginWidth = -1;
    marginHeight = -1;
    scrolling = QScrollView::Auto;
    noresize = false;
}

HTMLFrameElementImpl::~HTMLFrameElementImpl()
{
}

const DOMString HTMLFrameElementImpl::nodeName() const
{
    return "FRAME";
}

ushort HTMLFrameElementImpl::id() const
{
    return ID_FRAME;
}

void HTMLFrameElementImpl::parseAttribute(AttrImpl *attr)
{
    kdDebug( 6031 ) << "parsing attribute " << attr->attrId << "=" << attr->value().string() << endl;

    switch(attr->attrId)
    {
    case ATTR_SRC:
	url = attr->value();
	break;
    case ATTR_NAME:
	name = attr->value();
	break;
    case ATTR_FRAMEBORDER:
	if(attr->value() == "0" || strcasecmp( attr->value(), "no" ) == 0 )
	    frameBorder = false;
	break;
    case ATTR_MARGINWIDTH:
	marginWidth = attr->val()->toInt();
	break;
    case ATTR_MARGINHEIGHT:
	marginHeight = attr->val()->toInt();
	break;
    case ATTR_NORESIZE:
	noresize = true;
	break;
    case ATTR_SCROLLING:
	kdDebug( 6031 ) << "set scroll mode" << endl;
	if( strcasecmp( attr->value(), "auto" ) == 0 )
	    scrolling = QScrollView::Auto;
	else if( strcasecmp( attr->value(), "yes" ) == 0 )
	    scrolling = QScrollView::AlwaysOn;
	else if( strcasecmp( attr->value(), "no" ) == 0 )
	    scrolling = QScrollView::AlwaysOff;

    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameElementImpl::attach(KHTMLView *w)
{
    kdDebug( 6031 ) << "Frame::attach" <<endl;

    m_style = document->styleSelector()->styleForElement( this );

    khtml::RenderObject *r = _parent->renderer();

    if ( !r )
      return;

    khtml::RenderFrame *renderFrame = new khtml::RenderFrame( w, this );
    m_render = renderFrame;
    m_render->setStyle(m_style);
    r->addChild( m_render, _next ? _next->renderer() : 0 );

    // we need a unique name for every frame in the frameset. Hope that's unique enough.
    if(name.isEmpty())
    {
      QString tmp;
      tmp.sprintf("0x%p", this);
      name = DOMString(tmp) + url;
      kdDebug( 6030 ) << "creating frame name: " << name.string() << endl;
    }

    w->part()->requestFrame( renderFrame, url.string(), name.string() );

    NodeBaseImpl::attach( w );
    return;
}

void HTMLFrameElementImpl::detach()
{
    delete view;
    parentWidget = 0;
    HTMLElementImpl::detach();
}

// -------------------------------------------------------------------------

HTMLFrameSetElementImpl::HTMLFrameSetElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    // default value for rows and cols...
    m_totalRows = 1;
    m_totalCols = 1;

    m_rows = m_cols = 0;

    frameborder = true;
    m_border = 4;
    noresize = false;

    m_resizing = false;

    view = 0;
}

HTMLFrameSetElementImpl::~HTMLFrameSetElementImpl()
{
  if ( m_rows ) delete m_rows;
  if ( m_cols ) delete m_cols;
}

const DOMString HTMLFrameSetElementImpl::nodeName() const
{
    return "FRAMESET";
}

ushort HTMLFrameSetElementImpl::id() const
{
    return ID_FRAMESET;
}

void HTMLFrameSetElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ROWS:
	m_rows = attr->val()->toLengthList();
	m_totalRows = m_rows->count();
	break;
    case ATTR_COLS:
	m_cols = attr->val()->toLengthList();
	m_totalCols = m_cols->count();
	break;
    case ATTR_FRAMEBORDER:
	if(attr->value() == "0" || strcasecmp( attr->value(), "no" ) == 0 )
	    frameborder = false;
	break;
    case ATTR_NORESIZE:
	noresize = true;
	break;
    case ATTR_BORDER:
	m_border = attr->val()->toInt();
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameSetElementImpl::attach(KHTMLView *w)
{
    m_style = document->styleSelector()->styleForElement( this );
    view = w;

    // view->layout()

    khtml::RenderObject *r = _parent->renderer();

    if ( !r )
      return;

    khtml::RenderFrameSet *renderFrameSet = new khtml::RenderFrameSet( this, w, m_rows, m_cols );
    m_render = renderFrameSet;
    m_render->setStyle(m_style);
    r->addChild( m_render, _next ? _next->renderer() : 0 );

    NodeBaseImpl::attach( w );
}

NodeImpl *HTMLFrameSetElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << nodeName().string() << "(FrameSet)::addChild( " << child->nodeName().string() << " )" << endl;
#endif

    return NodeBaseImpl::addChild(child);
}

bool HTMLFrameSetElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url,
                                          NodeImpl *&innerNode, long &offset)
{
    _x-=_tx;
    _y-=_ty;
    kdDebug( 6030 ) << "mouseEvent" << endl;

    NodeImpl *child = _first;
    while(child)
    {
	if(child->id() == ID_FRAMESET)
	    if(child->mouseEvent( _x, _y, button, type, _tx, _ty, url, innerNode, offset)) return true;
	child = child->nextSibling();
    }

    kdDebug( 6030 ) << "children done.." << endl;

    if(noresize) return true;

    if ( !m_render || !m_render->layouted() )
    {
      kdDebug( 6030 ) << "ugh, not layouted or not attached?!" << endl;
      return true;
    }

    return static_cast<khtml::RenderFrameSet *>(m_render)->userResize( _x, _y, type );

    /*
    {
      kdDebug( 6030 ) << "mouseEvent:check" << endl;
	
	hSplit = -1;
	vSplit = -1;
	//bool resizePossible = true;

	// check if we're over a horizontal or vertical boundary
	int pos = m_colWidth[0];
	for(int c = 1; c < m_totalCols; c++)
	{
	    if(_x >= pos && _x <= pos+m_border)
	    {
		if(vSplitVar && vSplitVar[c-1] == true) vSplit = c-1;
		kdDebug( 6030 ) << "vsplit!" << endl;
		break;
	    }
	    pos += m_colWidth[c] + m_border;
	}
	pos = m_rowHeight[0];
	for(int r = 1; r < m_totalRows; r++)
	{
	    if( _y >= pos && _y <= pos+m_border)
	    {
		if(hSplitVar && hSplitVar[r-1] == true) hSplit = r-1;
		kdDebug( 6030 ) << "hsplitvar = " << hSplitVar << endl;
		kdDebug( 6030 ) << "hsplit!" << endl;
		break;
	    }
	    pos += m_rowHeight[r] + m_border;
	}
	kdDebug( 6030 ) << hSplit << "/" << vSplit << endl;

	QCursor cursor;
	if(hSplit != -1 && vSplit != -1)
	{
	    cursor = Qt::sizeAllCursor;
	}
	else if( vSplit != -1 )
	{
	    cursor = Qt::splitHCursor;
	}	
	else if( hSplit != -1 )
	{
	    cursor = Qt::splitVCursor;
	}

	if(type == MousePress)
	{
	    resizing = true;
	    KApplication::setOverrideCursor(cursor);
	    vSplitPos = _x;
	    hSplitPos = _y;
	}
	else
	    view->setCursor(cursor);
    }

    // ### need to draw a nice movin indicator for the resize.
    // ### check the resize is not going out of bounds.
    if(resizing && type == MouseRelease)
    {
	resizing = false;
	KApplication::restoreOverrideCursor();

	if(hSplit)
	{
	    kdDebug( 6030 ) << "split xpos=" << _x << endl;
	    int delta = vSplitPos - _x;
	    m_colWidth[vSplit] -= delta;
	    m_colWidth[vSplit+1] += delta;
	}	
	if(vSplit)
	{
	    kdDebug( 6030 ) << "split ypos=" << _y << endl;
	    int delta = hSplitPos - _y;
	    m_rowHeight[hSplit] -= delta;
	    m_rowHeight[hSplit+1] += delta;
	}
	kdDebug( 6030 ) << "starting relayout!" << endl;
	m_render->layout( true );
    }
    */
}
// -------------------------------------------------------------------------

HTMLHeadElementImpl::HTMLHeadElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLHeadElementImpl::~HTMLHeadElementImpl()
{
}

const DOMString HTMLHeadElementImpl::nodeName() const
{
    return "HEAD";
}

ushort HTMLHeadElementImpl::id() const
{
    return ID_HEAD;
}

void HTMLHtmlElementImpl::attach(KHTMLView *w)
{
    m_style = document->styleSelector()->styleForElement( this );
    khtml::RenderObject *r = _parent->renderer();

    if ( !r )
      return;

    m_render = new khtml::RenderHtml();
    m_render->setStyle(m_style);
    r->addChild( m_render, _next ? _next->renderer() : 0 );

    NodeBaseImpl::attach( w );
}


// -------------------------------------------------------------------------

HTMLHtmlElementImpl::HTMLHtmlElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLHtmlElementImpl::~HTMLHtmlElementImpl()
{
}

const DOMString HTMLHtmlElementImpl::nodeName() const
{
    return "HTML";
}

ushort HTMLHtmlElementImpl::id() const
{
    return ID_HTML;
}


// -------------------------------------------------------------------------

HTMLIFrameElementImpl::HTMLIFrameElementImpl(DocumentImpl *doc) : HTMLFrameElementImpl(doc)
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

void HTMLIFrameElementImpl::parseAttribute(AttrImpl *attr )
{
  DOM::DOMStringImpl *stringImpl = attr->value().implementation();
  QString val = QConstString( stringImpl->s, stringImpl->l ).string();
  switch (  attr->attrId )
  {
    case ATTR_WIDTH:
      addCSSLength( CSS_PROP_WIDTH, attr->value(), false );
      break;
    case ATTR_HEIGHT:
      addCSSLength( CSS_PROP_HEIGHT, attr->value(), false );
      break;
    default:
      HTMLFrameElementImpl::parseAttribute( attr );
  }
}

void HTMLIFrameElementImpl::attach(KHTMLView *w)
{
  m_style = document->styleSelector()->styleForElement( this );

  khtml::RenderObject *r = _parent->renderer();

  if ( !r )
    return;

  // we need a unique name for every frame in the frameset. Hope that's unique enough.
  if(name.isEmpty())
  {
    QString tmp;
    tmp.sprintf("0x%p", this);
    name = DOMString(tmp) + url;
    kdDebug( 6030 ) << "creating frame name: " << name.string() << endl;
  }

  khtml::RenderPartObject *renderFrame = new khtml::RenderPartObject( w, this );
  m_render = renderFrame;
  m_render->setStyle(m_style);
  r->addChild( m_render, _next ? _next->renderer() : 0 );


  NodeBaseImpl::attach( w );
}

