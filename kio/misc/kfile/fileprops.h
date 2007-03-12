/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFILEPROPS_H
#define KFILEPROPS_H

#include <QtCore/QString>

#include <kfilemetainfo.h>

class FileProps
{
public:
    FileProps( const QString& path);
    virtual ~FileProps();

    bool isValid() const;

    QString fileName() const { return m_info->url().path(); }
 
    QStringList supportedKeys() const;
    QStringList availableKeys() const;

    QString getValue( const QString& key ) const;
    bool setValue( const QString& key, const QString &value );

    QStringList allValues() const;

    bool isReadOnly( const QString& key );

private:
    static QString createKeyValue( const KFileMetaInfoItem& item );
    static QStringList createKeyValueList( const KFileMetaInfoItemList& items);
    bool sync();

    KFileMetaInfo *m_info;
    bool m_dirty;

};

#endif // KFILEPROPS_H
