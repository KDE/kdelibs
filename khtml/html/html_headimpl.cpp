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
// -------------------------------------------------------------------------

#include "dom_string.h"
#include "dom_textimpl.h"
#include "html_headimpl.h"
#include "html_form.h"
#include "html_documentimpl.h"
using namespace DOM;

#include "khtmlview.h"
#include "khtml_part.h"
#include "htmlhashes.h"

#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
using namespace khtml;

#include <qfile.h>
#include <qtextstream.h>

#include <kurl.h>

#include <kdebug.h>

HTMLBaseElementImpl::HTMLBaseElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLBaseElementImpl::~HTMLBaseElementImpl()
{
}

const DOMString HTMLBaseElementImpl::nodeName() const
{
    return "BASE";
}

ushort HTMLBaseElementImpl::id() const
{
    return ID_BASE;
}

void HTMLBaseElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_HREF:
      _href = attr->value();
      break;
    case ATTR_TARGET:
      _target = attr->value();
      break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}


void HTMLBaseElementImpl::attach(KHTMLView *v)
{
    m_style = document->styleSelector()->styleForElement(this);
    if(_href.length())
    {
      v->part()->setBaseURL( KURL( _href.string() ) );
    }	
    if(_target.length())
    {
      v->part()->setBaseTarget(_target.string());
    }
    NodeBaseImpl::attach( v );
}

// -------------------------------------------------------------------------

HTMLIsIndexElementImpl::HTMLIsIndexElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLIsIndexElementImpl::~HTMLIsIndexElementImpl()
{
}

const DOMString HTMLIsIndexElementImpl::nodeName() const
{
    return "ISINDEX";
}

ushort HTMLIsIndexElementImpl::id() const
{
    return ID_ISINDEX;
}

HTMLFormElementImpl *HTMLIsIndexElementImpl::form() const
{
  // ###
  return 0;
}

// -------------------------------------------------------------------------

HTMLLinkElementImpl::HTMLLinkElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
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

const DOMString HTMLLinkElementImpl::nodeName() const
{
    return "LINK";
}

ushort HTMLLinkElementImpl::id() const
{
    return ID_LINK;
}

bool HTMLLinkElementImpl::disabled() const
{
  // ###
  return false;
}

void HTMLLinkElementImpl::setDisabled( bool )
{
}

// other stuff...
void HTMLLinkElementImpl::attach(KHTMLView *v)
{
    m_style = document->styleSelector()->styleForElement(this);

    QString type = m_type.string().lower();
    QString rel = m_rel.string().lower();

    if(m_type == "text/css" && !rel.contains("alternate"))
    {
	QString str = m_media.string().lower();
	// no need to load style sheets which aren't for the screen output
	if(m_media.isNull() || str.contains("screen") || str.contains("all"))
	{
	    m_loading = true;
	    HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
	    m_cachedSheet = Cache::requestStyleSheet(m_url, doc->baseURL());
	    m_cachedSheet->ref(this);
	}
    }
    NodeBaseImpl::attach( v );
}

void HTMLLinkElementImpl::parseAttribute(AttrImpl *attr)
{
    switch (attr->attrId)
    {
    case ATTR_REL:
	m_rel = attr->value(); break;
    case ATTR_HREF:
	m_url = attr->value(); break;
    case ATTR_TYPE:
	m_type = attr->value(); break;
    case ATTR_MEDIA:
	m_media = attr->value(); break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLLinkElementImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet)
{
    kdDebug( 6030 ) << "HTMLLinkElement::setStyleSheet()" << endl;
    m_sheet = new CSSStyleSheetImpl(this, url);
    m_sheet->ref();
    m_sheet->parseString(sheet);
    m_loading = false;

    if(!isLoading()) sheetLoaded();
}

bool HTMLLinkElementImpl::isLoading()
{
    kdDebug( 6030 ) << "link: checking if loading!" << endl;
    if(m_loading) return true;
    if(!m_sheet) return false;
    //if(!m_sheet->isCSSStyleSheet()) return false;
    return static_cast<CSSStyleSheetImpl *>(m_sheet)->isLoading();
}

void HTMLLinkElementImpl::sheetLoaded()
{
    document->createSelector();
}

// -------------------------------------------------------------------------

HTMLMetaElementImpl::HTMLMetaElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLMetaElementImpl::~HTMLMetaElementImpl()
{
}

const DOMString HTMLMetaElementImpl::nodeName() const
{
    return "META";
}

ushort HTMLMetaElementImpl::id() const
{
    return ID_META;
}

void HTMLMetaElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_HTTP_EQUIV:
      _equiv = attr->value();
      break;
    case ATTR_CONTENT:
      _content = attr->value();
      break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}


