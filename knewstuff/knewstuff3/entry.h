/*
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_KNEWSTUFFENTRY_H
#define KNEWSTUFF3_KNEWSTUFFENTRY_H

#include <QtCore/QStringList>
#include <QtCore/QSharedDataPointer>
#include <kurl.h>

#include <knewstuff3/knewstuff_export.h>


namespace KNS3
{

/**
 * @short KNewStuff information about changed entries
 *
 * This class provides information about the entries that
 * have been installed while the new stuff dialog was shown.
 * It is a minimal version that only gives applications what they need
 * to know.
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT Entry
{
public:
    typedef QList<Entry> List;

    /**
    * Status of the entry. An entry will be downloadable from the provider's
    * site prior to the download. Once downloaded and installed, it will
    * be either installed or updateable, implying an out-of-date
    * installation. Finally, the entry can be deleted and hence show up as
    * downloadable again.
    * Entries not taking part in this cycle, for example those in upload,
    * have an invalid status.
    */
    enum Status {
        Invalid,
        Downloadable,
        Installed,
        Updateable,
        Deleted
    };

    ~Entry();
    Entry(const Entry& other);
    Entry& operator=(const Entry& other);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name
     */
    QString name() const;

    /**
     * Retrieve the category of the data object.
     *
     * @return object category
     */
    QString category() const;

    /**
     * Retrieve the locally installed files.
     * @return file names
     */
    QStringList installedFiles() const;

    /**
     * Retrieve the locally uninstalled files.
     * @return file names
     */
    QStringList uninstalledFiles() const;

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    Status status() const;

    /**
    * Retrieve the license name of the object.
    *
    * @return object license
    */
    QString license() const;

    /**
    * Retrieve a short description about the object.
    *
    * @return object description
    */
    QString summary() const;

    /**
    * Retrieve the version string of the object.
    *
    * @return object version
    */
    QString version() const;

private:
    Entry();

    class Private;
    QExplicitlySharedDataPointer<Private> d;

    friend class EntryInternal;
};

}

#endif
