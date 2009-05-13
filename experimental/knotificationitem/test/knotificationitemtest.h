/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFICATIONITEMTEST_H
#define KNOTIFICATIONITEMTEST_H

#include <QObject>

namespace Experimental
{
    class KNotificationItem;
} // namespace Experitmental

class KNotificationItemTest : public QObject
{
  Q_OBJECT

public:
    KNotificationItemTest(QObject *parent, Experimental::KNotificationItem *tray);
    //~KNotificationItemTest();

public Q_SLOTS:
    void setNeedsAttention();
    void setActive();
    void setPassive();
private:
    Experimental::KNotificationItem *m_tray;
};

#endif
