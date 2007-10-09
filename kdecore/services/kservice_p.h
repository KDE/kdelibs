/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     1999-2006  David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
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

#ifndef KSERVICEPRIVATE_H
#define KSERVICEPRIVATE_H

#include "kservice.h"

#include <ksycocaentry_p.h>

class KServicePrivate : public KSycocaEntryPrivate
{
public:
    K_SYCOCATYPE( KST_KService, KSycocaEntryPrivate )

    KServicePrivate(const QString &path)
        : KSycocaEntryPrivate(path),  m_bValid(true)
    {
    }
    KServicePrivate(QDataStream& _str, int _offset)
        : KSycocaEntryPrivate(_str, _offset), m_bValid(true)
    {
        load(_str);
    }

    void init(const KDesktopFile *config, KService* q);
    void load( QDataStream& );
    virtual void save( QDataStream& );

    virtual QString name() const
    {
        return m_strName;
    }

    virtual bool isValid() const
    {
        return m_bValid;
    }

    virtual QVariant property(const QString &name) const;

    virtual QStringList propertyNames() const;

    QVariant property(const QString &_name, QVariant::Type t) const;

    QStringList categories;
    QString menuId;
    QString m_strType;
    QString m_strName;
    QString m_strExec;
    QString m_strIcon;
    QString m_strTerminalOptions;
    QString m_strPath;
    QString m_strComment;
    QString m_strLibrary;
    QStringList m_lstServiceTypes;
    int m_initialPreference;
    QString m_strDesktopEntryName;
    KService::DBusStartupType m_DBUSStartusType;
    QMap<QString,QVariant> m_mapProps;
    QStringList m_lstKeywords;
    QString m_strGenName;
    bool m_bAllowAsDefault : 1;
    bool m_bTerminal : 1;
    bool m_bValid : 1;
};
#endif
