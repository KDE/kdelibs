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

// -------------------------------------------------------------------------
#ifndef HTML_BLOCKIMPL_H
#define HTML_BLOCKIMPL_H

#include "html_elementimpl.h"
#include "dtd.h"

namespace DOM {

// -------------------------------------------------------------------------

class HTMLDivElementImpl : public HTMLElementImpl
{
public:
    HTMLDivElementImpl(DocumentPtr *doc)
        : HTMLElementImpl(doc) {}

    virtual NodeImpl::Id id() const;
    virtual void parseAttribute(AttributeImpl *token);
};

// -------------------------------------------------------------------------

class HTMLHRElementImpl : public HTMLElementImpl
{
public:
    HTMLHRElementImpl(DocumentPtr *doc)
        : HTMLElementImpl(doc) {}

    virtual NodeImpl::Id id() const;
    virtual void parseAttribute(AttributeImpl *);
    virtual void attach();
};

// -------------------------------------------------------------------------

class HTMLPreElementImpl : public HTMLGenericElementImpl
{
public:
    HTMLPreElementImpl(DocumentPtr *doc, ushort _tagid)
        : HTMLGenericElementImpl(doc, _tagid) {}

    long width() const;
    void setWidth( long w );
};

}; //namespace
#endif
