/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002 Apple Computer, Inc.
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
    getDocument()->setBaseURL( KURL() );
    getDocument()->setBaseTarget( QString::null );
}

void HTMLBaseElementImpl::process()
{
    if (!inDocument())
	return;

    if(!m_href.isEmpty() && getDocument()->view())
	getDocument()->setBaseURL( KURL( getDocument()->view()->part()->url(), m_href.string() ) );

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
        part->browserExtension()->setIconURL( KURL(m_url.string()) );

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
    m_sheet->parseString( sheetStr, getDocument()->parseMode() == DocumentImpl::Strict );

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
    case ATTR_NAME:
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

NodeImpl::Id HTMLScriptElementImpl::id() const
{
    return ID_SCRIPT;
}

DOMString HTMLScriptElementImpl::text() const
{
    if (firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
        return firstChild()->nodeValue();
    }
    return "";
}

void HTMLScriptElementImpl::setText(const DOMString& str)
{
    int exceptioncode = 0;
    if (firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
        static_cast<DOM::TextImpl *>(firstChild())->setData(str, exceptioncode);
        return;
    }
    // No child text node found, creating one
    DOM::TextImpl* t = getDocument()->createTextNode(str.implementation());
    appendChild(t, exceptioncode);
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
        m_sheet->parseString( text, (getDocument()->parseMode() == DocumentImpl::Strict) );
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
