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

#include "xml/dom_docimpl.h"
#include "css/cssstyleselector.h"
#include "css/csshelper.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "rendering/render_applet.h"
#include "rendering/render_frames.h"
#include "rendering/render_image.h"
#include "xml/dom2_eventsimpl.h"

#ifndef Q_WS_QWS // We don't have Java in Qt Embedded
#include "java/kjavaappletwidget.h"
#include "java/kjavaappletcontext.h"
#endif

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------
LiveConnectElementImpl::LiveConnectElementImpl(DocumentPtr *doc)
  : HTMLElementImpl(doc), liveconnect(0L), timer (new QTimer(this)) {
    connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));
}

bool LiveConnectElementImpl::get(const unsigned long objid, const QString & field, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->get(objid, field, type, retobjid, value);
}

bool LiveConnectElementImpl::put(const unsigned long objid, const QString & field, const QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->put(objid, field, value);
}

bool LiveConnectElementImpl::call(const unsigned long objid, const QString & func, const QStringList & args, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value) {
    if (!liveconnect)
        return false;
    return liveconnect->call(objid, func, args, type, retobjid, value);
}

void LiveConnectElementImpl::unregister(const unsigned long objid) {
    if (!liveconnect)
        return;
    liveconnect->unregister(objid);
}

void LiveConnectElementImpl::setLiveConnect(KParts::LiveConnectExtension * lc) {
    liveconnect = lc;
    if (lc)
        connect(lc, SIGNAL(partEvent(const unsigned long, const QString &, const KParts::LiveConnectExtension::ArgList &)), static_cast<LiveConnectElementImpl*>(this), SLOT(liveConnectEvent( const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList &)));
}

void LiveConnectElementImpl::liveConnectEvent(const unsigned long, const QString & event, const KParts::LiveConnectExtension::ArgList & args)
{
    if (!liveconnect)
        // not attached
        return;

    if (timer->isActive()) {
        timerDone();
        timer->stop();
    }
    script.sprintf("%s(", event.latin1());
    KParts::LiveConnectExtension::ArgList::const_iterator i = args.begin();
    for ( ; i != args.end(); i++) {
        if (i != args.begin())
            script += ",";
        if ((*i).first == KParts::LiveConnectExtension::TypeString) {
            script += "\"";
            script += (*i).second;
            script += "\"";
        } else
            script += (*i).second;
    }
    script += ")";

    kdDebug(6036) << "LiveConnectElementImpl::liveConnectEvent " << script << endl;
    KHTMLView* w = getDocument()->view();
    w->part()->executeScript(this, script);
    //timer->start(0, true);
}

void LiveConnectElementImpl::timerDone() {
    KHTMLView* w = getDocument()->view();
    w->part()->executeScript(this, script);
}

void LiveConnectElementImpl::detach() {
    if (timer->isActive())
        timer->stop();
    setLiveConnect(0L);

    HTMLElementImpl::detach();
}

// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentPtr *doc)
  : LiveConnectElementImpl(doc)
{
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
}

NodeImpl::Id HTMLAppletElementImpl::id() const
{
    return ID_APPLET;
}

KJavaApplet* HTMLAppletElementImpl::applet() const
{
#ifndef Q_WS_QWS // We don't have Java in Qt Embedded
    if (!m_render || !m_render->isApplet())
        return 0L;

    return static_cast<KJavaAppletWidget*>(static_cast<RenderApplet*>(m_render)->widget())->applet();
#else
    return 0;
#endif
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
    case ATTR_WIDTH:
        addCSSLength(CSS_PROP_WIDTH, attr->value());
        break;
    case ATTR_HEIGHT:
        addCSSLength(CSS_PROP_HEIGHT, attr->value());
        break;
    case ATTR_ALIGN:
	addHTMLAlignment( attr->value() );
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLAppletElementImpl::attach()
{
    if (!parentNode()->renderer() || getAttribute(ATTR_CODE).isNull()) {
        NodeBaseImpl::attach();
        return;
    }

    KHTMLView *view = getDocument()->view();

#ifndef Q_WS_QWS // FIXME?
    KURL url = getDocument()->baseURL();
    DOMString codeBase = getAttribute( ATTR_CODEBASE );
    DOMString code = getAttribute( ATTR_CODE );
    if ( !codeBase.isEmpty() )
        url = KURL( url, codeBase.string() );
    if ( !code.isEmpty() )
        url = KURL( url, code.string() );

    if( view->part()->javaEnabled() && isURLAllowed( url.url() ) )
    {
	QMap<QString, QString> args;

	args.insert( "code", code.string());
	if(!codeBase.isNull())
	    args.insert( "codeBase", codeBase.string() );
	DOMString name = getDocument()->htmlMode() != DocumentImpl::XHtml ?
			 getAttribute(ATTR_NAME) : getAttribute(ATTR_ID);
        if (name.isNull())
            setAttribute(ATTR_ID, code.string());
        else
	    args.insert( "name", name.string() );
	DOMString archive = getAttribute(ATTR_ARCHIVE);
	if(!archive.isNull())
	    args.insert( "archive", archive.string() );

	args.insert( "baseURL", getDocument()->baseURL() );
        m_render = new RenderApplet(this, args);
        setLiveConnect(applet()->getLiveConnectExtension());

        m_render->setStyle(getDocument()->styleSelector()->styleForElement(this));
        parentNode()->renderer()->addChild(m_render, nextRenderer());
        NodeBaseImpl::attach();
    }
    else
#endif
        ElementImpl::attach();
}

// -------------------------------------------------------------------------

HTMLEmbedElementImpl::HTMLEmbedElementImpl(DocumentPtr *doc)
    : LiveConnectElementImpl(doc)
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
  DOM::DOMStringImpl *stringImpl = attr->val();
  QConstString cv( stringImpl->s, stringImpl->l );
  QString val = cv.string();

  int pos;
  switch ( attr->id() )
  {
     case ATTR_TYPE:
        serviceType = val.lower();
        pos = serviceType.find( ";" );
        if ( pos!=-1 )
            serviceType = serviceType.left( pos );
        serviceType.truncate( serviceType.find( "-plugin" ) );
        break;
     case ATTR_CODE:
     case ATTR_SRC:
         url = khtml::parseURL(attr->val()).string();
         break;
     case ATTR_WIDTH:
        addCSSLength( CSS_PROP_WIDTH, attr->value() );
        break;
     case ATTR_HEIGHT:
        addCSSLength( CSS_PROP_HEIGHT, attr->value());
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
     case ATTR_PLUGINSPAGE:
        pluginPage = val;
        break;
     case ATTR_HIDDEN:
        if (val.lower()=="yes" || val.lower()=="true")
           hidden = true;
        else
           hidden = false;
        break;
     default:
        HTMLElementImpl::parseAttribute( attr );
  }
}

void HTMLEmbedElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);

    if (parentNode()->renderer()) {
        KHTMLView* w = getDocument()->view();
        RenderStyle* _style = getDocument()->styleSelector()->styleForElement( this );
        _style->ref();

        if (w->part()->pluginsEnabled() && isURLAllowed( url ) &&
            parentNode()->id() != ID_OBJECT && _style->display() != NONE ) {
            m_render = new RenderPartObject(this);
            m_render->setStyle(_style );
            parentNode()->renderer()->addChild(m_render, nextRenderer());
            static_cast<RenderPartObject*>(m_render)->updateWidget();
            setLiveConnect(w->part()->liveConnectExtension(static_cast<RenderPartObject*>(m_render)));
        }
        _style->deref();
    }

    NodeBaseImpl::attach();
}

// -------------------------------------------------------------------------

HTMLObjectElementImpl::HTMLObjectElementImpl(DocumentPtr *doc)
    : LiveConnectElementImpl(doc)
{
    needWidgetUpdate = false;
    m_renderAlternative = false;
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
  DOM::DOMStringImpl *stringImpl = attr->val();
  QString val = QConstString( stringImpl->s, stringImpl->l ).string();
  int pos;
  switch ( attr->id() )
  {
    case ATTR_TYPE:
    case ATTR_CODETYPE:
      serviceType = val.lower();
      pos = serviceType.find( ";" );
      if ( pos!=-1 )
          serviceType = serviceType.left( pos );
      serviceType.truncate( serviceType.find( "-plugin" ) );
      needWidgetUpdate = true;
      break;
    case ATTR_DATA:
      url = khtml::parseURL( val ).string();
      needWidgetUpdate = true;
      break;
    case ATTR_WIDTH:
      addCSSLength( CSS_PROP_WIDTH, attr->value());
      break;
    case ATTR_HEIGHT:
      addCSSLength( CSS_PROP_HEIGHT, attr->value());
      break;
    case ATTR_CLASSID:
      classId = val;
      needWidgetUpdate = true;
      break;
    case ATTR_ONLOAD: // ### support load/unload on object elements
        setHTMLEventListener(EventImpl::LOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONUNLOAD:
        setHTMLEventListener(EventImpl::UNLOAD_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    default:
      HTMLElementImpl::parseAttribute( attr );
  }
}

DocumentImpl* HTMLObjectElementImpl::contentDocument() const
{
    if ( !m_render ) return 0;
    if ( !m_render->isWidget() ) return 0;
    QWidget* widget = static_cast<RenderWidget*>( m_render )->widget();
    if( widget && widget->inherits("KHTMLView") )
        return static_cast<KHTMLView*>( widget )->part()->xmlDocImpl();
    return 0;
}

void HTMLObjectElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);

    KHTMLView* w = getDocument()->view();
    if ( !w->part()->pluginsEnabled() ||
         ( url.isEmpty() && classId.isEmpty() ) ||
         m_renderAlternative || !isURLAllowed( url ) ) {
        // render alternative content
        ElementImpl::attach();
        return;
    }

    RenderStyle* _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();

    if (parentNode()->renderer() && _style->display() != NONE) {
        needWidgetUpdate=false;
        bool imagelike = serviceType.startsWith("image/");
        if (imagelike)
            m_render = new RenderImage(this);
        else
            m_render = new RenderPartObject(this);

        m_render->setStyle(_style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
        if (imagelike)
            m_render->updateFromElement();
    }

    _style->deref();

    NodeBaseImpl::attach();

    // ### do this when we are actually finished loading instead
    if (m_render)  dispatchHTMLEvent(EventImpl::LOAD_EVENT,false,false);
}

void HTMLObjectElementImpl::detach()
{
    if (attached())
        // ### do this when we are actualy removed from document instead
        dispatchHTMLEvent(EventImpl::UNLOAD_EVENT,false,false);

  HTMLElementImpl::detach();
}

void HTMLObjectElementImpl::renderAlternative()
{
    // an unbelievable hack. FIXME!!

    if ( m_renderAlternative ) return;

    if ( attached() )
        detach();

    m_renderAlternative = true;

    attach();
}

void HTMLObjectElementImpl::recalcStyle( StyleChange ch )
{
    if (needWidgetUpdate) {
        if(m_render && strcmp( m_render->renderName(),  "RenderPartObject" ) == 0 )
            static_cast<RenderPartObject*>(m_render)->updateWidget();
        needWidgetUpdate = false;
    }
    HTMLElementImpl::recalcStyle( ch );
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
