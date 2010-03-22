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

#include "modelspy.h"

#include <kdebug.h>

ModelSpy::ModelSpy(QObject *parent)
  : QObject(parent), QList<QVariantList>(), m_model(0), m_isSpying(false), m_lazyPersist(false)
{
  qRegisterMetaType<QModelIndex>("QModelIndex");
}

void ModelSpy::setModel(QAbstractItemModel *model)
{
  stopSpying();
  m_model = model;
}

void ModelSpy::clearTestData()
{
  m_changeList.clear();
  m_unchangedIndexes.clear();
  m_unchangedPersistentIndexes.clear();
}

void ModelSpy::startSpying()
{

  // If a signal is connected to a slot multiple times, the slot gets called multiple times.
  // As we're doing start and stop spying all the time, we disconnect here first to make sure.
  stopSpying();

  m_isSpying = true;

  connect(m_model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(rowsAboutToBeInserted(const QModelIndex &, int, int)));
  connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(rowsInserted(const QModelIndex &, int, int)));
  connect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(rowsRemoved(const QModelIndex &, int, int)));
  connect(m_model, SIGNAL(layoutAboutToBeChanged()),
          SLOT(layoutAboutToBeChanged()));
  connect(m_model, SIGNAL(layoutChanged()),
          SLOT(layoutChanged()));
  connect(m_model, SIGNAL(modelAboutToBeReset()),
          SLOT(modelAboutToBeReset()));
  connect(m_model, SIGNAL(modelReset()),
          SLOT(modelReset()));
  connect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          SLOT(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          SLOT(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));

  connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));

  connect(m_model, SIGNAL(destroyed()), SLOT(modelDestroyed()));
}


void ModelSpy::stopSpying()
{
  m_isSpying = false;
  if (!m_model)
    return;

  disconnect(m_model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          this, SLOT(rowsAboutToBeInserted(const QModelIndex &, int, int)));
  disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          this, SLOT(rowsInserted(const QModelIndex &, int, int)));
  disconnect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          this, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
  disconnect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
  disconnect(m_model, SIGNAL(layoutAboutToBeChanged()),
          this, SLOT(layoutAboutToBeChanged()));
  disconnect(m_model, SIGNAL(layoutChanged()),
          this, SLOT(layoutChanged()));
  disconnect(m_model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(modelAboutToBeReset()));
  disconnect(m_model, SIGNAL(modelReset()),
          this, SLOT(modelReset()));
  disconnect(m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int,const QModelIndex &, int)),
          this, SLOT(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
  disconnect(m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
          this, SLOT(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));

  disconnect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));

  disconnect(m_model, SIGNAL(destroyed(QObject *)), this, SLOT(modelDestroyed()));
}

void ModelSpy::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  append(QVariantList() << RowsAboutToBeInserted << QVariant::fromValue(parent) << start << end);

  if (m_lazyPersist)
    doPersist();
}

void ModelSpy::rowsInserted(const QModelIndex &parent, int start, int end)
{
  append(QVariantList() << RowsInserted << QVariant::fromValue(parent) << start << end);
}

void ModelSpy::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  append(QVariantList() << RowsAboutToBeRemoved << QVariant::fromValue(parent) << start << end);

  if (m_lazyPersist)
    doPersist();
}

void ModelSpy::rowsRemoved(const QModelIndex &parent, int start, int end)
{
  append(QVariantList() << RowsRemoved << QVariant::fromValue(parent) << start << end);
}

void ModelSpy::layoutAboutToBeChanged()
{
  append(QVariantList() << LayoutAboutToBeChanged);

  if (m_lazyPersist)
    doPersist();
}

void ModelSpy::layoutChanged()
{
  append(QVariantList() << LayoutChanged);
}

void ModelSpy::modelAboutToBeReset()
{
  append(QVariantList() << ModelAboutToBeReset);

  // This is called in setSourceModel for example, which is not when we want to persist.
  if (m_lazyPersist && m_model->hasChildren())
    doPersist();
}

void ModelSpy::modelReset()
{
  append(QVariantList() << ModelReset);
}

void ModelSpy::modelDestroyed()
{
  stopSpying();
  m_model = 0;
}

void ModelSpy::rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart)
{
  append(QVariantList() << RowsAboutToBeMoved << QVariant::fromValue(srcParent) << start << end << QVariant::fromValue(destParent) << destStart);

  // Don't do a lazy persist here. That will be done on the layoutAboutToBeChanged signal.
//   if (m_lazyPersist)
//     doPersist();
}

