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

#include "khtml.h"
#include "khtmlattrs.h"

#include <stdio.h>

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

void HTMLBaseElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
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


void HTMLBaseElementImpl::attach(KHTMLWidget *v)
{
    if(_href.length())
    {
	v->setBaseUrl(_href.string());
    }	
    if(_target.length())
    {
	v->setBaseTarget(_target.string());
    }
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
}

HTMLLinkElementImpl::~HTMLLinkElementImpl()
{
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
  return true;
}

void HTMLLinkElementImpl::setDisabled( bool )
{
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

void HTMLMetaElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
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


void HTMLMetaElementImpl::attach(KHTMLWidget *v)
{
    printf("meta::attach() equiv=%s, content=%s\n", _equiv.string().ascii(), _content.string().ascii());
    if(strcasecmp(_equiv, "refresh") == 0 && !_content.isNull())
    {
	// get delay and url
	QString str = _content.string();
	int pos = str.find(QRegExp("[;,]"));
	int delay = str.left(pos).toInt();
	printf("delay = %d, separator at %d\n", delay, pos);
	pos++;
	while(pos < str.length() && str[pos].isSpace()) pos++;
	if(pos < str.length()) str = str.mid(pos);
	if(strncasecmp(str, "url=", 4) == 0)
	{
	    str = str.mid(4);
	    printf("====> got redirect to %s\n", str.ascii());
	    v->scheduleRedirection(delay, str);
	}
    }
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
}

HTMLStyleElementImpl::~HTMLStyleElementImpl()
{
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
  return true;
}

void HTMLStyleElementImpl::setDisabled( bool )
{
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

void HTMLTitleElementImpl::close()
{
    printf("Title:close\n");
    if(!_first || _first->id() != ID_TEXT) return;
    TextImpl *t = static_cast<TextImpl *>(_first);
    QString s = t->data().string();

    HTMLDocumentImpl *d = static_cast<HTMLDocumentImpl *>(document);
    d->HTMLWidget()->setTitle(s);
}
