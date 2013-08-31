/* This file is part of the KDE project
   Copyright (C) 2010 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "htmlextension.h"

#include "part.h"
#include <QHash>
#include <klocalizedstring.h>

using namespace KParts;

KParts::HtmlExtension::HtmlExtension(KParts::ReadOnlyPart* parent)
    : QObject(parent), d(0)
{
}

KParts::HtmlExtension::~HtmlExtension()
{
}

bool HtmlExtension::hasSelection() const
{
    return false;
}

HtmlExtension * KParts::HtmlExtension::childObject( QObject *obj )
{
    return obj->findChild<KParts::HtmlExtension *>(QString(), Qt::FindDirectChildrenOnly);
}

SelectorInterface::QueryMethods SelectorInterface::supportedQueryMethods() const
{
    return KParts::SelectorInterface::None;
}

class SelectorInterface::ElementPrivate : public QSharedData
{
public:
    QString tag;
    QHash<QString, QString> attributes;
};

SelectorInterface::Element::Element()
    : d(new ElementPrivate)
{
}

SelectorInterface::Element::Element(const SelectorInterface::Element& other)
    : d(other.d)
{
}

SelectorInterface::Element::~Element()
{
}

bool SelectorInterface::Element::isNull() const
{
    return d->tag.isNull();
}

void SelectorInterface::Element::setTagName(const QString& tag)
{
    d->tag = tag;
}

QString SelectorInterface::Element::tagName() const
{
    return d->tag;
}

void SelectorInterface::Element::setAttribute(const QString& name, const QString& value)
{
    d->attributes[name] = value; // insert or replace
}

QStringList SelectorInterface::Element::attributeNames() const
{
    return d->attributes.keys();
}

QString SelectorInterface::Element::attribute(const QString& name, const QString& defaultValue) const
{
    return d->attributes.value(name, defaultValue);
}

bool SelectorInterface::Element::hasAttribute(const QString& name) const
{
    return d->attributes.contains(name);
}

const char* HtmlSettingsInterface::javascriptAdviceToText(HtmlSettingsInterface::JavaScriptAdvice advice)
{
    // NOTE: The use of I18N_NOOP below is intended to allow GUI code to call
    // i18n on the returned text without affecting use of untranslated text in
    // config files.
    switch (advice) {
    case JavaScriptAccept:
        return I18N_NOOP("Accept");
    case JavaScriptReject:
        return I18N_NOOP("Reject");
    default:
        break;
    }

    return 0;
}

HtmlSettingsInterface::JavaScriptAdvice HtmlSettingsInterface::textToJavascriptAdvice(const QString& text)
{
    JavaScriptAdvice ret = JavaScriptDunno;

    if (!text.isEmpty()) {
        if (text.compare(QLatin1String("accept"), Qt::CaseInsensitive) == 0) {
            ret = JavaScriptAccept;
        } else if (text.compare(QLatin1String("reject"), Qt::CaseInsensitive) == 0) {
            ret = JavaScriptReject;
        }
    }

    return ret;
}

void HtmlSettingsInterface::splitDomainAdvice(const QString& adviceStr, QString& domain, HtmlSettingsInterface::JavaScriptAdvice& javaAdvice, HtmlSettingsInterface::JavaScriptAdvice& javaScriptAdvice)
{
    const QString tmp(adviceStr);
    const int splitIndex = tmp.indexOf(QLatin1Char(':'));

    if (splitIndex == -1) {
        domain = adviceStr.toLower();
        javaAdvice = JavaScriptDunno;
        javaScriptAdvice = JavaScriptDunno;
    } else {
        domain = tmp.left(splitIndex).toLower();
        const QString adviceString = tmp.mid(splitIndex+1, tmp.length());
        const int splitIndex2 = adviceString.indexOf(QLatin1Char(':'));
        if (splitIndex2 == -1) {
            // Java advice only
            javaAdvice = textToJavascriptAdvice(adviceString);
            javaScriptAdvice = JavaScriptDunno;
        } else {
            // Java and JavaScript advice
            javaAdvice = textToJavascriptAdvice(adviceString.left(splitIndex2));
            javaScriptAdvice = textToJavascriptAdvice(adviceString.mid(splitIndex2+1,
                                                      adviceString.length()));
        }
    }
}
