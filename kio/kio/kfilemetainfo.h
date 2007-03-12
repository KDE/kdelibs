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
#include <QList>
#include <QStringList>
class KUrl;

class KFileMetaInfoItem;
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
class KIO_EXPORT KFileMetaInfo {
public:
    /**
     * This is used to specify what a KFileMetaInfo object should read, so
     * you can specify if you want to read "expensive" items or not.
     */
    enum What
    {
      Fastest       = 0x1,  /**< do the fastest possible read and omit all item
s
                                 that might need a significantly longer time
                                 than the others */
      DontCare      = 0x2,  ///< let the plugin decide what to read

      TechnicalInfo = 0x4,  /**< extract technical details about the file, like
                                 e.g. play time, resolution or a compressioni
                                 type */
      ContentInfo   = 0x8,  /**< read information about the content of the file
,
                                 like comments or id3 tags */
      ExtenedAttr   = 0x10, /**< read filesystem based extended attributes if
                                 they are supported for the filesystem */
      Thumbnail     = 0x20, /**< only read the file's thumbnail, if it contains
                                 one */
      Preferred     = 0x40,  ///< get at least the preferred items
      Everything    = 0xffff ///< read everything, even if it might take a while

    };
    Q_DECLARE_FLAGS(WhatFlags, What)

    KFileMetaInfo(const QString& path, const QString& mimetype = QString(),
        WhatFlags w = Everything);
    KFileMetaInfo(const KUrl& url);
    KFileMetaInfo();
    KFileMetaInfo(const KFileMetaInfo&);
    ~KFileMetaInfo();
    const KFileMetaInfo& operator=(KFileMetaInfo const& kfmi);
    bool applyChanges();
    const QHash<QString, KFileMetaInfoItem>& items() const;
    KFileMetaInfoItem& item(const QString& key);
    const KFileMetaInfoItem& item(const QString& key) const;
    bool isValid() const;
    QStringList preferredKeys() const;
    QStringList supportedKeys() const;
    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& )
;
    KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo&);
    KFileMetaInfoGroupList preferredGroups() const;
/*    KFileMetaInfoGroupList supportedGroups() const {
        return KFileMetaInfoGroupList();
    }*/
    KFileMetaInfoGroupList groups() const;
    QStringList keys() const;
    const KUrl& url() const;

private:
    QSharedDataPointer<KFileMetaInfoPrivate> p;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFileMetaInfo::WhatFlags)


#endif
