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
#include <kstringhandler.h>
#include <kdebug.h>

using namespace khtml;

HTMLBaseElementImpl::HTMLBaseElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
}

HTMLBaseElementImpl::~HTMLBaseElementImpl()
{
}

NodeImpl::Id HTMLBaseElementImpl::id() const
{
    return ID_BASE;
}

void HTMLBaseElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_HREF:
      _href = khtml::parseURL(attr->value());
      break;
    case ATTR_TARGET:
      _target = attr->value();
      break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLBaseElementImpl::init()
{
    HTMLElementImpl::init();

    if(!_href.isEmpty())
        getDocument()->setBaseURL( KURL( getDocument()->view()->part()->url(), _href.string() ).url() );

    if(!_target.isEmpty())
        getDocument()->setBaseTarget( _target.string() );
}

// -------------------------------------------------------------------------

HTMLLinkElementImpl::HTMLLinkElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
    m_sheet = 0;
    m_loading = false;
    m_cachedSheet = 0;
}

HTMLLinkElementImpl::~HTMLLinkElementImpl()
{
    if(m_sheet) m_sheet->deref();
    if(m_cachedSheet) m_cachedSheet->deref(this);
}

NodeImpl::Id HTMLLinkElementImpl::id() const
{
    return ID_LINK;
}

void HTMLLinkElementImpl::init()
{
    HTMLElementImpl::init();

    QString type = m_type.string().lower();
    QString rel = m_rel.string().lower();

    KHTMLPart* part = getDocument()->view() ? getDocument()->view()->part() : 0;

    // IE extension: location of small icon for locationbar / bookmarks
    if ( part && rel.contains("shortcut icon") && !m_url.isEmpty() && !part->parentPart())
        part->browserExtension()->setIconURL( KURL(m_url.string()) );

    if((type.contains("text/css") || rel == "stylesheet") && !rel.contains("alternate"))
    {
        // no need to load style sheets which aren't for the screen output
        // ### there may be in some situations e.g. for an editor or script to manipulate
        if( m_media.isNull() || m_media.contains("screen") || m_media.contains("all") || m_media.contains("print") )
        {
            m_loading = true;
            DocumentImpl *doc = getDocument();
            // we must have a doc->docLoader() !
            QString chset = getAttribute( ATTR_CHARSET ).string();
            m_cachedSheet = doc->docLoader()->requestStyleSheet(m_url, chset);
            if(m_cachedSheet) m_cachedSheet->ref(this);
        }
    }

    getDocument()->updateStyleSelector();
}

void HTMLLinkElementImpl::parseAttribute(AttrImpl *attr)
{
    switch (attr->attrId)
    {
    case ATTR_REL:
        m_rel = attr->value();
        break;
    case ATTR_HREF:
        m_url = getDocument()->completeURL( khtml::parseURL(attr->value()).string() );
        break;
    case ATTR_TYPE:
        m_type = attr->value();
        break;
    case ATTR_MEDIA:
        m_media = attr->value().string().lower();
        break;
    case ATTR_DISABLED:
        // ###
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLLinkElementImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheetStr)
{
//    kdDebug( 6030 ) << "HTMLLinkElement::setStyleSheet()" << endl;
//    kdDebug( 6030 ) << "**** current medium: " << m_media << endl;

    if( m_sheet ) return;
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString(sheetStr);

    MediaListImpl *media = new MediaListImpl( m_sheet, m_media );
    m_sheet->setMedia( media );

    m_loading = false;

    getDocument()->updateStyleSelector();
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
    getDocument()->updateStyleSelector();
}

StyleSheetImpl *HTMLLinkElementImpl::sheet() const
{
    //kdDebug( 6030 ) << "**** HTMLLinkElementImpl::sheet()" << endl;

    return m_sheet;
}


// -------------------------------------------------------------------------

HTMLMetaElementImpl::HTMLMetaElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc)
{
    m_processed = false;
}

HTMLMetaElementImpl::~HTMLMetaElementImpl()
{
}

NodeImpl::Id HTMLMetaElementImpl::id() const
{
    return ID_META;
}

void HTMLMetaElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_HTTP_EQUIV:
	m_equiv = attr->value();
	m_processed = false;
	checkProcess();
	break;
    case ATTR_CONTENT:
	m_content = attr->value();
	m_processed = false;
	checkProcess();
	break;
    case ATTR_NAME:
      break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLMetaElementImpl::insertedIntoDocument()
{
    checkProcess();
}

void HTMLMetaElementImpl::checkProcess()
{
    if (!m_processed && !m_equiv.isNull() && !m_content.isNull()) {
	// Get the document to process the tag, but only if we're actually part of DOM tree (changing a meta tag while
	// it's not in the tree shouldn't have any effect on the document)

	NodeImpl *doc = parentNode();
	while (doc && (doc->nodeType() != Node::DOCUMENT_NODE))
	    doc = doc->parentNode();
	if (doc) {
	    static_cast<DocumentImpl*>(doc)->processHttpEquiv(m_equiv,m_content);
	    m_processed = true;
	}
    }
}

// -------------------------------------------------------------------------

HTMLScriptElementImpl::HTMLScriptElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc)
{
}

HTMLScriptElementImpl::~HTMLScriptElementImpl()
{
}

NodeImpl::Id HTMLScriptElementImpl::id() const
{
    return ID_SCRIPT;
}

// -------------------------------------------------------------------------

HTMLStyleElementImpl::HTMLStyleElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc)
{
    m_sheet = 0;
}

HTMLStyleElementImpl::~HTMLStyleElementImpl()
{
    if(m_sheet) m_sheet->deref();
}

NodeImpl::Id HTMLStyleElementImpl::id() const
{
    return ID_STYLE;
}

// other stuff...
void HTMLStyleElementImpl::parseAttribute(AttrImpl *attr)
{
    switch (attr->attrId)
    {
    case ATTR_TYPE:
        m_type = attr->value(); break;
    case ATTR_MEDIA:
        m_media = attr->value(); break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

NodeImpl *HTMLStyleElementImpl::addChild(NodeImpl *child)
{
    NodeImpl *r = NodeBaseImpl::addChild(child);
    reparseSheet();
    sheetLoaded();
    return r;
}

void HTMLStyleElementImpl::setChanged(bool b)
{
    // TextImpl sets it's parent to be changed when appendData() or similar is called (hack)
    // ### make this work properly in all situations
    if (b)
        reparseSheet();

    HTMLElementImpl::setChanged(b);
}

bool HTMLStyleElementImpl::isLoading() const
{
    if(!m_sheet) return false;
    //if(!m_sheet->isCSSStyleSheet()) return false;
    return static_cast<CSSStyleSheetImpl *>(m_sheet)->isLoading();
}

void HTMLStyleElementImpl::sheetLoaded()
{
    getDocument()->updateStyleSelector();
}

void HTMLStyleElementImpl::reparseSheet()
{
    DOMString text = "";
    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling()) {
        if (n->nodeType() == Node::TEXT_NODE ||
            n->nodeType() == Node::CDATA_SECTION_NODE ||
            n->nodeType() == Node::COMMENT_NODE)
        text += static_cast<CharacterDataImpl*>(n)->data();
    }

    if(m_sheet)
	m_sheet->deref();
    m_sheet = new CSSStyleSheetImpl(this);
    m_sheet->ref();
    m_sheet->parseString( text, (getDocument()->parseMode() == DocumentImpl::Strict) );
    getDocument()->updateStyleSelector();
}

void HTMLStyleElementImpl::init()
{
    HTMLElementImpl::init();

    if (m_sheet) getDocument()->updateStyleSelector();
}

// -------------------------------------------------------------------------

HTMLTitleElementImpl::HTMLTitleElementImpl(DocumentPtr *doc)
    : HTMLElementImpl(doc)
{
}

HTMLTitleElementImpl::~HTMLTitleElementImpl()
{
}

NodeImpl::Id HTMLTitleElementImpl::id() const
{
    return ID_TITLE;
}

void HTMLTitleElementImpl::setTitle()
{
    if(!_first || _first->id() != ID_TEXT) return;
    TextImpl *t = static_cast<TextImpl *>(_first);
    QString s = t->data().string();
    s.compose();
    s = KStringHandler::csqueeze( s, 128 );

    DocumentImpl *d = getDocument();
    if ( !d->view()->part()->parentPart() )
        emit d->view()->part()->setWindowCaption( s );
}
