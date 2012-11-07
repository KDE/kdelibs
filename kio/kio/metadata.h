/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KIO_METADATA_H
#define KIO_METADATA_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <kio/kio_export.h>

namespace KIO
{

/**
 * MetaData is a simple map of key/value strings.
 */
class KIO_EXPORT MetaData : public QMap<QString, QString>
{
public:
    /**
     * Creates an empty meta data map.
     */
    MetaData() : QMap<QString, QString>() { }
    /**
     * Copy constructor.
     */
    MetaData(const QMap<QString, QString>&metaData) :
        QMap<QString, QString>(metaData) { }

    /**
     * Creates a meta data map from a QVaraint map.
     * @since 4.3.1
     */
    MetaData(const QMap<QString,QVariant> &);

    /**
     * Adds the given meta data map to this map.
     * @param metaData the map to add
     * @return this map
     */
    MetaData & operator += ( const QMap<QString,QString> &metaData )
    {
        QMap<QString,QString>::ConstIterator it;
        for(it = metaData.constBegin(); it !=  metaData.constEnd(); ++it)
        {
            insert(it.key(), it.value());
        }
        return *this;
    }

    /**
     * Same as above except the value in the map is a QVariant.
     *
     * This convenience function allows you to easily assign the values
     * of a QVariant to this meta data class.
     *
     * @param metaData the map to add
     * @return this map
     * @since 4.3.1
     */
    MetaData & operator += ( const QMap<QString,QVariant> &metaData );

    /**
     * Sets the given meta data map to this map.
     * @param metaData the map to add
     * @return this map
     * @since 4.3.1
     */
    MetaData & operator = ( const QMap<QString,QVariant> &metaData );

    /**
     * Returns the contents of the map as a QVariant.
     *
     * @return a QVariant representation of the meta data map.
     * @since 4.3.1
     */
    QVariant toVariant() const;
};

} // namespace KIO

#endif /* KIO_METADATA_H */
