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


#include "dynamictreemodel.h"

#include <QHash>
#include <QList>
#include <QTimer>

#include <QDebug>

#include <kdebug.h>

DynamicTreeModel::DynamicTreeModel(QObject *parent)
  : QAbstractItemModel(parent),
    nextId(1)
{
}

QModelIndex DynamicTreeModel::index(int row, int column, const QModelIndex &parent) const
{
//   if (column != 0)
//     return QModelIndex();

  if ( column < 0 || row < 0 )
    return QModelIndex();

  QList<QList<qint64> > childIdColumns = m_childItems.value(parent.internalId());


  if (childIdColumns.size() == 0)
    return QModelIndex();

  if (column >= childIdColumns.size())
    return QModelIndex();

  QList<qint64> rowIds = childIdColumns.at(column);

  if ( row >= rowIds.size())
    return QModelIndex();

  qint64 id = rowIds.at(row);

  return createIndex(row, column, reinterpret_cast<void *>(id));

}

qint64 DynamicTreeModel::findParentId(qint64 searchId) const
{
  if (searchId <= 0)
    return -1;

  QHashIterator<qint64, QList<QList<qint64> > > i(m_childItems);
  while (i.hasNext())
  {
    i.next();
    QListIterator<QList<qint64> > j(i.value());
    while (j.hasNext())
    {
      QList<qint64> l = j.next();
      if (l.contains(searchId))
      {
        return i.key();
      }
    }
  }
  return -1;
}

QModelIndex DynamicTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  qint64 searchId = index.internalId();
  qint64 parentId = findParentId(searchId);
  // Will never happen for valid index, but what the hey...
  if (parentId <= 0)
    return QModelIndex();

  qint64 grandParentId = findParentId(parentId);
  if (grandParentId < 0)
    grandParentId = 0;

  int column = 0;
  QList<qint64> childList = m_childItems.value(grandParentId).at(column);

  int row = childList.indexOf(parentId);

  return createIndex(row, column, reinterpret_cast<void *>(parentId));

}

int DynamicTreeModel::rowCount(const QModelIndex &index ) const
{
  QList<QList<qint64> > cols = m_childItems.value(index.internalId());

  if (cols.size() == 0 )
    return 0;

  if (index.column() > 0)
    return 0;

  return cols.at(0).size();
}

int DynamicTreeModel::columnCount(const QModelIndex &index ) const
{
//   Q_UNUSED(index);
  return m_childItems.value(index.internalId()).size();
}

QVariant DynamicTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (Qt::DisplayRole == role)
  {
    return m_items.value(index.internalId());
  }
  return QVariant();
}

void DynamicTreeModel::clear()
{
  m_items.clear();
  m_childItems.clear();
  nextId = 1;
  reset();
}


ModelChangeCommand::ModelChangeCommand( DynamicTreeModel *model, QObject *parent )
    : QObject(parent), m_model(model), m_numCols(1), m_startRow(-1), m_endRow(-1)
{

}

QModelIndex ModelChangeCommand::findIndex(QList<int> rows)
{
  const int col = 0;
  QModelIndex parent = QModelIndex();
  QListIterator<int> i(rows);
  while (i.hasNext())
  {
    parent = m_model->index(i.next(), col, parent);
    Q_ASSERT(parent.isValid());
  }
  return parent;
}

ModelInsertCommand::ModelInsertCommand(DynamicTreeModel *model, QObject *parent )
    : ModelChangeCommand(model, parent)
{

}

void ModelInsertCommand::doCommand()
{
  QModelIndex parent = findIndex(m_rowNumbers);
  m_model->beginInsertRows(parent, m_startRow, m_endRow);
  qint64 parentId = parent.internalId();
  for (int row = m_startRow; row <= m_endRow; row++)
  {
    for(int col = 0; col < m_numCols; col++ )
    {
      if (m_model->m_childItems[parentId].size() <= col)
      {
        m_model->m_childItems[parentId].append(QList<qint64>());
      }
      qint64 id = m_model->newId();
      QString name = QString::number(id);

      m_model->m_items.insert(id, name);
      m_model->m_childItems[parentId][col].insert(row, id);

    }
  }
  m_model->endInsertRows();
}


