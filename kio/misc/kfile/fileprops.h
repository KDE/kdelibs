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

#include <qstring.h>

#include <kfilemetainfo.h>

class FileProps
{
public:
    FileProps( const QString& path, const QStringList& suppliedGroups );
    virtual ~FileProps();

    bool isValid() const;

    QString fileName() const { return m_info->path(); }
    
    QStringList supportedGroups() const;
    QStringList availableGroups() const;
    QStringList translatedGroups();

    const QStringList& groupsToUse() const { return m_groupsToUse; }
    bool userSuppliedGroups() const { return m_userSuppliedGroups; }

    QStringList supportedKeys( const QString& group ) const;
    QStringList availableKeys( const QString& group ) const;
    QStringList preferredKeys( const QString& group ) const;

    QStringList supportedKeys() const { return m_info->supportedKeys(); }
    QStringList preferredKeys() const { return m_info->preferredKeys(); }

    QString getValue( const QString& group, const QString& key ) const;
    bool setValue( const QString& group,
                   const QString& key, const QString &value );

    QStringList allValues( const QString& group ) const;
    QStringList preferredValues( const QString& group ) const;

    bool isReadOnly( const QString& group, const QString& key );

private:
    static QString createKeyValue( const KFileMetaInfoGroup& g,
                                   const QString& key );
    static QStringList createKeyValueList( const KFileMetaInfoGroup&,
                                           const QStringList& );
    bool sync();

    KFileMetaInfo *m_info;
    bool m_dirty;
    bool m_userSuppliedGroups;

    QStringList m_groupsToUse;

};

#endif // KFILEPROPS_H
