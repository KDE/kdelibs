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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
    : HTMLElementImpl(doc), liveconnect(0L)
{
    needWidgetUpdate = false;
    m_renderAlternative = false;
}

void HTMLObjectBaseElementImpl::setServiceType(const QString & val) {
    serviceType = val.lower();
    int pos = serviceType.find( ";" );
    if ( pos!=-1 )
        serviceType.truncate( pos );
    pos = serviceType.find( "-plugin" );
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
            addCSSLength( CSS_PROP_WIDTH, attr->value());
            break;
        case ATTR_HEIGHT:
            addCSSLength( CSS_PROP_HEIGHT, attr->value());
            break;
        default:
            HTMLElementImpl::parseAttribute( attr );
    }
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

bool HTMLObjectBaseElementImpl::get(const unsigned long objid, const QString & field, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->get(objid, field, type, retobjid, value);
}

bool HTMLObjectBaseElementImpl::put(const unsigned long objid, const QString & field, const QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->put(objid, field, value);
}

bool HTMLObjectBaseElementImpl::call(const unsigned long objid, const QString & func, const QStringList & args, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->call(objid, func, args, type, retobjid, value);
}

void HTMLObjectBaseElementImpl::unregister(const unsigned long objid) {
    if (!liveconnect)
        return;
    liveconnect->unregister(objid);
}

void HTMLObjectBaseElementImpl::setLiveConnect(KParts::LiveConnectExtension * lc) {
    liveconnect = lc;
    if (lc)
        connect(lc, SIGNAL(partEvent(const unsigned long, const QString &, const KParts::LiveConnectExtension::ArgList &)), static_cast<HTMLObjectBaseElementImpl*>(this), SLOT(liveConnectEvent( const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList &)));
}

void HTMLObjectBaseElementImpl::liveConnectEvent(const unsigned long, const QString & event, const KParts::LiveConnectExtension::ArgList & args)
{
    if (!liveconnect)
        // not attached
        return;

    QString script;
    script.sprintf("%s(", event.latin1());
    KParts::LiveConnectExtension::ArgList::const_iterator i = args.begin();
    for ( ; i != args.end(); i++) {
        if (i != args.begin())
            script += ",";
        if ((*i).first == KParts::LiveConnectExtension::TypeString) {
            script += "\"";
            script += QString((*i).second).replace('\\', "\\\\").replace('"', "\\\"");
            script += "\"";
        } else
            script += (*i).second;
    }
    script += ")";

    kdDebug(6036) << "HTMLObjectBaseElementImpl::liveConnectEvent " << script << endl;
    KHTMLView* w = getDocument()->view();
    if (w)
	w->part()->executeScript(this, script);
}

void HTMLObjectBaseElementImpl::attach() {
    assert(!attached());
    assert(!m_render);

    if (m_renderAlternative) {
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
        bool imagelike = serviceType.startsWith("image/") &&
                         (KImageIO::typeForMime(serviceType) != QString::null);
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

void HTMLObjectBaseElementImpl::detach() {
    setLiveConnect(0L);

    if (attached())
        // ### do this when we are actualy removed from document instead
        dispatchHTMLEvent(EventImpl::UNLOAD_EVENT,false,false);

    HTMLElementImpl::detach();
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
    case ATTR_ID:
    case ATTR_NAME:
        break;
    case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
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
        addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value());
        break;
     case ATTR_PLUGINPAGE:
     case ATTR_PLUGINSPAGE: {
        pluginPage = attr->value().string();
        break;
      }
     case ATTR_HIDDEN:
        if (attr->value().lower()=="yes" || attr->value().lower()=="true")
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
	    getDocument()->createHTMLEventListener(attr->value().string(),"onload"));
        break;
    case ATTR_ONUNLOAD:
        setHTMLEventListener(EventImpl::UNLOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string(),"onunload"));
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
    if( widget && ::qt_cast<KHTMLView*>( widget ) )
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
    case ATTR_ID:
        if (getDocument()->htmlMode() != DocumentImpl::XHtml) break;
        // fall through
    case ATTR_NAME:
        m_name = attr->value().string();
        break;
    case ATTR_VALUE:
        m_value = attr->value().string();
        break;
    }
}

#include "html_objectimpl.moc"
