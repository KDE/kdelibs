/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_PROVIDER_HANDLER_H
#define KNEWSTUFF2_PROVIDER_HANDLER_H

#include <knewstuff2/core/provider.h>

#include <QtXml/qdom.h>
#include <QtCore/QObject>
#include <QtCore/QString>

namespace KIO { class Job; }

namespace KNS {

/**
 * @short Parser and dumper for KNewStuff providers.
 *
 * This class can be used to construct provider entry objects
 * from their XML representation. It can also be used
 * for the inverse.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT ProviderHandler
{
  public:
    ProviderHandler(const QDomElement &providerxml);
    ProviderHandler(const Provider& provider);

    bool isValid();
    Provider provider();
    Provider *providerptr();
    QDomElement providerXML();

  private:
    QDomElement serializeElement(const Provider& entry);
    Provider deserializeElement(const QDomElement& providerxml);
    QDomElement addElement(QDomDocument& doc, QDomElement& parent,
      const QString& tag, const QString& value);

    Provider mProvider;
    QDomElement mProviderXML;
    bool mValid;
};

}

#endif