void ModelSpy::rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destStart)
{
  append(QVariantList() << RowsMoved << QVariant::fromValue(srcParent) << start << end << QVariant::fromValue(destParent) << destStart);
}

void ModelSpy::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  append(QVariantList() << DataChanged << QVariant::fromValue(topLeft) << QVariant::fromValue(bottomRight));
}

QModelIndexList ModelSpy::getDescendantIndexes(const QModelIndex &parent)
{
  QModelIndexList list;
  const int column = 0;
  for(int row = 0; row < m_model->rowCount(parent); ++row)
  {
    QModelIndex idx = m_model->index(row, column, parent);
    list << idx;
    list << getDescendantIndexes(idx);
  }
  return list;
}

QList< QPersistentModelIndex > ModelSpy::toPersistent(QModelIndexList list)
{
  QList<QPersistentModelIndex > persistentList;
  foreach(QModelIndex idx, list)
  {
    persistentList << QPersistentModelIndex(idx);
  }
  return persistentList;
}

QModelIndexList ModelSpy::getUnchangedIndexes(const QModelIndex &parent, QList<QItemSelectionRange> ignoredRanges)
{
  QModelIndexList list;
  int rowCount = m_model->rowCount(parent);
  for (int row = 0; row < rowCount; )
  {
    int column = 0;
    QModelIndex idx = m_model->index( row, column, parent);
    Q_ASSERT(idx.isValid());
    bool found = false;
    foreach(QItemSelectionRange range, ignoredRanges)
    {
      if (range.topLeft().parent() == parent &&  range.topLeft().row() == idx.row())
      {
        row = range.bottomRight().row() + 1;
        found = true;
        break;
      }
    }
    if (!found)
    {
      for (column = 0; column < m_model->columnCount(); ++column )
        list << m_model->index( row, column, parent);
      list << getUnchangedIndexes(idx, ignoredRanges);
      ++row;
    }
  }
  return list;
}

void ModelSpy::preTestPersistIndexes(const PersistentChangeList &changeList)
{
  m_changeList = changeList;
  if (!m_lazyPersist)
    doPersist();
}

void ModelSpy::doPersist()
{
  Q_ASSERT(m_unchangedIndexes.isEmpty());
  Q_ASSERT(m_unchangedPersistentIndexes.isEmpty());

  const int columnCount = m_model->columnCount();
  QMutableListIterator<PersistentIndexChange> it(m_changeList);

  // The indexes are defined by the test are described with IndexFinder before anything in the model exists.
  // Now that the indexes should exist, resolve them in the change objects.
  QList<QItemSelectionRange> changedRanges;

  while (it.hasNext())
  {
    PersistentIndexChange change = it.next();
    change.parentFinder.setModel(m_model);
    QModelIndex parent = change.parentFinder.getIndex();

    Q_ASSERT(change.startRow >= 0);
    Q_ASSERT(change.startRow <= change.endRow);

    if (change.endRow >= m_model->rowCount(parent))
      kDebug() << m_model << parent << change.startRow << change.endRow << parent.data() << m_model->rowCount(parent);

    Q_ASSERT(change.endRow < m_model->rowCount(parent));

    QModelIndex topLeft = m_model->index( change.startRow, 0, parent );
    QModelIndex bottomRight = m_model->index( change.endRow, columnCount - 1, parent );

    // We store the changed ranges so that we know which ranges should not be changed
    changedRanges << QItemSelectionRange(topLeft, bottomRight);

    // Store the inital state of the indexes in the model which we expect to change.
    for (int row = change.startRow; row <= change.endRow; ++row )
    {
      for (int column = 0; column < columnCount; ++column)
      {
        QModelIndex idx = m_model->index(row, column, parent);
        Q_ASSERT(idx.isValid());
        change.indexes << idx;
        change.persistentIndexes << QPersistentModelIndex(idx);
      }

      // Also store the descendants of changed indexes so that we can verify the effect on them
      QModelIndex idx = m_model->index(row, 0, parent);
      QModelIndexList descs = getDescendantIndexes(idx);
      change.descendantIndexes << descs;
      change.persistentDescendantIndexes << toPersistent(descs);
    }
    it.setValue(change);
  }
  // Any indexes outside of the ranges we expect to be changed are stored
  // so that we can later verify that they remain unchanged.
  m_unchangedIndexes = getUnchangedIndexes(QModelIndex(), changedRanges);
  m_unchangedPersistentIndexes = toPersistent(m_unchangedIndexes);
}

