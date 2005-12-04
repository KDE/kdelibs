/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann (hausmann@kde.org)
 *           (C) 2001-2003 Dirk Mueller (mueller@kde.org)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
// -------------------------------------------------------------------------

#include "html/html_baseimpl.h"
#include "html/html_documentimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"

#include "rendering/render_frames.h"
#include "rendering/render_body.h"
#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "css/csshelper.h"
#include "misc/loader.h"
#include "misc/htmlhashes.h"
#include "dom/dom_string.h"
#include "dom/dom_doc.h"
#include "xml/dom2_eventsimpl.h"

#include <kurl.h>
#include <kdebug.h>

using namespace DOM;
using namespace khtml;

HTMLBodyElementImpl::HTMLBodyElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc),
    m_bgSet( false ), m_fgSet( false )
{
    m_styleSheet = 0;
}

HTMLBodyElementImpl::~HTMLBodyElementImpl()
{
    if(m_styleSheet) m_styleSheet->deref();
}

NodeImpl::Id HTMLBodyElementImpl::id() const
{
    return ID_BODY;
}

void HTMLBodyElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {

    case ATTR_BACKGROUND:
    {
        QString url = khtml::parseURL( attr->value() ).string();
        if (!url.isEmpty()) {
            url = getDocument()->completeURL( url );
            addCSSProperty(CSS_PROP_BACKGROUND_IMAGE, "url('"+url+"')" );
            m_bgSet = true;
        }
        else {
             removeCSSProperty(CSS_PROP_BACKGROUND_IMAGE);
             m_bgSet = false;
         }
        break;
    }
    case ATTR_MARGINWIDTH: {
	KHTMLView* w = getDocument()->view();
	if (w)
	    w->setMarginWidth( -1 ); // unset this, so it doesn't override the setting here
        addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value() );
    }
        /* nobreak; */
    case ATTR_LEFTMARGIN:
        addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value() );
        break;
    case ATTR_MARGINHEIGHT: {
	KHTMLView* w = getDocument()->view();
	if (w)
	    w->setMarginHeight( -1 ); // unset this, so it doesn't override the setting here
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value());
    }
        /* nobreak */
    case ATTR_TOPMARGIN:
        addCSSLength(CSS_PROP_MARGIN_TOP, attr->value());
        break;
    case ATTR_BGCOLOR:
        addHTMLColor(CSS_PROP_BACKGROUND_COLOR, attr->value());
        m_bgSet = !attr->value().isNull();
        break;
    case ATTR_TEXT:
        addHTMLColor(CSS_PROP_COLOR, attr->value());
        m_fgSet = !attr->value().isNull();
        break;
    case ATTR_BGPROPERTIES:
        if ( strcasecmp( attr->value(), "fixed" ) == 0)
            addCSSProperty(CSS_PROP_BACKGROUND_ATTACHMENT, CSS_VAL_FIXED);
        break;
    case ATTR_VLINK:
    case ATTR_ALINK:
    case ATTR_LINK:
    {
        if(!m_styleSheet) {
            m_styleSheet = new CSSStyleSheetImpl(this,DOMString(),true);
            m_styleSheet->ref();
        }
        QString aStr;
	if ( attr->id() == ATTR_LINK )
	    aStr = "a:link";
	else if ( attr->id() == ATTR_VLINK )
	    aStr = "a:visited";
	else if ( attr->id() == ATTR_ALINK )
	    aStr = "a:active";
	aStr += " { color: " + attr->value().string() + "; }";
        m_styleSheet->parseString(aStr, !getDocument()->inCompatMode());
        m_styleSheet->setNonCSSHints();
        if (attached())
            getDocument()->updateStyleSelector();
        break;
    }
    case ATTR_ONLOAD:
        getDocument()->setHTMLWindowEventListener(EventImpl::LOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onload", NULL));
        break;
    case ATTR_ONUNLOAD:
        getDocument()->setHTMLWindowEventListener(EventImpl::UNLOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onunload", NULL));
        break;
    case ATTR_ONBLUR:
        getDocument()->setHTMLWindowEventListener(EventImpl::BLUR_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onblur", NULL));
        break;
    case ATTR_ONFOCUS:
        getDocument()->setHTMLWindowEventListener(EventImpl::FOCUS_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onfocus", NULL));
        break;
    case ATTR_ONRESIZE:
        getDocument()->setHTMLWindowEventListener(EventImpl::RESIZE_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onresize", NULL));
        break;
    case ATTR_NOSAVE:
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLBodyElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();

    KHTMLView* w = getDocument()->view();
    if(w && w->marginWidth() != -1) {
        QString s;
        s.sprintf( "%d", w->marginWidth() );
        addCSSLength(CSS_PROP_MARGIN_LEFT, s);
        addCSSLength(CSS_PROP_MARGIN_RIGHT, s);
    }
    if(w && w->marginHeight() != -1) {
        QString s;
        s.sprintf( "%d", w->marginHeight() );
        addCSSLength(CSS_PROP_MARGIN_TOP, s);
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, s);
    }

    if ( m_bgSet && !m_fgSet )
        addCSSProperty(CSS_PROP_COLOR, CSS_VAL_BLACK);

    if (m_styleSheet)
        getDocument()->updateStyleSelector();
}

void HTMLBodyElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();

    if (m_styleSheet)
        getDocument()->updateStyleSelector();
}

void HTMLBodyElementImpl::attach()
{
    assert(!m_render);
    assert(parentNode());

    RenderStyle* style = getDocument()->styleSelector()->styleForElement(this);
    style->ref();
    if (parentNode()->renderer() && style->display() != NONE) {
        if (style->display() == BLOCK)
            // only use the quirky class for block display
            m_render = new (getDocument()->renderArena()) RenderBody(this);
        else
            m_render = RenderObject::createObject(this, style);
        m_render->setStyle(style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
    }
    style->deref();

    NodeBaseImpl::attach();
}

// -------------------------------------------------------------------------

HTMLFrameElementImpl::HTMLFrameElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
    frameBorder = true;
    frameBorderSet = false;
    marginWidth = -1;
    marginHeight = -1;
    scrolling = QScrollView::Auto;
    noresize = false;
    url = "about:blank";
}

HTMLFrameElementImpl::~HTMLFrameElementImpl()
{
}

NodeImpl::Id HTMLFrameElementImpl::id() const
{
    return ID_FRAME;
}

void HTMLFrameElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_SRC:
        setLocation(khtml::parseURL(attr->val()));
        break;
    case ATTR_FRAMEBORDER:
    {
        frameBorder = attr->value().toInt();
        frameBorderSet = ( attr->val() != 0 );
        // FIXME: when attached, has no effect
    }
    break;
    case ATTR_MARGINWIDTH:
        marginWidth = attr->val()->toInt();
        // FIXME: when attached, has no effect
        break;
    case ATTR_MARGINHEIGHT:
        marginHeight = attr->val()->toInt();
        // FIXME: when attached, has no effect
        break;
    case ATTR_NORESIZE:
        noresize = true;
        // FIXME: when attached, has no effect
        break;
    case ATTR_SCROLLING:
        if( strcasecmp( attr->value(), "auto" ) == 0 )
            scrolling = QScrollView::Auto;
        else if( strcasecmp( attr->value(), "yes" ) == 0 )
            scrolling = QScrollView::AlwaysOn;
        else if( strcasecmp( attr->value(), "no" ) == 0 )
            scrolling = QScrollView::AlwaysOff;
        // when attached, has no effect
        break;
    case ATTR_ONLOAD:
        setHTMLEventListener(EventImpl::LOAD_EVENT,
            getDocument()->createHTMLEventListener(attr->value().string(), "onload", this));
        break;
    case ATTR_ONUNLOAD:
        setHTMLEventListener(EventImpl::UNLOAD_EVENT,
            getDocument()->createHTMLEventListener(attr->value().string(), "onunload", this));
        break;
    case ATTR_ID:
    case ATTR_NAME:
        // FIXME: if already attached, doesn't change the frame name
        // FIXME: frame name conflicts, no unique frame name anymore
        name = attr->value();
        //fallthrough intentional, let the base handle it
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameElementImpl::attach()
{
    assert(!attached());
    assert(parentNode());

    name = getAttribute(ATTR_NAME);
    if (name.isNull())
        name = getAttribute(ATTR_ID);

    // inherit default settings from parent frameset
    HTMLElementImpl* node = static_cast<HTMLElementImpl*>(parentNode());
    while(node)
    {
        if(node->id() == ID_FRAMESET)
        {
            HTMLFrameSetElementImpl* frameset = static_cast<HTMLFrameSetElementImpl*>(node);
            if(!frameBorderSet)  frameBorder = frameset->frameBorder();
            if(!noresize)  noresize = frameset->noResize();
            break;
        }
        node = static_cast<HTMLElementImpl*>(node->parentNode());
    }

    if (parentNode()->renderer() && getDocument()->isURLAllowed(url.string()))  {
        RenderStyle* _style = getDocument()->styleSelector()->styleForElement(this);
        _style->ref();
        if ( _style->display() != NONE ) {
            m_render = new (getDocument()->renderArena()) RenderFrame(this);
            m_render->setStyle(_style);
            parentNode()->renderer()->addChild(m_render, nextRenderer());
        }
        _style->deref();
    }

    NodeBaseImpl::attach();

    if (!m_render)
        return;

    KHTMLView* w = getDocument()->view();
    if (w) {
	// we need a unique name for every frame in the frameset. Hope that's unique enough.
	if(name.isEmpty() || w->part()->frameExists( name.string() ) )
	    name = DOMString(w->part()->requestFrameName());

	// load the frame contents
	w->part()->requestFrame( static_cast<RenderFrame*>(m_render), url.string(), name.string() );
    }
}

void HTMLFrameElementImpl::setLocation( const DOMString& str )
{
    if ( url == str )
        return;

    url = str;

    if( !attached() )
        return;

    if( !m_render ) {
        detach();
        attach();
        return;
    }

    if( !getDocument()->isURLAllowed(url.string()) )
        return;

    // load the frame contents
    KHTMLView *w = getDocument()->view();
    if (w) {
	KHTMLPart *part = w->part()->findFrame(  name.string() );
	if ( part ) {
	    part->openURL( KURL( getDocument()->completeURL( url.string() ) ) );
	} else {
	    w->part()->requestFrame( static_cast<RenderFrame*>( m_render ), url.string(), name.string() );
	}
    }
}

bool HTMLFrameElementImpl::isFocusable() const
{
    return m_render!=0;
}

void HTMLFrameElementImpl::setFocus(bool received)
{
    HTMLElementImpl::setFocus(received);
    khtml::RenderFrame *renderFrame = static_cast<khtml::RenderFrame *>(m_render);
    if (!renderFrame || !renderFrame->widget())
	return;
    if (received)
	renderFrame->widget()->setFocus();
    else
	renderFrame->widget()->clearFocus();
}

DocumentImpl* HTMLFrameElementImpl::contentDocument() const
{
    if ( !m_render ) return 0;

    RenderPart* render = static_cast<RenderPart*>( m_render );

    if(render->widget() && ::qt_cast<KHTMLView*>( render->widget()) )
        return static_cast<KHTMLView*>( render->widget() )->part()->xmlDocImpl();

    return 0;
}

// -------------------------------------------------------------------------

HTMLFrameSetElementImpl::HTMLFrameSetElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
    // default value for rows and cols...
    m_totalRows = 1;
    m_totalCols = 1;

    m_rows = m_cols = 0;

    frameborder = true;
    frameBorderSet = false;
    m_border = 4;
    noresize = false;

    m_resizing = false;
}

HTMLFrameSetElementImpl::~HTMLFrameSetElementImpl()
{
    delete [] m_rows;
    delete [] m_cols;
}

NodeImpl::Id HTMLFrameSetElementImpl::id() const
{
    return ID_FRAMESET;
}

void HTMLFrameSetElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ROWS:
        if (!attr->val()) break;
        delete [] m_rows;
        m_rows = attr->val()->toLengthArray(m_totalRows);
        setChanged();
    break;
    case ATTR_COLS:
        if (!attr->val()) break;
        delete [] m_cols;
        m_cols = attr->val()->toLengthArray(m_totalCols);
        setChanged();
    break;
    case ATTR_FRAMEBORDER:
        // false or "no" or "0"..
        if ( attr->value().toInt() == 0 ) {
            frameborder = false;
            m_border = 0;
        }
        frameBorderSet = true;
        break;
    case ATTR_NORESIZE:
        noresize = true;
        break;
    case ATTR_BORDER:
        m_border = attr->val()->toInt();
        if(!m_border)
            frameborder = false;
        break;
    case ATTR_ONLOAD:
        getDocument()->setHTMLEventListener(EventImpl::LOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onload", this));
        break;
    case ATTR_ONUNLOAD:
        getDocument()->setHTMLEventListener(EventImpl::UNLOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onunload", this));
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameSetElementImpl::attach()
{
    assert(!attached() );
    assert(parentNode());

    // inherit default settings from parent frameset
    HTMLElementImpl* node = static_cast<HTMLElementImpl*>(parentNode());
    while(node)
    {
        if(node->id() == ID_FRAMESET)
        {
            HTMLFrameSetElementImpl* frameset = static_cast<HTMLFrameSetElementImpl*>(node);
            if(!frameBorderSet)  frameborder = frameset->frameBorder();
            if(!noresize)  noresize = frameset->noResize();
            break;
        }
        node = static_cast<HTMLElementImpl*>(node->parentNode());
    }

    // ignore display: none
    if ( parentNode()->renderer() ) {
        m_render = new (getDocument()->renderArena()) RenderFrameSet(this);
        m_render->setStyle(getDocument()->styleSelector()->styleForElement(this));
       parentNode()->renderer()->addChild(m_render, nextRenderer());
    }

    NodeBaseImpl::attach();
}

void HTMLFrameSetElementImpl::defaultEventHandler(EventImpl *evt)
{
    if (evt->isMouseEvent() && !noresize && m_render)
        static_cast<khtml::RenderFrameSet *>(m_render)->userResize(static_cast<MouseEventImpl*>(evt));

    evt->setDefaultHandled();
    HTMLElementImpl::defaultEventHandler(evt);
}

void HTMLFrameSetElementImpl::detach()
{
    if(attached())
        // ### send the event when we actually get removed from the doc instead of here
        getDocument()->dispatchHTMLEvent(EventImpl::UNLOAD_EVENT,false,false);

    HTMLElementImpl::detach();
}

void HTMLFrameSetElementImpl::recalcStyle( StyleChange ch )
{
    if (changed() && m_render) {
        m_render->setNeedsLayout(true);
//         m_render->layout();
        setChanged(false);
    }
    HTMLElementImpl::recalcStyle( ch );
}


// -------------------------------------------------------------------------

NodeImpl::Id HTMLHeadElementImpl::id() const
{
    return ID_HEAD;
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLHtmlElementImpl::id() const
{
    return ID_HTML;
}


// -------------------------------------------------------------------------

HTMLIFrameElementImpl::HTMLIFrameElementImpl(DocumentPtr *doc) : HTMLFrameElementImpl(doc)
{
    frameBorder = false;
    marginWidth = 0;
    marginHeight = 0;
    needWidgetUpdate = false;
}

HTMLIFrameElementImpl::~HTMLIFrameElementImpl()
{
}

NodeImpl::Id HTMLIFrameElementImpl::id() const
{
    return ID_IFRAME;
}

void HTMLIFrameElementImpl::parseAttribute(AttributeImpl *attr )
{
    switch (  attr->id() )
    {
    case ATTR_WIDTH:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_WIDTH, attr->value());
        else
            removeCSSProperty(CSS_PROP_WIDTH);
        break;
    case ATTR_HEIGHT:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_HEIGHT, attr->value());
        else
            removeCSSProperty(CSS_PROP_HEIGHT);
        break;
    case ATTR_SRC:
        needWidgetUpdate = true; // ### do this for scrolling, margins etc?
        HTMLFrameElementImpl::parseAttribute( attr );
        break;
    default:
        HTMLFrameElementImpl::parseAttribute( attr );
    }
}

void HTMLIFrameElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    name = getAttribute(ATTR_NAME);
    if (name.isNull())
        name = getAttribute(ATTR_ID);

    RenderStyle* style = getDocument()->styleSelector()->styleForElement(this);
    style->ref();
    if (getDocument()->isURLAllowed(url.string()) &&
        parentNode()->renderer() && style->display() != NONE) {
        m_render = new (getDocument()->renderArena()) RenderPartObject(this);
        m_render->setStyle(style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
    }
    style->deref();

    NodeBaseImpl::attach();

    if (m_render) {
        // we need a unique name for every frame in the frameset. Hope that's unique enough.
        KHTMLView* w = getDocument()->view();
        if(w && (name.isEmpty() || w->part()->frameExists( name.string() )))
            name = DOMString(w->part()->requestFrameName());

        static_cast<RenderPartObject*>(m_render)->updateWidget();
        needWidgetUpdate = false;
    }
}

void HTMLIFrameElementImpl::recalcStyle( StyleChange ch )
{
    if (needWidgetUpdate) {
        if(m_render)  static_cast<RenderPartObject*>(m_render)->updateWidget();
        needWidgetUpdate = false;
    }
    HTMLElementImpl::recalcStyle( ch );
}

