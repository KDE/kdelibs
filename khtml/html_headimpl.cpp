/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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
#include "html_headimpl.h"
#include "html_form.h"
using namespace DOM;


HTMLBaseElementImpl::HTMLBaseElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
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

HTMLFormElement HTMLIsIndexElementImpl::form() const
{
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
}

void HTMLLinkElementImpl::setDisabled( const bool & )
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
}

void HTMLScriptElementImpl::setDefer( const bool & )
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
}

void HTMLStyleElementImpl::setDisabled( const bool & )
{
}

// -------------------------------------------------------------------------

HTMLTitleElementImpl::HTMLTitleElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
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

