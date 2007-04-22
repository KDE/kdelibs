/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <faure@kde.org>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef KFILESHAREPRIVATE_H
#define KFILESHAREPRIVATE_H

#include <QObject>
class KDirWatch;

/**
 * @internal
 * Do not use, ever.
 */
class KFileSharePrivate : public QObject
{
  Q_OBJECT

public:
  KFileSharePrivate();
  ~KFileSharePrivate();
  KDirWatch* m_watchFile;
  static KFileSharePrivate *self();
protected Q_SLOTS: // this is why this class needs to be in the .h
 void slotFileChange(const QString &);
};

#endif

