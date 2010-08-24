/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef KCHECKABLEPROXYMODEL_H
#define KCHECKABLEPROXYMODEL_H

#include "kidentityproxymodel.h"

#include "kdeui_export.h"

#include <QItemSelection>

class KCheckableProxyModelPrivate;

class KDEUI_EXPORT KCheckableProxyModel : public Future::KIdentityProxyModel
{
  Q_OBJECT
public:
  KCheckableProxyModel(QObject* parent = 0);
  ~KCheckableProxyModel();

  void setSelectionModel(QItemSelectionModel *itemSelectionModel);

  /* reimp */ Qt::ItemFlags flags(const QModelIndex& index) const;

  /* reimp */ QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  /* reimp */ bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  /* reimp */ void setSourceModel(QAbstractItemModel* sourceModel);

protected:
  virtual bool select( const QItemSelection &selection, QItemSelectionModel::SelectionFlags command );

private:
  Q_DECLARE_PRIVATE(KCheckableProxyModel )
  KCheckableProxyModelPrivate * const d_ptr;

  Q_PRIVATE_SLOT(d_func(), void selectionChanged(const QItemSelection &, const QItemSelection &) )
};

#endif

