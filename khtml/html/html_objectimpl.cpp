/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
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
#include "html/html_objectimpl.h"

#include "khtml_part.h"
#include "dom/dom_string.h"
#include "misc/htmlhashes.h"
#include "khtmlview.h"
#include <qstring.h>
#include <qvariant.h>
#include <qmap.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kimageio.h>

#include "xml/dom_docimpl.h"
#include "css/cssstyleselector.h"
#include "css/csshelper.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "rendering/render_applet.h"
#include "rendering/render_frames.h"
#include "rendering/render_image.h"
#include "xml/dom2_eventsimpl.h"

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------
HTMLObjectBaseElementImpl::HTMLObjectBaseElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
    needWidgetUpdate = false;
    m_renderAlternative = false;
}

void HTMLObjectBaseElementImpl::setServiceType(const QString & val) {
    serviceType = val.lower();
    int pos = serviceType.find( ";" );
    if ( pos!=-1 )
        serviceType.truncate( pos );
}

void HTMLObjectBaseElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch ( attr->id() )
    {
        case ATTR_TYPE:
        case ATTR_CODETYPE:
	    if (attr->val()) {
	      DOM::DOMStringImpl *stringImpl = attr->val();
	      QString val = QConstString( stringImpl->s, stringImpl->l ).string();
	      setServiceType( val );
              needWidgetUpdate = true;
	    }
            break;
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
        case ATTR_NAME:
            if (inDocument() && m_name != attr->value()) {
                getDocument()->underDocNamedCache().remove(m_name.string(),        this);
                getDocument()->underDocNamedCache().add   (attr->value().string(), this);
            }
            m_name = attr->value();
            //fallthrough
        default:
            HTMLElementImpl::parseAttribute( attr );
    }
}

void HTMLObjectBaseElementImpl::removedFromDocument()
{
    getDocument()->underDocNamedCache().remove(m_name.string(), this);
    HTMLElementImpl::removedFromDocument();
}

void HTMLObjectBaseElementImpl::insertedIntoDocument()
{
    getDocument()->underDocNamedCache().add(m_name.string(), this);
    HTMLElementImpl::insertedIntoDocument();
}

void HTMLObjectBaseElementImpl::removeId(const QString& id)
{
    getDocument()->underDocNamedCache().remove(id, this);
    HTMLElementImpl::removeId(id);
}

void HTMLObjectBaseElementImpl::addId   (const QString& id)
{
    getDocument()->underDocNamedCache().add(id, this);
    HTMLElementImpl::addId(id);
}

void HTMLObjectBaseElementImpl::recalcStyle( StyleChange ch )
{
    if (needWidgetUpdate) {
        if(m_render && strcmp( m_render->renderName(),  "RenderPartObject" ) == 0 )
            static_cast<RenderPartObject*>(m_render)->updateWidget();
        needWidgetUpdate = false;
    }
    HTMLElementImpl::recalcStyle( ch );
}

void HTMLObjectBaseElementImpl::renderAlternative()
{
    if ( m_renderAlternative ) return;
    QTimer::singleShot( 0, this, SLOT( slotRenderAlternative() ) );
}

void HTMLObjectBaseElementImpl::slotRenderAlternative()
{
    // an unbelievable hack. FIXME!!
    if ( m_renderAlternative ) return;

    // ### there can be a m_render if this is called from our attach indirectly
    if ( attached() || m_render)
        detach();

    m_renderAlternative = true;

    attach();
}

void HTMLObjectBaseElementImpl::attach() {
    assert(!attached());
    assert(!m_render);

    if (serviceType.isEmpty() && url.startsWith("data:")) {
        // Extract the MIME type from the data URL.
        int index = url.find(';');
        if (index == -1)
            index = url.find(',');
        if (index != -1) {
            int len = index - 5;
            if (len > 0)
                serviceType = url.mid(5, len);
            else
                serviceType = "text/plain"; // Data URLs with no MIME type are considered text/plain.
        }
    }

    bool imagelike = serviceType.startsWith("image/") &&
                   !KImageIO::typeForMime(serviceType).isNull();

    if (m_renderAlternative && !imagelike) {
        // render alternative content
        ElementImpl::attach();
        return;
    }

    if (!parentNode()->renderer()) {
        NodeBaseImpl::attach();
        return;
    }

    RenderStyle* _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();

    if (parentNode()->renderer() && parentNode()->renderer()->childAllowed() &&
        _style->display() != NONE)
    {
        needWidgetUpdate = false;

        if (imagelike) {
            m_render = new (getDocument()->renderArena()) RenderImage(this);
            // make sure we don't attach the inner contents
            addCSSProperty(CSS_PROP_DISPLAY, CSS_VAL_NONE);
        }
        else
            m_render = new (getDocument()->renderArena())RenderPartObject(this);

        m_render->setStyle(_style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
        if (imagelike)
            m_render->updateFromElement();
    }

    _style->deref();
    NodeBaseImpl::attach();

    // ### do this when we are actually finished loading instead
    if (m_render) dispatchHTMLEvent(EventImpl::LOAD_EVENT, false, false);
}


// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentPtr *doc)
  : HTMLObjectBaseElementImpl(doc)
{
    serviceType = "application/x-java-applet";
    needWidgetUpdate = true;
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
}

NodeImpl::Id HTMLAppletElementImpl::id() const
{
    return ID_APPLET;
}

void HTMLAppletElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch( attr->id() )
    {
    case ATTR_CODEBASE:
    case ATTR_ARCHIVE:
    case ATTR_CODE:
    case ATTR_OBJECT:
    case ATTR_ALT:
        break;
    case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
	break;
    case ATTR_VSPACE:
        addCSSLength(CSS_PROP_MARGIN_TOP, attr->value());
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value());
        break;
    case ATTR_HSPACE:
        addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value());
        addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value());
        break;
    case ATTR_VALIGN:
        addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower() );
        break;
    default:
        HTMLObjectBaseElementImpl::parseAttribute(attr);
    }
}

