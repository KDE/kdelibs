/* This file is part of the KDE libraries
    Copyright (c) 2006 Thiago Macieira <thiagokde.org>

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

#ifndef KUNIQUEAPPLICATION_P_H
#define KUNIQUEAPPLICATION_P_H

#include <QtDBus/QtDBus>

class KUniqueApplicationAdaptor: public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KUniqueApplication")
public:
  KUniqueApplicationAdaptor(KUniqueApplication *parent)
    : QDBusAbstractAdaptor(parent)
  { }

  inline KUniqueApplication *parent() const
  { return static_cast<KUniqueApplication *>(QDBusAbstractAdaptor::parent()); }

public slots:
  int newInstance(const QByteArray &asn_id = QByteArray())
  {
    if (!asn_id.isEmpty())
      parent()->setStartupId(asn_id);
    return parent()->newInstance();
  }
};

#endif

/*
 * Local variables:
 *  c-basic-offset: 2
 *  indent-tabs-mode: nil
 * End:
 */
