/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2003 Apple Computer, Inc.
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

#include "html/html_headimpl.h"
#include "html/html_documentimpl.h"
#include "xml/dom_textimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"

#include "misc/htmlhashes.h"
#include "misc/loader.h"
#include "misc/helper.h"

#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include "css/csshelper.h"

#include "ecma/kjs_proxy.h"

#include <kurl.h>
#include <kdebug.h>

using namespace khtml;

NodeImpl::Id HTMLBaseElementImpl::id() const
{
    return ID_BASE;
}

void HTMLBaseElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_HREF:
	m_href = khtml::parseURL(attr->value());
	process();
	break;
    case ATTR_TARGET:
	m_target = attr->value();
	process();
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLBaseElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();
    process();
}

void HTMLBaseElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();

    // Since the document doesn't have a base element...
    // (This will break in the case of multiple base elements, but that's not valid anyway (?))
    getDocument()->setBaseURL( KUrl() );
    getDocument()->setBaseTarget( QString() );
}

void HTMLBaseElementImpl::process()
{
    if (!inDocument())
	return;

    if(!m_href.isEmpty() && getDocument()->view())
	getDocument()->setBaseURL( KUrl( getDocument()->view()->part()->url(), m_href.string() ) );

    if(!m_target.isEmpty())
	getDocument()->setBaseTarget( m_target.string() );

    // ### should changing a document's base URL dynamically automatically update all images, stylesheets etc?
}

// -------------------------------------------------------------------------


HTMLLinkElementImpl::~HTMLLinkElementImpl()
{
    if(m_sheet) m_sheet->deref();
    if(m_cachedSheet) m_cachedSheet->deref(this);
}

NodeImpl::Id HTMLLinkElementImpl::id() const
{
    return ID_LINK;
}

void HTMLLinkElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch (attr->id())
    {
    case ATTR_HREF:
        m_url = getDocument()->completeURL( khtml::parseURL(attr->value()).string() );
	process();
        break;
    case ATTR_REL:
    case ATTR_TYPE:
	process();
        break;
    case ATTR_TITLE:
        // ### when title changes we have to reconsider our alternative
        // stylesheet choice
        break;
    case ATTR_MEDIA:
        m_media = attr->value().string().lower();
        process();
        break;
    case ATTR_DISABLED: {
        bool m_oldisDisabled = m_isDisabled;
        m_isDisabled = attr->val();
        if (m_oldisDisabled != m_isDisabled) {
            if (isLoading()) {
                if (m_oldisDisabled)
                    getDocument()->addPendingSheet();
                else if (!m_alternate)
                    getDocument()->styleSheetLoaded();
            }
            if (m_oldisDisabled) {
                // enabling: if it's an alternate sheet, pretend it's not.
                m_alternate = false;
            } else if (!m_alternate) {
                // disabling: recheck alternate status
                QString rel =  getAttribute(ATTR_REL).string().lower();
                QString type = getAttribute(ATTR_TYPE).string().lower();
                m_alternate = (type.contains("text/css") || rel.contains("stylesheet")) && rel.contains("alternate");
            }
            if (isLoading())
                break;
            if (!m_sheet && !m_isDisabled) {
                process();
                if (isLoading() && m_alternate)
                    getDocument()->addPendingSheet();
                m_alternate = false;
            } else
                getDocument()->updateStyleSelector(); // Update the style selector.
        }
        break;
    }
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLLinkElementImpl::process()
{
    if (!inDocument())
        return;

    QString type = getAttribute(ATTR_TYPE).string().lower();
    QString rel = getAttribute(ATTR_REL).string().lower();

    KHTMLPart* part = getDocument()->view() ? getDocument()->view()->part() : 0;

    // IE extension: location of small icon for locationbar / bookmarks
    // Uses both "shortcut icon" and "icon"
    if ( part && rel.contains("icon") && !m_url.isEmpty() && !part->parentPart())
        part->browserExtension()->setIconURL( KUrl(m_url.string()) );

    // Stylesheet
    else if (!m_isDisabled &&
        (type.contains("text/css") || rel.contains("stylesheet"))) {
        // no need to load style sheets which aren't for the screen output
        // ### there may be in some situations e.g. for an editor or script to manipulate
        if( m_media.isNull() || m_media.contains("screen") || m_media.contains("all") || m_media.contains("print") ) {
            m_loading = true;
            // Add ourselves as a pending sheet, but only if we aren't an alternate
            // stylesheet.  Alternate stylesheets don't hold up render tree construction.
            m_alternate = rel.contains("alternate");
            if (!isAlternate())
                getDocument()->addPendingSheet();

            QString chset = getAttribute( ATTR_CHARSET ).string();
            if (m_cachedSheet)
		m_cachedSheet->deref(this);
            m_cachedSheet = getDocument()->docLoader()->requestStyleSheet(m_url, chset);
            if (m_cachedSheet) {
                m_isCSSSheet = true;
		m_cachedSheet->ref(this);
            }
            else if (!isAlternate()) {
                // Error requesting sheet; decrement pending sheet count
                getDocument()->styleSheetLoaded();
            }
        }
    }
    else if (m_sheet) {
	// we no longer contain a stylesheet, e.g. perhaps rel or type was changed
	m_sheet->deref();
	m_sheet = 0;
        m_isCSSSheet = false;
	getDocument()->updateStyleSelector();
    }
}

void HTMLLinkElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();
    process();
}

void HTMLLinkElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();
    getDocument()->updateStyleSelector();
}

void HTMLLinkElementImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheetStr)
{
    if (m_sheet)
        m_sheet->deref();
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString( sheetStr, !getDocument()->inCompatMode() );

    MediaListImpl *media = new MediaListImpl( m_sheet, m_media );
    m_sheet->setMedia( media );

    finished();
}

void HTMLLinkElementImpl::finished()
{
    m_loading = false;

    // Tell the doc about the sheet.
    if (!isLoading() && !isDisabled() && !isAlternate())
        getDocument()->styleSheetLoaded();

    // ### major inefficiency, but currently necessary for proper
    // alternate styles support. don't recalc the styleselector
    // when nothing actually changed!
    if ( isAlternate() && m_sheet && !isDisabled())
        getDocument()->updateStyleSelector();
}

void HTMLLinkElementImpl::error( int, const QString& )
{
    finished();
}

bool HTMLLinkElementImpl::isLoading() const
{
//    kdDebug( 6030 ) << "link: checking if loading!" << endl;
    if(m_loading) return true;
    if(!m_sheet) return false;
    //if(!m_sheet->isCSSStyleSheet()) return false;
    return static_cast<CSSStyleSheetImpl *>(m_sheet)->isLoading();
}

void HTMLLinkElementImpl::sheetLoaded()
{
    if (!isLoading() && !isDisabled() && !isAlternate())
        getDocument()->styleSheetLoaded();
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLMetaElementImpl::id() const
{
    return ID_META;
}

void HTMLMetaElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_HTTP_EQUIV:
	m_equiv = attr->value();
	process();
	break;
    case ATTR_CONTENT:
	m_content = attr->value();
	process();
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLMetaElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();
    process();
}

void HTMLMetaElementImpl::process()
{
    // Get the document to process the tag, but only if we're actually part of DOM tree (changing a meta tag while
    // it's not in the tree shouldn't have any effect on the document)
    if (inDocument() && !m_equiv.isNull() && !m_content.isNull())
	getDocument()->processHttpEquiv(m_equiv,m_content);
}

// -------------------------------------------------------------------------

HTMLScriptElementImpl::HTMLScriptElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc), m_cachedScript(0), m_createdByParser(false), m_evaluated(false)
{
}

HTMLScriptElementImpl::~HTMLScriptElementImpl()
{
    if (m_cachedScript)
        m_cachedScript->deref(this);
}

NodeImpl::Id HTMLScriptElementImpl::id() const
{
    return ID_SCRIPT;
}

bool HTMLScriptElementImpl::isURLAttribute(AttributeImpl *attr) const
{
    return attr->id() == ATTR_SRC;
}

