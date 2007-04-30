/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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
#ifndef KFILEMETAINFOITEM_H
#define KFILEMETAINFOITEM_H

#include <kio/kio_export.h>
#include <QtCore/QSharedData>

class KFileWritePlugin;
class QVariant;
class PredicateProperties;

class KFileMetaInfoItemPrivate;
class KIO_EXPORT KFileMetaInfoItem {
friend class KFileMetaInfo;
friend class KFileMetaInfoPrivate;
friend class KMetaInfoWriter;
public:
    /**
     * @brief Default constructor
     **/
    KFileMetaInfoItem();
    /**
     * @brief Copy constructor
     **/
    KFileMetaInfoItem(const KFileMetaInfoItem& item);
    /**
     * @brief Destructor
     **/
    ~KFileMetaInfoItem();
    /**
     * @brief Copy operator
     **/
    const KFileMetaInfoItem& operator=(const KFileMetaInfoItem& item);
    /**
     * @brief Can this item be edited.
     **/
    bool isEditable() const;
    /**
     * @brief Has this value been removed, e.g with setValue(QVarian())
     **/
    bool isRemoved() const;
    /**
     * @brief Can this item be removed.
     **/
    bool isModified() const;
    /**
     * @brief Has this value been skipped by analyzer due to performance or other considerations.
     **/
    bool isSkipped() const;
    /**
     * @brief Retrieve the current value of this item
     **/
    const QVariant& value() const;
    /**
     * @brief Set the value of this field.
     **/
    bool setValue(const QVariant& value);
    /**
     * @brief Append a value to the properties in this file.
     **/
    bool addValue(const QVariant&);
    /**
     * @brief Is this a valid item.
     **/
    bool isValid() const;
    /**
     * @brief Retrieve the properties of this field type.
     **/
    const PredicateProperties& properties() const;
    /**
     * Localized name of the predicate.
     * Convenience function for properties().name();
     **/
    const QString& name() const;
    /**
     * This method returns a translated suffix to be displayed after the
     * value. Think of the kbps in 128kbps
     *
     * @return the suffix
     */
    QString suffix() const;
    /**
     * This method returns a translated prefix to be displayed before the
     * value.
     *
     * @return the prefix
     */
    QString prefix() const;
private:
    QSharedDataPointer<KFileMetaInfoItemPrivate> p;

    KFileMetaInfoItem(const PredicateProperties& p,
        const QVariant& v, KFileWritePlugin* w, bool e);
};

#endif
