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
#ifndef KFILEMETAINFO_H
#define KFILEMETAINFO_H

#include "predicateproperties.h"
#include "kfilemetainfoitem.h"
#include <QtCore/QList>
#include <QtCore/QStringList>
class KUrl;

typedef QList<KFileMetaInfoItem> KFileMetaInfoItemList;

class KFileMetaInfoGroupPrivate;
class KIO_EXPORT KFileMetaInfoGroup {
public:
    KFileMetaInfoGroup();
    KFileMetaInfoGroup(const KFileMetaInfoGroup&);
    ~KFileMetaInfoGroup();
    const KFileMetaInfoGroup& operator=(const KFileMetaInfoGroup&);
    KFileMetaInfoItemList items() const;
    const QString& name() const;
    const QStringList& keys() const;
private:
    QSharedDataPointer<KFileMetaInfoGroupPrivate> p;
};

typedef QList<KFileMetaInfoGroup> KFileMetaInfoGroupList;

class KFileMetaInfoPrivate;
/**
 * KFileMetaInfo provides metadata extracted from a file or other resource.
 *
 * When instantiating an instance of this class, the metadata related to it
 * will be retrieved and stored in the instance. The data can be inspected
 * through KFileMetaInfoItem objects.
 **/
class KIO_EXPORT KFileMetaInfo {
public:
    /**
     * This is used to specify what a KFileMetaInfo object should read, so
     * you can specify if you want to read "expensive" items or not.
     * This is like a preset which can be customized by passing additional
     * parameters to constructors.
     */
    enum What
    {
      Fastest       = 0x1,  /**< do the fastest possible read and omit all items
                                 that might need a significantly longer time
                                 than the others */
// Deprecated
//      DontCare      = 0x2,  ///< let the plugin decide what to read.

      TechnicalInfo = 0x4,  /**< extract technical details about the file, like
                                 e.g. play time, resolution or a compressioni
                                 type */
      ContentInfo   = 0x8,  /**< read information about the content of the file
                                 like comments or id3 tags */
      ExternalSources = 0x10, /**<read external metadata sources such as
                                 filesystem based extended attributes if
                                 they are supported for the filesystem;
                                 RDF storages etc */
      Thumbnail     = 0x20, /**< only read the file's thumbnail, if it contains
                                 one */
// Deprecated
//      Preferred     = 0x40,  ///< get at least the preferred items
      LinkedData    = 0x80, //< extract linked/related files like html links, source #include etc
      Everything    = 0xffff ///< read everything, even if it might take a while

    };
    Q_DECLARE_FLAGS(WhatFlags, What)

    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p path.
     *
     * When w is not Everything, a limit of 64kbytes is imposed on the file size.
     **/
    explicit KFileMetaInfo(const QString& path, const QString& mimetype = QString(),
        WhatFlags w = Everything);
    /**
     * @brief Construct a KFileMetaInfo that contains metainformation about
     * the resource pointed to by @p url.
     * @note that c'tor is not thread-safe
     **/
    KFileMetaInfo(const KUrl& url);
    /**
     * @brief Construct an empty, invalid KFileMetaInfo instance.
     **/
    KFileMetaInfo();
    /**
     * @brief Construct a KFileMetaInfo instance from another one.
     **/
    KFileMetaInfo(const KFileMetaInfo&);
    /**
     * @brief Destructor.
     **/
    ~KFileMetaInfo();
    /**
     * @brief Copy a KFileMetaInfo instance from another one.
     **/
    const KFileMetaInfo& operator=(KFileMetaInfo const& kfmi);
    /**
     * @brief Save the changes made to this KFileMetaInfo instance.
     */
    bool applyChanges();
    /**
     * @brief Retrieve all the items.
     **/
    const QHash<QString, KFileMetaInfoItem>& items() const;
    KFileMetaInfoItem& item(const QString& key);
    const KFileMetaInfoItem& item(const QString& key) const;
    bool isValid() const;
    /**
     * Deprecated
     **/
    QStringList preferredKeys() const;
    /**
     * Deprecated
     **/
    QStringList supportedKeys() const;
    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& )
;
    KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo&);
    /**
     * Deprecated
     **/
    KDE_DEPRECATED KFileMetaInfoGroupList preferredGroups() const;
    /**
     * Deprecated
     **/
    KDE_DEPRECATED KFileMetaInfoGroupList supportedGroups() const;
    KFileMetaInfoGroupList groups() const;
    QStringList keys() const;
    const KUrl& url() const;

private:
    QSharedDataPointer<KFileMetaInfoPrivate> p;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFileMetaInfo::WhatFlags)


#endif
