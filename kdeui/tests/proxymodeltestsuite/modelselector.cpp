/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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


#include "modelselector.h"

ModelSelector::ModelSelector(ProxyModelTest* parent)
  : ProxyModelTestData(parent),
    m_model(0),
    m_selectionModel(0),
    m_rootModel(0)
{
  Q_ASSERT(parent);
}

void ModelSelector::setWatchedModel(QAbstractItemModel* model)
{
  m_model = model;
  connect(m_model, SIGNAL(destroyed(QObject*)), SLOT(modelDestroyed()));
}

void ModelSelector::setSelectionModel(QItemSelectionModel* selectionModel)
{
  if (selectionModel)
    Q_ASSERT(!selectionModel->hasSelection());
  m_selectionModel = selectionModel;
  connect(m_selectionModel, SIGNAL(destroyed(QObject*)), SLOT(modelDestroyed()));
}

void ModelSelector::setRootModel(DynamicTreeModel* rootModel)
{
  m_rootModel = rootModel;
}

void ModelSelector::setWatch(bool watch)
{
  if (!m_model)
    return;

  disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
             this, SLOT(rowsInserted(const QModelIndex &, int, int)));
  if (watch)
  {
    Q_ASSERT(m_model);
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            SLOT(rowsInserted(const QModelIndex &, int, int)));
    if (m_model->hasChildren())
      rowsInserted(QModelIndex(), 0, m_model->rowCount() - 1);
  }
}

void ModelSelector::rowsInserted(const QModelIndex &parent, int start, int end)
{
  Q_ASSERT(end >= start);
  Q_ASSERT(m_selectionModel);

  int row = start;
  static const int column = 0;
  QModelIndex idx = m_model->index(row, column, parent);

  while (idx.isValid() && row <= end)
  {
    int item = idx.data().toInt();
    if (m_selectedRows.contains(item))
    {
      m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);
    }
    if (m_model->hasChildren(idx))
    {
      rowsInserted(idx, 0, m_model->rowCount(idx) - 1);
    }
    idx = idx.sibling(++row, column);
  }
}

