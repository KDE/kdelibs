/*
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
#ifndef HTML_LISTIMPL_H
#define HTML_LISTIMPL_H

/*
 * we ignore the deprecated compact attribute. Netscape does so too...
 */

#include "html_elementimpl.h"

namespace DOM
{

class DOMString;

class HTMLUListElementImpl : public HTMLElementImpl
{
public:
    HTMLUListElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc) {}

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *);

    virtual int start() const { return 1; }
};

// -------------------------------------------------------------------------

class HTMLDirectoryElementImpl : public HTMLElementImpl
{
public:
    HTMLDirectoryElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc) {}

    virtual Id id() const;
};

// -------------------------------------------------------------------------

class HTMLMenuElementImpl : public HTMLElementImpl
{
public:
    HTMLMenuElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc) {}
    virtual ~HTMLMenuElementImpl() {}

    virtual Id id() const;
};

// -------------------------------------------------------------------------

class HTMLOListElementImpl : public HTMLUListElementImpl
{
public:
    HTMLOListElementImpl(DocumentPtr *doc)
        : HTMLUListElementImpl(doc), _start(1) {}

    virtual Id id() const;
    virtual void parseAttribute(AttributeImpl *);

    int start() const { return _start; }
private:
    int _start;
};

// -------------------------------------------------------------------------

class HTMLLIElementImpl : public HTMLElementImpl
{
public:
    HTMLLIElementImpl(DocumentPtr *doc)
        : HTMLElementImpl(doc) {}

    virtual Id id() const;
    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
};

// -------------------------------------------------------------------------

class HTMLDListElementImpl : public HTMLElementImpl
{
public:
    HTMLDListElementImpl(DocumentPtr *doc) : HTMLElementImpl(doc) {}
    virtual ~HTMLDListElementImpl() {}

    virtual Id id() const;
};

}; //namespace

#endif
