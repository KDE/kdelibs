/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KSECRETSSERVICEDBUSTYPES_H
#define KSECRETSSERVICEDBUSTYPES_H

#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusArgument>
#include <QtCore/QMap>
#include <QtCore/QString>

namespace KSecretsService {
    
/**
 * @internal
 * This is the basic Secret structure exchanged via the dbus API
 * See the spec for more details
 */
struct DBusSecretStruct {
    QDBusObjectPath m_session;
    QByteArray      m_parameters;
    QByteArray      m_value;
    QString         m_contentType;
};

typedef QMap<QString, QString> DBusStringStringMap;
typedef QMap<QDBusObjectPath, DBusSecretStruct> DBusObjectPathSecretMap;
typedef QMap<QString, QVariant> DBusStringVariantMap;


/**
 * @internal
 */
inline QDBusArgument &operator<<(QDBusArgument &argument, const DBusSecretStruct &secret)
{
    argument.beginStructure();
    argument << secret.m_session << secret.m_parameters << secret.m_value << secret.m_contentType;
    argument.endStructure();
    return argument;
}

/**
 * @internal
 */
inline const QDBusArgument &operator>>(const QDBusArgument &argument, DBusSecretStruct &secret)
{
    argument.beginStructure();
    argument >> secret.m_session >> secret.m_parameters >> secret.m_value >> secret.m_contentType;
    argument.endStructure();
    return argument;
}

} // namespace

#ifndef STRINGSTRINGMAP_METATYPE_DEFINED
Q_DECLARE_METATYPE( KSecretsService::DBusStringStringMap );
#define STRINGSTRINGMAP_METATYPE_DEFINED
#endif // STRINGSTRINGMAP_METATYPE_DEFINED

Q_DECLARE_METATYPE( KSecretsService::DBusObjectPathSecretMap );
Q_DECLARE_METATYPE( KSecretsService::DBusStringVariantMap );
Q_DECLARE_METATYPE( KSecretsService::DBusSecretStruct )

#endif // KSECRETSSERVICEDBUSTYPES_H

