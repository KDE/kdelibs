/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MODELDUMPER_H
#define MODELDUMPER_H

#include <QIODevice>

#include "proxymodeltestsuite_export.h"

class QModelIndex;
class QAbstractItemModel;

class PROXYMODELTESTSUITE_EXPORT ModelDumper
{
public:
  ModelDumper();

  QString dumpModel(const QAbstractItemModel * const model) const;
  QString dumpTree(const QAbstractItemModel * const model, const QModelIndex &index) const;
  QString dumpTree(const QAbstractItemModel * const model, const QModelIndex &index, int start, int end) const;

  void dumpModel(const QAbstractItemModel * const model, QIODevice *device) const;
  void dumpTree(const QAbstractItemModel * const model, QIODevice *device, const QModelIndex &index) const;
  void dumpTree(const QAbstractItemModel * const model, QIODevice *device, const QModelIndex &index, int start, int end) const;

private:
  QString dumpLevel(const QAbstractItemModel * const model, const QModelIndex &parent, int level) const;
  QString dumpLevel(const QAbstractItemModel * const model, const QModelIndex &parent, int level, int start, int end) const;

};

#endif