void HTMLMetaElementImpl::attach(KHTMLView *v)
{
    m_style = document->styleSelector()->styleForElement(this);
    kdDebug( 6030 ) << "meta::attach() equiv=" << _equiv.string() << ", content=" << _content.string() << endl;
    if(strcasecmp(_equiv, "refresh") == 0 && !_content.isNull())
    {
	// get delay and url
	QString str = _content.string();
	int pos = str.find(QRegExp("[;,]"));
	int delay = str.left(pos).toInt();
	kdDebug( 6030 ) << "delay = " << delay << ", separator at " << pos << endl;
	pos++;
	while(pos < (int)str.length() && str[pos].isSpace()) pos++;
	if(pos < (int)str.length()) str = str.mid(pos);
	if(strncasecmp(str, "url=", 4) == 0)
	{
	    str = str.mid(4);
	    kdDebug( 6030 ) << "====> got redirect to " << str << endl;
	    v->part()->scheduleRedirection(delay, str);
	}
    }
    NodeBaseImpl::attach( v );
}

// -------------------------------------------------------------------------

HTMLScriptElementImpl::HTMLScriptElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLScriptElementImpl::~HTMLScriptElementImpl()
{
}

const DOMString HTMLScriptElementImpl::nodeName() const
{
    return "SCRIPT";
}

ushort HTMLScriptElementImpl::id() const
{
    return ID_SCRIPT;
}

bool HTMLScriptElementImpl::defer() const
{
  // ###
  return false;
}

void HTMLScriptElementImpl::setDefer( bool )
{
}

// -------------------------------------------------------------------------

HTMLStyleElementImpl::HTMLStyleElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    m_sheet = 0;
}

HTMLStyleElementImpl::~HTMLStyleElementImpl()
{
    if(m_sheet) m_sheet->deref();
}

const DOMString HTMLStyleElementImpl::nodeName() const
{
    return "STYLE";
}

ushort HTMLStyleElementImpl::id() const
{
    return ID_STYLE;
}

bool HTMLStyleElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLStyleElementImpl::setDisabled( bool )
{
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
    if(!child->isTextNode()) return this;

    DOMString text = static_cast<TextImpl *>(child)->string();

    kdDebug( 6030 ) << "style: parsing sheet '" << text.string() << "'" << endl;

    if(m_sheet) m_sheet->deref();
    m_sheet = new CSSStyleSheetImpl(this);
    m_sheet->parseString(text);

    sheetLoaded();
    return NodeBaseImpl::addChild(child);
}

bool HTMLStyleElementImpl::isLoading()
{
    if(!m_sheet) return false;
    //if(!m_sheet->isCSSStyleSheet()) return false;
    return static_cast<CSSStyleSheetImpl *>(m_sheet)->isLoading();
}

void HTMLStyleElementImpl::sheetLoaded()
{
    document->createSelector();
}

// -------------------------------------------------------------------------

HTMLTitleElementImpl::HTMLTitleElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLTitleElementImpl::~HTMLTitleElementImpl()
{
}

const DOMString HTMLTitleElementImpl::nodeName() const
{
    return "TITLE";
}

ushort HTMLTitleElementImpl::id() const
{
    return ID_TITLE;
}

void HTMLTitleElementImpl::setTitle()
{
    kdDebug( 6030 ) << "Title:close" << endl;
    if(!_first || _first->id() != ID_TEXT) return;
    TextImpl *t = static_cast<TextImpl *>(_first);
    QString s = t->data().string();

    HTMLDocumentImpl *d = static_cast<HTMLDocumentImpl *>(document);
    emit d->view()->part()->setWindowCaption( s );
}
