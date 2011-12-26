/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBUSTYPES_H
#define DBUSTYPES_H

#include <QtCore/QMetaType>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QDBusArgument>

// #include "simpleresource.h"
// #include "nepomukdatamanagement_export.h"
#include "nepomuk_export.h"

//CAUTION: Q_DECLARE_METATYPE doesn't accept template arguments like QHash<T, T>
typedef QHash<QString, QString> __nepomuk_QHashQStringQString;
Q_DECLARE_METATYPE( __nepomuk_QHashQStringQString )

namespace Nepomuk {
    namespace DBus {
        QString NEPOMUK_EXPORT convertUri(const QUrl& uri);
        QStringList NEPOMUK_EXPORT convertUriList(const QList<QUrl>& uris);

        /// Convert QDBusArguments variants into QUrl, QDate, QTime, and QDateTime variants
        NEPOMUK_EXPORT QVariant resolveDBusArguments(const QVariant& v);
        NEPOMUK_EXPORT QVariantList resolveDBusArguments(const QVariantList& l);

        /// Replaces KUrl with QUrl for DBus marshalling.
        QVariantList normalizeVariantList(const QVariantList& l);

        NEPOMUK_EXPORT void registerDBusTypes();
    }
}

QDBusArgument& operator<<( QDBusArgument& arg, const QUrl& url );
const QDBusArgument& operator>>( const QDBusArgument& arg, QUrl& url );

#endif // DBUSTYPES_H
