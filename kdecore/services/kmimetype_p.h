/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/
#ifndef __kmimetype_p_h__
#define __kmimetype_p_h__

#include "kservicetype_p.h"

class KMimeTypePrivate: public KServiceTypePrivate
{
public:
  K_SYCOCATYPE( KST_KMimeType, KServiceTypePrivate )

  KMimeTypePrivate(const QString &path) : KServiceTypePrivate(path) {}
  KMimeTypePrivate(QDataStream &_str, int offset)
      : KServiceTypePrivate(_str, offset)
  {
    loadInternal(_str);
  }

  virtual void save(QDataStream &s);

  virtual QVariant property(const QString &name ) const;

  virtual QStringList propertyNames() const;

  virtual QString iconName(const KUrl &) const
  {
    QString icon = name();
    int slashindex = icon.indexOf( QLatin1Char( '/' ) );
    if ( slashindex != -1 ) {
        icon[ slashindex ] = QLatin1Char( '-' );
    }
    return icon;
  }

    bool inherits(const QString& mime) const;
    QString fallbackParent() const;
    QStringList parentMimeTypes() const;
    void collectParentMimeTypes(QStringList&) const;

  QStringList m_lstPatterns;
  QStringList m_parentMimeTypes; // shared-mime-info supports multiple inheritance

  void loadInternal( QDataStream& _str );
};

#endif // __kmimetype_p_h__
