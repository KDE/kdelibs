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
#ifndef HTML_OBJECTIMPL_H
#define HTML_OBJECTIMPL_H

#include "html_elementimpl.h"
#include "html_form.h"

class KHTMLWidget;
class KJavaAppletWidget;
class DOM::DOMStringImpl;
using namespace DOM;

// -------------------------------------------------------------------------
namespace DOM {

class HTMLAppletElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLAppletElementImpl(DocumentImpl *doc);

    ~HTMLAppletElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return APPLETStartTag; }
    virtual tagStatus endTag() { return APPLETEndTag; }

    virtual void parseAttribute(Attribute *token);
    virtual void layout(bool);

    virtual void attach(KHTMLWidget *w);
    virtual void detach();

    virtual void setXPos( int xPos );
    virtual void setYPos( int yPos );

    virtual bool isRendered() { return true; }

 protected:
    DOMStringImpl *base;
    DOMStringImpl *name;
    DOMStringImpl *code;

    KJavaAppletWidget *applet;
    KHTMLWidget *view;
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
    HTMLFormElement form() const;
    bool declare() const;

    void setDeclare( bool );
    long tabIndex() const;

    void setTabIndex( long );
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

    virtual void parseAttribute(Attribute *token);

 protected:
    DOMStringImpl *name;
    DOMStringImpl *value;
};

};
#endif