ModelInsertAndRemoveQueuedCommand::ModelInsertAndRemoveQueuedCommand(DynamicTreeModel* model, QObject* parent)
    : ModelChangeCommand(model, parent)
{
  qRegisterMetaType<QModelIndex>("QModelIndex");
}

void ModelInsertAndRemoveQueuedCommand::queuedBeginInsertRows(const QModelIndex& parent, int start, int end)
{
  m_model->beginInsertRows(parent, start, end);
}

void ModelInsertAndRemoveQueuedCommand::queuedEndInsertRows()
{
  m_model->endInsertRows();
}

void ModelInsertAndRemoveQueuedCommand::queuedBeginRemoveRows(const QModelIndex& parent, int start, int end)
{
  m_model->beginRemoveRows(parent, start, end);
}

void ModelInsertAndRemoveQueuedCommand::queuedEndRemoveRows()
{
  m_model->endRemoveRows();
}

void ModelInsertAndRemoveQueuedCommand::purgeItem(qint64 parent)
{
  QList<QList<qint64> > childItemRows = m_model->m_childItems.value(parent);

  if (childItemRows.size() > 0)
  {
    for (int col = 0; col < m_numCols; col++)
    {
      QList<qint64> childItems = childItemRows[col];
      foreach(qint64 item, childItems)
      {
        purgeItem(item);
        m_model->m_childItems[parent][col].removeOne(item);
      }
    }
  }
  m_model->m_items.remove(parent);
}

void ModelInsertAndRemoveQueuedCommand::doCommand()
{
  QModelIndex parent = findIndex(m_rowNumbers);

  connect (this, SIGNAL(beginInsertRows(const QModelIndex &, int, int)), SLOT(queuedBeginInsertRows(const QModelIndex &, int, int)), Qt::QueuedConnection);
  connect (this, SIGNAL(endInsertRows()), SLOT(queuedEndInsertRows()), Qt::QueuedConnection);
  connect (this, SIGNAL(beginRemoveRows(const QModelIndex &, int, int)), SLOT(queuedBeginRemoveRows(const QModelIndex &, int, int)), Qt::QueuedConnection);
  connect (this, SIGNAL(endRemoveRows()), SLOT(queuedEndRemoveRows()), Qt::QueuedConnection);

  emit beginInsertRows(parent, m_startRow, m_endRow);
//   m_model->beginInsertRows(parent, m_startRow, m_endRow);
  qint64 parentId = parent.internalId();
  for (int row = m_startRow; row <= m_endRow; row++)
  {
    for(int col = 0; col < m_numCols; col++ )
    {
      if (m_model->m_childItems[parentId].size() <= col)
      {
        m_model->m_childItems[parentId].append(QList<qint64>());
      }
      qint64 id = m_model->newId();
      QString name = QString::number(id);

      m_model->m_items.insert(id, name);
      m_model->m_childItems[parentId][col].insert(row, id);

    }
  }

  emit endInsertRows();
//   m_model->endInsertRows();

  emit beginRemoveRows(parent, m_startRow, m_endRow);
//   m_model->beginRemoveRows(parent, m_startRow, m_endRow);
  for(int col = 0; col < m_numCols; col++ )
  {
    QList<qint64> childItems = m_model->m_childItems.value(parentId).value(col);
    for (int row = m_startRow; row <= m_endRow; row++)
    {
      qint64 item = childItems[row];
      purgeItem(item);
      m_model->m_childItems[parentId][col].removeOne(item);
    }
  }
  emit endRemoveRows();
//   m_model->endRemoveRows();

}

ModelInsertWithDescendantsCommand::ModelInsertWithDescendantsCommand(DynamicTreeModel *model, QObject *parent)
    : ModelInsertCommand(model, parent)
{

}

void ModelInsertWithDescendantsCommand::setFragments(QList<InsertFragment> fragments)
{
  m_fragments = fragments;
}

void ModelInsertWithDescendantsCommand::insertFragment(qint64 parentIdentifier, InsertFragment fragment)
{
  for(int col = 0; col < m_numCols; col++ )
  {
    if (m_model->m_childItems[parentIdentifier].size() <= col)
    {
      m_model->m_childItems[parentIdentifier].append(QList<qint64>());
    }
    for (int row = 0; row < fragment.numRows; row++)
    {
      qint64 id = m_model->newId();
      QString name = QString::number(id);

      m_model->m_items.insert(id, name);
      m_model->m_childItems[parentIdentifier][col].append(id);

      if (col == 0 && fragment.subfragments.contains(row) )
      {
        insertFragment(id, fragment.subfragments.take(row));
      }
    }
  }

}

