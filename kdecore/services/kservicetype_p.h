/* This file is part of the KDE project
Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
1999 Waldo Bastian <bastian@kde.org>

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

#ifndef __kservicetype_p_h__
#define __kservicetype_p_h__

class KServiceTypePrivate
{
public:
    Q_DECLARE_PUBLIC(KServiceType)

    KServiceTypePrivate(KServiceType *q)
      : q_ptr(q), m_serviceOffersOffset( -1 )
    {
        m_bValid = false;
        m_bDerived = false;
        m_parentTypeLoaded = false;
    }
  
    virtual ~KServiceTypePrivate() {}
    
    void init( KDesktopFile *config );

    KServiceType *q_ptr;
    KServiceType::Ptr parentType;
    QString m_strName;
    QString m_strComment;
    int m_serviceOffersOffset;
    QMap<QString, QVariant::Type> m_mapPropDefs;
    QMap<QString,QVariant> m_mapProps;
    unsigned m_bValid: 1;
    unsigned m_bDerived: 1;
    unsigned m_parentTypeLoaded: 1;
};

#endif // __kservicetype_p_h__

