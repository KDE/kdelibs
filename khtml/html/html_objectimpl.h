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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef HTML_OBJECTIMPL_H
#define HTML_OBJECTIMPL_H

#include "html_elementimpl.h"
#include "xml/dom_stringimpl.h"
#include <qobject.h>
#include <qstringlist.h>

class KHTMLView;

// -------------------------------------------------------------------------
namespace DOM {

class HTMLFormElementImpl;
class DOMStringImpl;

class HTMLObjectBaseElementImpl : public QObject, public HTMLElementImpl
{
    Q_OBJECT
public:
    HTMLObjectBaseElementImpl(DocumentPtr *doc);

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();

    virtual void recalcStyle( StyleChange ch );

    void renderAlternative();

    void setServiceType(const QString &);

    QString url;
    QString classId;
    QString serviceType;
    bool needWidgetUpdate;
    bool m_renderAlternative;

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();
    virtual void addId(const QString& id);
    virtual void removeId(const QString& id);
protected Q_SLOTS:
    void slotRenderAlternative();
protected:
    DOMString     m_name;
};

// -------------------------------------------------------------------------

class HTMLAppletElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLAppletElementImpl(DocumentPtr *doc);

    ~HTMLAppletElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *token);
    virtual void attach();
protected:
    khtml::VAlign valign;
};

// -------------------------------------------------------------------------

class HTMLEmbedElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLEmbedElementImpl(DocumentPtr *doc);
    ~HTMLEmbedElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();

    QString pluginPage;
    bool hidden;
};

// -------------------------------------------------------------------------

class HTMLObjectElementImpl : public HTMLObjectBaseElementImpl
{
public:
    HTMLObjectElementImpl(DocumentPtr *doc);

    ~HTMLObjectElementImpl();

    virtual Id id() const;

    HTMLFormElementImpl *form() const;

    virtual void parseAttribute(AttributeImpl *token);

    virtual void attach();

    DocumentImpl* contentDocument() const;
};

// -------------------------------------------------------------------------

class HTMLParamElementImpl : public HTMLElementImpl
{
    friend class HTMLAppletElementImpl;
public:
    HTMLParamElementImpl(DocumentPtr* _doc) : HTMLElementImpl(_doc) {}

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *token);

    QString name() const { return m_name; }
    QString value() const { return m_value; }

 protected:
    QString m_name;
    QString m_value;
};

} // namespace
#endif