void ModelInsertWithDescendantsCommand::doCommand()
{
  QModelIndex fragmentParent = findIndex(m_rowNumbers);

  qint64 fragmentParentIdentifier = fragmentParent.internalId();

  const int column = 0;

  qint64 rootIdentifier = m_model->m_childItems[fragmentParentIdentifier][column][m_startRow];
  QModelIndex fragmentIndex = m_model->index(m_startRow, column, fragmentParent);

  InsertFragment fragment = m_fragments.at(0);

  m_model->beginInsertRows(fragmentIndex, 0, fragment.numRows - 1);
  insertFragment(rootIdentifier, fragment);
  m_model->endInsertRows();
}


ModelRemoveCommand::ModelRemoveCommand(DynamicTreeModel *model, QObject *parent )
    : ModelChangeCommand(model, parent)
{

}

void ModelRemoveCommand::doCommand()
{
  QModelIndex parent = findIndex(m_rowNumbers);
  m_model->beginRemoveRows(parent, m_startRow, m_endRow);
  qint64 parentId = parent.internalId();
  for(int col = 0; col < m_numCols; col++ )
  {
    QList<qint64> childItems = m_model->m_childItems.value(parentId).value(col);
    for (int row = m_startRow; row <= m_endRow; row++)
    {
      qint64 item = childItems[row];
      purgeItem(item);
      m_model->m_childItems[parentId][col].removeOne(item);
    }
  }
  m_model->endRemoveRows();
}

void ModelRemoveCommand::purgeItem(qint64 parent)
{
  QList<QList<qint64> > childItemRows = m_model->m_childItems.value(parent);

  if (childItemRows.size() > 0)
  {
    for (int col = 0; col < m_numCols; col++)
    {
      QList<qint64> childItems = childItemRows[col];
      foreach(qint64 item, childItems)
      {
        purgeItem(item);
        m_model->m_childItems[parent][col].removeOne(item);
      }
    }
  }
  m_model->m_items.remove(parent);
}


ModelDataChangeCommand::ModelDataChangeCommand(DynamicTreeModel *model, QObject *parent)
  : ModelChangeCommand(model, parent), m_startColumn(0)
{

}

void ModelDataChangeCommand::doCommand()
{
  QModelIndex parent = findIndex(m_rowNumbers);
  QModelIndex topLeft = m_model->index(m_startRow, m_startColumn, parent);
  QModelIndex bottomRight = m_model->index(m_endRow, m_numCols - 1, parent);

  QList<QList<qint64> > childItems = m_model->m_childItems[parent.internalId()];


  for (int col = m_startColumn; col < m_startColumn + m_numCols; col++)
  {
    for (int row = m_startRow; row <= m_endRow; row++ )
    {
      QString name = QString::number( m_model->newId() );
      m_model->m_items[childItems[col][row]] = name;
    }
  }
  m_model->dataChanged(topLeft, bottomRight);
}


ModelMoveCommand::ModelMoveCommand(DynamicTreeModel *model, QObject *parent)
: ModelChangeCommand(model, parent)
{

}

bool ModelMoveCommand::emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
#ifdef QT_VERSION >= 0x040600
  return m_model->beginMoveRows(srcParent, srcStart, srcEnd, destParent, destRow);
#endif
}

void ModelMoveCommand::doCommand()
{
  QModelIndex srcParent = findIndex(m_rowNumbers);
  QModelIndex destParent = findIndex(m_destRowNumbers);

  if (!emitPreSignal(srcParent, m_startRow, m_endRow, destParent, m_destRow))
  {
    return;
  }

  for (int column = 0; column < m_numCols; ++column)
  {
    QList<qint64> l = m_model->m_childItems.value(srcParent.internalId())[column].mid(m_startRow, m_endRow - m_startRow + 1 );

    for (int i = m_startRow; i <= m_endRow ; i++)
    {
      m_model->m_childItems[srcParent.internalId()][column].removeAt(m_startRow);
    }
    int d;
    if (m_destRow < m_startRow)
      d = m_destRow;
    else
    {
      if (srcParent == destParent)
        d = m_destRow - (m_endRow - m_startRow + 1);
      else
        d = m_destRow - (m_endRow - m_startRow) + 1;
    }

    foreach(const qint64 id, l)
    {
      m_model->m_childItems[destParent.internalId()][column].insert(d++, id);
    }
  }

  emitPostSignal();
}

