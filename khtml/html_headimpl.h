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
#ifndef HTML_HEADIMPL_H
#define HTML_HEADIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"

namespace DOM {

class DOMString;
class HTMLFormElement;

class HTMLBaseElementImpl : public HTMLElementImpl
{
public:
    HTMLBaseElementImpl(DocumentImpl *doc);

    ~HTMLBaseElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BASEStartTag; }
    virtual tagStatus endTag() { return BASEEndTag; }
};

// -------------------------------------------------------------------------

class HTMLIsIndexElementImpl : public HTMLElementImpl
{
public:
    HTMLIsIndexElementImpl(DocumentImpl *doc);

    ~HTMLIsIndexElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return ISINDEXStartTag; }
    virtual tagStatus endTag() { return ISINDEXEndTag; }
    HTMLFormElement form() const;
};

// -------------------------------------------------------------------------

class HTMLLinkElementImpl : public HTMLElementImpl
{
public:
    HTMLLinkElementImpl(DocumentImpl *doc);

    ~HTMLLinkElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return LINKStartTag; }
    virtual tagStatus endTag() { return LINKEndTag; }
    bool disabled() const;

    void setDisabled( bool );
};

// -------------------------------------------------------------------------

class HTMLMetaElementImpl : public HTMLElementImpl
{
public:
    HTMLMetaElementImpl(DocumentImpl *doc);

    ~HTMLMetaElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return METAStartTag; }
    virtual tagStatus endTag() { return METAEndTag; }
};

// -------------------------------------------------------------------------

class HTMLScriptElementImpl : public HTMLElementImpl
{
public:
    HTMLScriptElementImpl(DocumentImpl *doc);

    ~HTMLScriptElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return SCRIPTStartTag; }
    virtual tagStatus endTag() { return SCRIPTEndTag; }
    bool defer() const;

    void setDefer( bool );
};

// -------------------------------------------------------------------------

class HTMLStyleElementImpl : public HTMLElementImpl
{
public:
    HTMLStyleElementImpl(DocumentImpl *doc);

    ~HTMLStyleElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return STYLEStartTag; }
    virtual tagStatus endTag() { return STYLEEndTag; }
    bool disabled() const;

    void setDisabled( bool );
};

// -------------------------------------------------------------------------

class HTMLTitleElementImpl : public HTMLElementImpl
{
public:
    HTMLTitleElementImpl(DocumentImpl *doc);

    ~HTMLTitleElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return TITLEStartTag; }
    virtual tagStatus endTag() { return TITLEEndTag; }
};

}; //namespace

#endif