void HTMLScriptElementImpl::childrenChanged()
{
    // If a node is inserted as a child of the script element
    // and the script element has been inserted in the document
    // we evaluate the script.
    if (!m_createdByParser && inDocument() && firstChild())
        evaluateScript(getDocument()->URL().url(), text());
}

void HTMLScriptElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();

    assert(!m_cachedScript);

    if (m_createdByParser)
        return;

    QString url = getAttribute(ATTR_SRC).string();
    if (!url.isEmpty()) {
        QString charset = getAttribute(ATTR_CHARSET).string();
        m_cachedScript = getDocument()->docLoader()->requestScript(DOMString(url), charset);
        if (m_cachedScript)
            m_cachedScript->ref(this);
        return;
    }

    // If there's an empty script node, we shouldn't evaluate the script
    // because if a script is inserted afterwards (by setting text or innerText)
    // it should be evaluated, and evaluateScript only evaluates a script once.
    DOMString scriptString = text();
    if (!scriptString.isEmpty())
        evaluateScript(getDocument()->URL().url(), scriptString);
}

void HTMLScriptElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();

    if (m_cachedScript) {
        m_cachedScript->deref(this);
        m_cachedScript = 0;
    }
}

void HTMLScriptElementImpl::notifyFinished(CachedObject* o)
{
    CachedScript *cs = static_cast<CachedScript *>(o);

    assert(cs == m_cachedScript);

    QString   URL    = cs->url().string();
    DOMString script = cs->script();
    cs->deref(this);
    m_cachedScript = 0;

    evaluateScript(URL, script);
}

void HTMLScriptElementImpl::evaluateScript(const QString &URL, const DOMString &script)
{
    if (m_evaluated)
        return;

    KHTMLPart *part = getDocument()->part();
    if (part) {
        KJSProxy *proxy = KJSProxy::proxy(part);
        if (proxy) {
            m_evaluated = true;
            proxy->evaluate(URL, 0, script.string(), 0);
            DocumentImpl::updateDocumentsRendering();
        }
    }
}

DOMString HTMLScriptElementImpl::text() const
{
    DOMString val = "";

    for (NodeImpl *n = firstChild(); n; n = n->nextSibling()) {
        if (n->isTextNode())
            val += static_cast<TextImpl *>(n)->data();
    }

    return val;
}

void HTMLScriptElementImpl::setText(const DOMString &value)
{
    int exceptioncode = 0;
    int numChildren = childNodeCount();

    if (numChildren == 1 && firstChild()->isTextNode()) {
        static_cast<DOM::TextImpl *>(firstChild())->setData(value, exceptioncode);
        return;
    }

    if (numChildren > 0) {
        removeChildren();
    }

    appendChild(getDocument()->createTextNode(value.implementation()), exceptioncode);
}

DOMString HTMLScriptElementImpl::htmlFor() const
{
    // DOM Level 1 says: reserved for future use.
    return DOMString();
}

void HTMLScriptElementImpl::setHtmlFor(const DOMString &/*value*/)
{
    // DOM Level 1 says: reserved for future use.
}

DOMString HTMLScriptElementImpl::event() const
{
    // DOM Level 1 says: reserved for future use.
    return DOMString();
}

void HTMLScriptElementImpl::setEvent(const DOMString &/*value*/)
{
    // DOM Level 1 says: reserved for future use.
}

DOMString HTMLScriptElementImpl::charset() const
{
    return getAttribute(ATTR_CHARSET);
}

void HTMLScriptElementImpl::setCharset(const DOMString &value)
{
    setAttribute(ATTR_CHARSET, value);
}

bool HTMLScriptElementImpl::defer() const
{
    return !getAttribute(ATTR_DEFER).isNull();
}

void HTMLScriptElementImpl::setDefer(bool defer)
{
    setAttribute(ATTR_DEFER, defer ? "" : 0);
}

DOMString HTMLScriptElementImpl::src() const
{
    return getDocument()->completeURL(getAttribute(ATTR_SRC).string());
}

void HTMLScriptElementImpl::setSrc(const DOMString &value)
{
    setAttribute(ATTR_SRC, value);
}

