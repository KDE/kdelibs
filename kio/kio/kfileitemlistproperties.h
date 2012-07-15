/* This file is part of the KDE project
   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
   Copyright (C) 2008 by George Goldberg <grundleborg@googlemail.com>
   Copyright     2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KFILEITEMLISTPROPERTIES_H
#define KFILEITEMLISTPROPERTIES_H

#include <kio/kio_export.h>

#include <kurl.h>
#include <QtCore/QSharedDataPointer>

class KFileItemListPropertiesPrivate;
class KFileItemList;

/**
 * @brief Provides information about the common properties of a group of
 *        KFileItem objects.
 *
 * Given a list of KFileItems, this class can determine (and cache) the common
 * mimetype for all items, whether all items are directories, whether all items
 * are readable, writable, etc.
 * As soon as one file item does not support a specific capability (read, write etc.),
 * it is marked as unsupported for all items.
 *
 * This class is implicitly shared, which means it can be used as a value and
 * copied around at almost no cost.
 *
 * @since 4.3
 */
class KIO_EXPORT KFileItemListProperties
{
public:
    /**
     * @brief Default constructor. Use setItems to specify the items.
     */
    KFileItemListProperties();
    /**
     * @brief Constructor that takes a KFileItemList and sets the capabilities
     *        supported by all the FileItems as true.
     * @param items The list of items that are to have their supported
     *              capabilities checked.
     */
    KFileItemListProperties(const KFileItemList& items);
    /**
     * @brief Copy constructor
     */
    KFileItemListProperties(const KFileItemListProperties&);
    /**
     * @brief Destructor
     */
    virtual ~KFileItemListProperties();
    /**
     * @brief Assignment operator
     */
    KFileItemListProperties& operator=(const KFileItemListProperties& other);
    /**
     * Sets the items that are to have their supported capabilities checked.
     */
    void setItems(const KFileItemList& items);

    /**
     * @brief Check if reading capability is supported
     * @return true if all the FileItems can be read, otherwise false.
     */
    bool supportsReading() const;
    /**
     * @brief Check if deleting capability is supported
     * @return true if all the FileItems can be deleted, otherwise false.
     */
    bool supportsDeleting() const;
    /**
     * @brief Check if writing capability is supported
     * (file managers use this mostly for directories)
     * @return true if all the FileItems can be written to, otherwise false.
     */
    bool supportsWriting() const;
    /**
     * @brief Check if moving capability is supported
     * @return true if all the FileItems can be moved, otherwise false.
     */
    bool supportsMoving() const;
    /**
     * @brief Check if files are local
     * @return true if all the FileItems are local, otherwise there is one or more
     *         remote file, so false.
     */
    bool isLocal() const;

    /**
     * List of fileitems passed to the constructor or to setItems().
     */
    KFileItemList items() const;

    /**
     * List of urls, gathered from the fileitems
     */
    QList<QUrl> urlList() const;

    /**
     * @return true if all items are directories
     */
    bool isDirectory() const;

    /**
     * @return the mimetype of all items, if they all have the same, otherwise empty
     */
    QString mimeType() const;

    /**
     * @return the mimetype group (e.g. "text") of all items, if they all have the same, otherwise empty
     */
    QString mimeGroup() const;

private:
    /** @brief d-pointer */
    QSharedDataPointer<KFileItemListPropertiesPrivate> d;
};

#endif /* KFILEITEMLISTPROPERTIES_H */