void HTMLAppletElementImpl::attach()
{
    KHTMLView* w = getDocument()->view();

#ifndef Q_WS_QWS // FIXME?
    DOMString codeBase = getAttribute( ATTR_CODEBASE );
    DOMString code = getAttribute( ATTR_CODE );
    if ( !codeBase.isEmpty() )
        url = codeBase.string();
    if ( !code.isEmpty() )
        url = code.string();

    if (!w || !w->part()->javaEnabled())
#endif
        m_renderAlternative = true;

    HTMLObjectBaseElementImpl::attach();
}

// -------------------------------------------------------------------------

HTMLEmbedElementImpl::HTMLEmbedElementImpl(DocumentPtr *doc)
    : HTMLObjectBaseElementImpl(doc)
{
}

HTMLEmbedElementImpl::~HTMLEmbedElementImpl()
{
}

NodeImpl::Id HTMLEmbedElementImpl::id() const
{
    return ID_EMBED;
}

void HTMLEmbedElementImpl::parseAttribute(AttributeImpl *attr)
{
  switch ( attr->id() )
  {
     case ATTR_CODE:
     case ATTR_SRC:
         url = khtml::parseURL(attr->val()).string();
         needWidgetUpdate = true;
         break;
     case ATTR_BORDER:
        addCSSLength(CSS_PROP_BORDER_WIDTH, attr->value());
        addCSSProperty( CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_SOLID );
        addCSSProperty( CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_SOLID );
        addCSSProperty( CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_SOLID );
        addCSSProperty( CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_SOLID );
        break;
     case ATTR_VSPACE:
        addCSSLength(CSS_PROP_MARGIN_TOP, attr->value());
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value());
        break;
     case ATTR_HSPACE:
        addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value());
        addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value());
        break;
     case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
	break;
     case ATTR_VALIGN:
        addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower() );
        break;
     case ATTR_PLUGINPAGE:
     case ATTR_PLUGINSPAGE: {
        pluginPage = attr->value().string();
        break;
      }
     case ATTR_HIDDEN:
        if (strcasecmp( attr->value(), "yes" ) == 0 || strcasecmp( attr->value() , "true") == 0 )
           hidden = true;
        else
           hidden = false;
        break;
     default:
        HTMLObjectBaseElementImpl::parseAttribute( attr );
  }
}

void HTMLEmbedElementImpl::attach()
{
    KHTMLView* w = getDocument()->view();

    if (!w || !w->part()->pluginsEnabled())
        m_renderAlternative = true;

    if (parentNode()->id() == ID_OBJECT)
        NodeBaseImpl::attach();
    else
        HTMLObjectBaseElementImpl::attach();
}

// -------------------------------------------------------------------------

HTMLObjectElementImpl::HTMLObjectElementImpl(DocumentPtr *doc)
    : HTMLObjectBaseElementImpl(doc)
{
}

HTMLObjectElementImpl::~HTMLObjectElementImpl()
{
}

NodeImpl::Id HTMLObjectElementImpl::id() const
{
    return ID_OBJECT;
}

HTMLFormElementImpl *HTMLObjectElementImpl::form() const
{
  return 0;
}

void HTMLObjectElementImpl::parseAttribute(AttributeImpl *attr)
{
  switch ( attr->id() )
  {
    case ATTR_DATA:
      url = khtml::parseURL( attr->val() ).string();
      needWidgetUpdate = true;
      break;
    case ATTR_CLASSID:
      classId = attr->value().string();
      needWidgetUpdate = true;
      break;
    case ATTR_ONLOAD: // ### support load/unload on object elements
        setHTMLEventListener(EventImpl::LOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onload", this));
        break;
    case ATTR_ONUNLOAD:
        setHTMLEventListener(EventImpl::UNLOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(), "onunload", this));
        break;
     case ATTR_VSPACE:
        addCSSLength(CSS_PROP_MARGIN_TOP, attr->value());
        addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value());
        break;
     case ATTR_HSPACE:
        addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value());
        addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value());
        break;
     case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
	break;
     case ATTR_VALIGN:
        addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower() );
        break;
    default:
      HTMLObjectBaseElementImpl::parseAttribute( attr );
  }
}

DocumentImpl* HTMLObjectElementImpl::contentDocument() const
{
    if ( !m_render ) return 0;
    if ( !m_render->isWidget() ) return 0;
    QWidget* widget = static_cast<RenderWidget*>( m_render )->widget();
    if( widget && qobject_cast<KHTMLView*>( widget ) )
        return static_cast<KHTMLView*>( widget )->part()->xmlDocImpl();
    return 0;
}

void HTMLObjectElementImpl::attach()
{
    KHTMLView* w = getDocument()->view();

    if (!w || !w->part()->pluginsEnabled())
        m_renderAlternative = true;

    HTMLObjectBaseElementImpl::attach();
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLParamElementImpl::id() const
{
    return ID_PARAM;
}

void HTMLParamElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch( attr->id() )
    {
    case ATTR_VALUE:
        m_value = attr->value().string();
        break;
    case ATTR_ID:
        if (getDocument()->htmlMode() != DocumentImpl::XHtml) break;
        // fall through
    case ATTR_NAME:
        m_name = attr->value().string();
        // fall through
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

#include "html_objectimpl.moc"
