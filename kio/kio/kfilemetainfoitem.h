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

#include "kdelibs_export.h"
#include <QSharedData>

class KFileWritePlugin;
class QVariant;
class PredicateProperties;

class KIO_EXPORT KFileMetaInfoItem {
friend class KFileMetaInfo;
friend class KMetaInfoWriter;
public:
    KFileMetaInfoItem();
    KFileMetaInfoItem(const KFileMetaInfoItem& item);
    ~KFileMetaInfoItem();
    const KFileMetaInfoItem& operator=(const KFileMetaInfoItem& item);
    bool setValue(const QVariant& value);
    bool isEditable() const;
    bool isRemoved() const;
    bool isModified() const;
    const QVariant& value() const;
    bool addValue(const QVariant&);
    bool isValid() const;
    const PredicateProperties& properties() const;
    /**
     * Localized name of the predicate.
     * Convenience function for properties().name();
     **/
    const QString& name() const;
private:
    class Private;
    QSharedDataPointer<Private> p;

    void setWriter(KFileWritePlugin* w);

    KFileMetaInfoItem(const PredicateProperties& p,
        const QVariant& v, KFileWritePlugin* w, bool e);
};

#endif