DOMString HTMLScriptElementImpl::type() const
{
    return getAttribute(ATTR_TYPE);
}

void HTMLScriptElementImpl::setType(const DOMString &value)
{
    setAttribute(ATTR_TYPE, value);
}

// -------------------------------------------------------------------------

HTMLStyleElementImpl::~HTMLStyleElementImpl()
{
    if(m_sheet) m_sheet->deref();
}

NodeImpl::Id HTMLStyleElementImpl::id() const
{
    return ID_STYLE;
}

// other stuff...
void HTMLStyleElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch (attr->id())
    {
    case ATTR_TYPE:
        m_type = attr->value().lower();
        break;
    case ATTR_MEDIA:
        m_media = attr->value().string().lower();
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLStyleElementImpl::insertedIntoDocument()
{
    HTMLElementImpl::insertedIntoDocument();
    if (m_sheet)
        getDocument()->updateStyleSelector();
}

void HTMLStyleElementImpl::removedFromDocument()
{
    HTMLElementImpl::removedFromDocument();
    if (m_sheet)
        getDocument()->updateStyleSelector();
}

void HTMLStyleElementImpl::childrenChanged()
{
    HTMLElementImpl::childrenChanged();

    DOMString text = "";

    for (NodeImpl *c = firstChild(); c != 0; c = c->nextSibling()) {
	if ((c->nodeType() == Node::TEXT_NODE) ||
	    (c->nodeType() == Node::CDATA_SECTION_NODE) ||
	    (c->nodeType() == Node::COMMENT_NODE))
	    text += c->nodeValue();
    }

    if (m_sheet) {
        m_sheet->deref();
        m_sheet = 0;
    }

    m_loading = false;
    if ((m_type.isEmpty() || m_type == "text/css") // Type must be empty or CSS
         && (m_media.isNull() || m_media.contains("screen") || m_media.contains("all") || m_media.contains("print"))) {
        getDocument()->addPendingSheet();
        m_loading = true;
        m_sheet = new CSSStyleSheetImpl(this);
        m_sheet->ref();
        m_sheet->parseString( text, !getDocument()->inCompatMode() );
        MediaListImpl* media = new MediaListImpl( m_sheet, DOMString( m_media ) );
        m_sheet->setMedia( media );
        m_loading = false;
    }

    if (!isLoading() && m_sheet)
        getDocument()->styleSheetLoaded();
}

bool HTMLStyleElementImpl::isLoading() const
{
    if (m_loading) return true;
    if(!m_sheet) return false;
    return static_cast<CSSStyleSheetImpl *>(m_sheet)->isLoading();
}

void HTMLStyleElementImpl::sheetLoaded()
{
    if (!isLoading())
        getDocument()->styleSheetLoaded();
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLTitleElementImpl::id() const
{
    return ID_TITLE;
}

void HTMLTitleElementImpl::childrenChanged()
{
    HTMLElementImpl::childrenChanged();

    m_title = "";
    for (NodeImpl *c = firstChild(); c != 0; c = c->nextSibling()) {
	if ((c->nodeType() == Node::TEXT_NODE) || (c->nodeType() == Node::CDATA_SECTION_NODE))
	    m_title += c->nodeValue();
    }
    if ( !m_title.isEmpty() && inDocument())
        getDocument()->setTitle(m_title);
}

DOMString HTMLTitleElementImpl::text()
{
    if (firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
        return firstChild()->nodeValue();
    }
    return "";
}

void HTMLTitleElementImpl::setText( const DOMString& str )
{
    int exceptioncode = 0;
    // Look for an existing text child node
    DOM::NodeListImpl* nl(childNodes());
    if (nl)
    {
        for (unsigned int i = 0; i < nl->length(); i++) {
            if (nl->item(i)->nodeType() == DOM::Node::TEXT_NODE) {
                static_cast<DOM::TextImpl *>(nl->item(i))->setData(str, exceptioncode);
                return;
            }
        }
        delete nl;
    }
    // No child text node found, creating one
    DOM::TextImpl* t = getDocument()->createTextNode(str.implementation());
    appendChild(t, exceptioncode);
}
