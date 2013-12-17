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

#include "kitemmodels_export.h"

#include <QItemSelection>
#include <QIdentityProxyModel>

class KCheckableProxyModelPrivate;

/**
 * @brief Adds a checkable capability to a source model
 *
 * Items is standard Qt views such as QTreeView and QListView can have a
 * checkable capability and draw checkboxes. Adding such a capability
 * requires specific implementations of the data() and flags() virtual methods.
 * This class implements those methods generically so that it is not necessary to
 * implement them in your model.
 *
 * This can be combined with a KSelectionProxyModel showing the items currently selected
 *
 * @code
 *
 *   QItemSelectionModel *checkModel = new QItemSelectionModel(rootModel, this);
 *   KCheckableProxyModel *checkable = new KCheckableProxyModel(this);
 *   checkable->setSourceModel(rootModel);
 *   checkable->setSelectionModel(checkModel);
 *
 *   QTreeView *tree1 = new QTreeView(vSplitter);
 *   tree1->setModel(checkable);
 *   tree1->expandAll();
 *
 *   KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(checkModel, this);
 *   selectionProxy->setFilterBehavior(KSelectionProxyModel::ExactSelection);
 *   selectionProxy->setSourceModel(rootModel);
 *
 *   QTreeView *tree2 = new QTreeView(vSplitter);
 *   tree2->setModel(selectionProxy);
 * @endcode
 *
 * @image html kcheckableproxymodel.png "A KCheckableProxyModel and KSelectionProxyModel showing checked items"
 *
 * @since 4.6
 * @author Stephen Kelly <steveire@gmail.com>
 */
class KITEMMODELS_EXPORT KCheckableProxyModel : public QIdentityProxyModel
{
  Q_OBJECT
public:
  KCheckableProxyModel(QObject* parent = 0);
  ~KCheckableProxyModel();

  void setSelectionModel(QItemSelectionModel *itemSelectionModel);
  QItemSelectionModel *selectionModel() const;

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

