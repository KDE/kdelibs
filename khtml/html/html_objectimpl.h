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
#ifndef HTML_OBJECTIMPL_H
#define HTML_OBJECTIMPL_H

#include "html_elementimpl.h"
#include "xml/dom_stringimpl.h"
#include <kparts/browserextension.h>
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
    virtual void detach();

    virtual void recalcStyle( StyleChange ch );

    void renderAlternative();

    bool get(const unsigned long, const QString &, KParts::LiveConnectExtension::Type &, unsigned long &, QString &);
    bool put(const unsigned long, const QString &, const QString &);
    bool call(const unsigned long, const QString &, const QStringList &, KParts::LiveConnectExtension::Type &, unsigned long &, QString &);
    void unregister(const unsigned long);

    void setLiveConnect(KParts::LiveConnectExtension * lc);
    void setServiceType(const QString &);

    QString url;
    QString classId;
    QString serviceType;
    bool needWidgetUpdate;
    bool m_renderAlternative;

protected slots:
    void liveConnectEvent(const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList&);
    void slotRenderAlternative();

private:
    KParts::LiveConnectExtension *liveconnect;
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