void ModelMoveCommand::emitPostSignal()
{
#ifdef QT_VERSION >= 0x040600
  m_model->endMoveRows();
#endif
}


ModelMoveLayoutChangeCommand::ModelMoveLayoutChangeCommand(DynamicTreeModel* model, QObject* parent): ModelMoveCommand(model, parent)
{

}

ModelMoveLayoutChangeCommand::~ModelMoveLayoutChangeCommand()
{

}

bool ModelMoveLayoutChangeCommand::emitPreSignal(const QModelIndex& srcParent, int srcStart, int srcEnd, const QModelIndex& destParent, int destRow)
{
  m_model->layoutAboutToBeChanged();

  const int column = 0;

  for (int row = srcStart; row <= srcEnd; ++row)
  {
    m_beforeMoveList << m_model->index(row, column, srcParent);
  }

  if (srcParent != destParent)
  {
    for (int row = srcEnd + 1; row < m_model->rowCount(srcParent); ++row)
    {
      m_beforeMoveList << m_model->index(row, column, srcParent);
    }
    for (int row = destRow; row < m_model->rowCount(destParent); ++row)
    {
      m_beforeMoveList << m_model->index(row, column, destParent);
    }
  } else {
    if (destRow < srcStart)
    {
      for (int row = destRow; row < srcStart; ++row)
      {
        m_beforeMoveList << m_model->index(row, column, srcParent);
      }
    } else {
      for (int row = srcStart + (srcEnd - srcStart + 1); row < destRow; ++row)
      {
        m_beforeMoveList << m_model->index(row, column, srcParent);
      }
    }
  }
}

void ModelMoveLayoutChangeCommand::emitPostSignal()
{
  int srcStart = m_startRow;
  int srcEnd = m_endRow;
  int destRow = m_destRow;

  // Moving indexes may affect the m_rowNumbers and m_destRowNumbers.
  // Instead of adjusting them programmatically, the test writer must specify them if they change.

  const QList<int> sourceRowNumbers = m_endOfMoveSourceAncestors.isEmpty() ? m_rowNumbers : m_endOfMoveSourceAncestors;
  QModelIndex srcParent = findIndex(sourceRowNumbers);

  const QList<int> destRowNumbers = m_endOfMoveDestAncestors.isEmpty() ? m_destRowNumbers : m_endOfMoveDestAncestors;
  QModelIndex destParent = findIndex(destRowNumbers);

  const int column = 0;

  QModelIndexList afterMoveList;

  if (srcParent != destParent)
  {
    for (int row = destRow; row <= (destRow + (srcEnd - srcStart)); ++row)
    {
      afterMoveList << m_model->index(row, column, destParent);
    }
    for (int row = srcStart; row < m_model->rowCount(srcParent); ++row)
    {
      afterMoveList << m_model->index(row, column, srcParent);
    }
    for (int row = destRow + (srcEnd - srcStart + 1); row < m_model->rowCount(destParent); ++row)
    {
      afterMoveList << m_model->index(row, column, destParent);
    }
  } else {
    if (destRow < srcStart)
    {
      for (int row = srcStart; row <= srcEnd; ++row)
      {
        afterMoveList << m_model->index(destRow + (srcStart - row), column, destParent);
      }
    } else {
      for (int row = srcStart; row <= srcEnd; ++row)
      {
        afterMoveList << m_model->index(destRow + (srcStart - row - 1), column, destParent);
      }
    }
    if (destRow < srcStart)
    {
      for (int row = destRow + 1; row <= srcStart; ++row)
      {
        afterMoveList << m_model->index(row, column, srcParent);
      }
    } else {
      for (int row = srcStart + (srcEnd - srcStart + 1); row < (srcStart + (destRow - srcEnd)); ++row)
      {
        afterMoveList << m_model->index(row - (srcEnd - srcStart + 1), column, srcParent);
      }
    }
  }

  m_model->changePersistentIndexList(m_beforeMoveList, afterMoveList);
  m_beforeMoveList.clear();
  m_model->layoutChanged();


}


