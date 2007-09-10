/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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

#ifndef KSERVICEGROUPPRIVATE_H
#define KSERVICEGROUPPRIVATE_H

#include "kservicegroup.h"
#include <ksycocaentry_p.h>

#include <QtCore/QStringList>

class KServiceGroupPrivate : public KSycocaEntryPrivate
{
public:
    K_SYCOCATYPE( KST_KServiceGroup, KSycocaEntryPrivate )

    KServiceGroupPrivate(const QString &path, KServiceGroup *q_) 
        : KSycocaEntryPrivate(path), q(q_),
          m_bNoDisplay(false), m_bShowEmptyMenu(false), m_bShowInlineHeader(false), m_bInlineAlias(false),
          m_bAllowInline(false), m_inlineValue(4)
    {
    }

    KServiceGroupPrivate(QDataStream &str, int offset, KServiceGroup *q_) 
        : KSycocaEntryPrivate(str, offset), q(q_),
          m_bNoDisplay(false), m_bShowEmptyMenu(false), m_bShowInlineHeader(false), m_bInlineAlias(false),
          m_bAllowInline(false), m_inlineValue(4)
    {
    }

    virtual void save(QDataStream &s);

    virtual QString name() const
    {
        return q->entryPath();
    }


    KServiceGroup *q;
    bool m_bNoDisplay;
    bool m_bShowEmptyMenu;
    bool m_bShowInlineHeader;
    bool m_bInlineAlias;
    bool m_bAllowInline;
    int m_inlineValue;
    QStringList suppressGenericNames;
    QString directoryEntryPath;
    QStringList sortOrder;
};


#endif
