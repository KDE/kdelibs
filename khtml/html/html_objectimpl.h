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
#ifndef HTML_OBJECTIMPL_H
#define HTML_OBJECTIMPL_H

#include <qstringlist.h>
#include "html_elementimpl.h"
#include "misc/khtmllayout.h"

class KHTMLView;
class DOM::DOMStringImpl;

// -------------------------------------------------------------------------
namespace DOM {

class HTMLFormElementImpl;

class HTMLAppletElementImpl : public HTMLElementImpl
{
public:
    HTMLAppletElementImpl(DocumentImpl *doc);

    ~HTMLAppletElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return APPLETStartTag; }
    virtual tagStatus endTag() { return APPLETEndTag; }

    virtual void parseAttribute(AttrImpl *token);

    virtual void attach(KHTMLView *w);
    virtual void detach();

    virtual khtml::VAlign vAlign() { return valign; }

protected:
    DOMStringImpl *codeBase;
    DOMStringImpl *name;
    DOMStringImpl *code;
    DOMStringImpl *archive;
    int width;
    int height;

    KHTMLView *view;
    khtml::VAlign valign;
};

// -------------------------------------------------------------------------

class HTMLEmbedElementImpl : public HTMLElementImpl
{
public:
    HTMLEmbedElementImpl(DocumentImpl *doc);

    ~HTMLEmbedElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return OBJECTStartTag; }
    virtual tagStatus endTag() { return OBJECTEndTag; }

    virtual void parseAttribute(AttrImpl *attr);

    virtual void attach(KHTMLView *w);
    virtual void detach();

    QString url;
    QString pluginPage;
    QString serviceType;
    bool hidden;
    QStringList param;
};

// -------------------------------------------------------------------------

class HTMLObjectElementImpl : public HTMLElementImpl
{
public:
    HTMLObjectElementImpl(DocumentImpl *doc);

    ~HTMLObjectElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return OBJECTStartTag; }
    virtual tagStatus endTag() { return OBJECTEndTag; }

    HTMLFormElementImpl *form() const;

    long tabIndex() const;
    void setTabIndex( long );

    virtual void parseAttribute(AttrImpl *token);

    virtual void attach(KHTMLView *w);

    QString serviceType;
    QString url;
    QString classId;
};

// -------------------------------------------------------------------------

class HTMLParamElementImpl : public HTMLElementImpl
{
    friend class HTMLAppletElementImpl;
public:
    HTMLParamElementImpl(DocumentImpl *doc);

    ~HTMLParamElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return PARAMStartTag; }
    virtual tagStatus endTag() { return PARAMEndTag; }

    virtual void parseAttribute(AttrImpl *token);

    QString name() { return QConstString(m_name->s, m_name->l).string(); }
    QString value() { return QConstString(m_value->s, m_value->l).string(); }

 protected:
    DOMStringImpl *m_name;
    DOMStringImpl *m_value;
};

};
#endif
