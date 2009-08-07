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

#include "kreparentingproxymodel.h"
#include <kdebug.h>
#include <QStack>

#include <functional>
#include <algorithm>


class KReparentingProxyModelPrivate
{
  KReparentingProxyModelPrivate(KReparentingProxyModel *proxyModel)
  : q_ptr(proxyModel), m_nextId(0)
  {

  }

  qint64 newId() const { return m_nextId++; }

  enum MapStrategy
  {
    MapDescendants,
    MapChildrenOnly
  };

  /**
    Creates mappings of indexes in the source model between @p start
    and @p end which should be represented in the proxy model as descendants
    of @p parent.
  */
  QHash<QModelIndex, QModelIndexList> recreateMappings(const QModelIndex &parent, int start, int end = -1, int strategy = MapChildrenOnly) const;

  /**
    Merges all indexes from @p mappings which are descendants of @p parent into the model.
    Returns the remaing mappings.

    Note that this changes the internal model structure and must only be called between begin/end insert/remove/move/reset calls.
  */
  QHash<QModelIndex, QModelIndexList> mergeDescendants(QHash<QModelIndex, QModelIndexList> mappings, const QModelIndex &parent, int start );

  /**
    Verifies that the indexes below @p parent between @p start and rowCount(parent)
    are in the correct positions in the proxy model. Repositions them if not.
  */
  void verifyStructure(const QModelIndex &parent, int start);

  /**
    Returns the index vertically below index in the source model.
    Returns an invalid index if there is no index below @p index.
  */
  QModelIndex getIndexBelow(const QModelIndex &index) const;

  /**
    Returns the last descendant of @p index or itself if it has no children
  */
  QModelIndex getLastDescendant(const QModelIndex &index) const;

  /**
    Returns the ancestors of @p descendant that are already in the proxy model.

    Note that @p descendant does not have to be in the proxy yet, and it is not part of the
    result list.
  */
  QVector<QModelIndex> getExistingAncestors(const QModelIndex &descendant) const;

  void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void sourceRowsInserted(const QModelIndex &parent, int start, int end);
  void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
  void sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
  void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
  void sourceModelAboutToBeReset();
  void sourceModelReset();
  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();
  void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  mutable QHash<qint64, QPersistentModelIndex> m_parents;
  mutable QHash<QPersistentModelIndex, QList<QPersistentModelIndex> > m_childIndexes;

  struct PendingInsertion
  {
    PendingInsertion()
      : start(-1), end(-1)
    { }

    PendingInsertion(const QModelIndex &_index, int _start, int _end)
      : index(_index), start(_start), end(_end)
    { }

    QPersistentModelIndex index;
    int start;
    int end;
  };

  typedef PendingInsertion PendingRemoval;

  void handleInsertion(const PendingInsertion &pendingInsertion);

  void handleRemoval(const PendingRemoval &pendingRemoval);

  mutable QHash<QModelIndex, PendingInsertion> m_pendingInsertions;
  mutable QHash<QModelIndex, PendingInsertion> m_pendingRemovals;

  mutable qint64 m_nextId;

  Q_DECLARE_PUBLIC( KReparentingProxyModel )
  KReparentingProxyModel *q_ptr;

};

class LessThan
{
  const KReparentingProxyModel *m_model;
public:
  LessThan(const KReparentingProxyModel *model) : m_model(model) {}
  bool operator()(const QModelIndex &ancestor, const QModelIndex &descendant)
  {
    return m_model->isDescendantOf(ancestor, descendant);
  }
};

QModelIndex KReparentingProxyModelPrivate::getIndexBelow(const QModelIndex &index) const
{
  Q_Q(const KReparentingProxyModel);

  if (q->sourceModel()->hasChildren(index))
    return q->sourceModel()->index(0, 0, index);

  return index.sibling(index.row() + 1, index.column());
}

QModelIndex KReparentingProxyModelPrivate::getLastDescendant(const QModelIndex &index) const
{
  Q_Q(const KReparentingProxyModel);

  QModelIndex proxyIndex = q->mapFromSource(index);

  while (q->hasChildren(proxyIndex))
  {
    proxyIndex = proxyIndex.child(q->rowCount(proxyIndex), proxyIndex.column());
    if (!proxyIndex.isValid())
      break;
  }
  return q->mapToSource(proxyIndex);
}

QVector<QModelIndex> KReparentingProxyModelPrivate::getExistingAncestors(const QModelIndex &descendant) const
{
  Q_Q(const KReparentingProxyModel);

  QVector<QModelIndex> vector;
  if (!descendant.isValid())
    return vector;

  QModelIndex parent = q->mapFromSource(descendant).parent();
  vector.append(q->mapToSource(parent));
  while (parent.isValid())
  {
    parent = parent.parent();
    vector.prepend(q->mapToSource(parent));
  }
  return vector;
}

QHash<QModelIndex, QModelIndexList> KReparentingProxyModelPrivate::recreateMappings(const QModelIndex &ancestor, int start, int end, int strategy) const
{
  Q_Q(const KReparentingProxyModel);
  const int column = 0;

  QHash<QModelIndex, QModelIndexList> mappings;
  // Handle listing the root QModelIndex().
  if (!ancestor.isValid() && !q->sourceModel()->hasChildren())
    // Empty model. Nothing to do.
    return mappings;

  // A
  // - B
  // - - C
  // - D
  // If start refers to D, existing ancestors will contain only A.
  // We need to go 'up' to C and get its ancestors in case D is to be made a child of B or C
  QModelIndex indexAbove;
  if (start > 0)
    indexAbove = getLastDescendant(q->sourceModel()->index(start - 1, column, ancestor));
  else
    indexAbove = ancestor;

  QVector<QModelIndex> ancestors = getExistingAncestors(indexAbove);

  ancestors.append(indexAbove);

  QModelIndex nextIndex;

  for (int row = start; (row <= end || end == -1); ++row)
  {
    // TODO: Change this to get the next index by asking the proxy model, not the source model.
    if (MapDescendants == strategy)
    {
      nextIndex = getIndexBelow(nextIndex);
    } else {
      nextIndex = q->sourceModel()->index(row, column, ancestor);
    }
    if (!nextIndex.isValid())
      break;

    QModelIndex deepestAncestor = ancestors.last();
    if (q->isDescendantOf(deepestAncestor, nextIndex))
    {
      mappings[deepestAncestor].append(nextIndex);
      ancestors.append(nextIndex);
      continue;
    }

    QVector<QModelIndex>::iterator ancestorIt = qLowerBound(ancestors.begin(), ancestors.end(), nextIndex, LessThan(q));

    QModelIndex parent = *(ancestorIt - 1);
    ancestorIt = ancestors.insert(ancestorIt, nextIndex);

    if (ancestorIt != ancestors.end())
    {
      ++ancestorIt;
      while (ancestorIt != ancestors.end())
      {
        ancestorIt = ancestors.erase(ancestorIt);
      }
    }
    mappings[parent].append(nextIndex);
  }

  return mappings;
}

void KReparentingProxyModelPrivate::verifyStructure(const QModelIndex &sourceParent, int sourceStart)
{
  Q_Q( KReparentingProxyModel );

  // If the start structure is:

  // C
  // D
  // E

  // and then A and B are inserted, we may need to move C D and E. Not all of the siblings will
  // necessarily be moved to the same destination parent.
  // Some example finished scenarios depending on the outcome of isDescendantOf:

  // A
  // B
  // C
  // D
  // E

  // A
  // B
  // - C
  // - D
  // - E

  // A
  // - B
  // - C
  // - D
  // - E

  // A
  // - B
  // - - C
  // - D
  // E

  // Local variable mappings now contains all the information about finished state
  // When we locate the first child to be moved, we process it and its siblings

  QHash<QModelIndex, QModelIndexList> mappings = recreateMappings(sourceParent, sourceStart, -1);

  if (mappings.isEmpty())
    return;

  QModelIndex sourceFirstIndex = q->sourceModel()->index(sourceStart, 0, sourceParent);

  QModelIndex destinationParent;
  QModelIndexList movedIndexes;

  QHashIterator<QModelIndex, QModelIndexList> it(mappings);
  while (it.hasNext())
  {
    it.next();
    if (it.value().at(0) == sourceFirstIndex)
    {
      destinationParent = it.key();
      movedIndexes = it.value();
      break;
    }
  }

  forever
  {

    if (destinationParent == sourceParent)
      // No indexes moved
      return;

    Q_ASSERT(destinationParent.isValid());
    Q_ASSERT(!movedIndexes.isEmpty());

    // It's only possible for things to move right, and even that's only an option
    // for children of parent, but not their descendants. ie, children of C D and E will not need to be reparented.
    // They are already in the correct positions.

    QList<QPersistentModelIndex> &existingSourceIndexes = m_childIndexes[sourceParent];
    QList<QPersistentModelIndex> existingDestinationIndexes = m_childIndexes[destinationParent];

    QModelIndex proxySourceParent = q->mapFromSource(sourceParent);
    QModelIndex proxyDestinationParent = q->mapFromSource(destinationParent);

    // That is, start position of indexes to be moved from the source parent.
    int proxySourceStart = m_childIndexes.value(sourceParent).indexOf(movedIndexes.at(0));
    int proxySourceEnd = proxySourceStart + movedIndexes.size() - 1;

    // The moved indexes are appended to the destinationParent. Nothing else is possible.
    // If they were to be inserted in the middle somewhere, they would already be there.

    int destinationRow = existingDestinationIndexes.size();

    bool allowMove = q->beginMoveRows(proxySourceParent, proxySourceStart, proxySourceEnd, proxyDestinationParent, destinationRow);
    Q_ASSERT(allowMove);

    for (int row = proxySourceEnd; row >= proxySourceStart; --row)
    {
      existingSourceIndexes.removeAt(row);
    }

    QHash<QModelIndex, QModelIndexList> mapping;
    mapping.insert(destinationParent, movedIndexes);
    mergeDescendants(mapping, destinationParent, existingDestinationIndexes.size());

    q->endMoveRows();

    if (!mappings.contains(q->mapToSource(proxyDestinationParent.parent())))
      break;

    destinationParent = q->mapToSource(proxyDestinationParent.parent());
    movedIndexes = mappings.value(destinationParent);
  }
}


KReparentingProxyModel::KReparentingProxyModel(QObject* parent)
  : QAbstractProxyModel(parent), d_ptr(new KReparentingProxyModelPrivate(this))
{

}

bool KReparentingProxyModel::isDescendantOf(const QModelIndex& ancestor, const QModelIndex& descendant) const
{
  return (!ancestor.isValid() && descendant.isValid());
}

QModelIndex KReparentingProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
  Q_D( const KReparentingProxyModel );
  if (!sourceIndex.isValid())
    return QModelIndex();

  QModelIndex sourceIndexFirstColumn = sourceIndex.sibling(sourceIndex.row(), 0);

  QHash<QPersistentModelIndex, QList<QPersistentModelIndex> >::const_iterator it;
  const QHash<QPersistentModelIndex, QList<QPersistentModelIndex> >::const_iterator begin = d->m_childIndexes.constBegin();
  const QHash<QPersistentModelIndex, QList<QPersistentModelIndex> >::const_iterator end = d->m_childIndexes.constEnd();

  for(it = begin; it != end; ++it)
  {
    QList<QPersistentModelIndex> list = it.value();
    if (list.contains(sourceIndexFirstColumn))
    {
      QModelIndex sourceParent = it.key();
      QModelIndex proxyParent = mapFromSource(sourceParent);
      int row = list.indexOf(sourceIndexFirstColumn);

      // There must have been a mapping made for it.
      Q_ASSERT(d->m_parents.values().contains(sourceParent));

      qint64 id = d->m_parents.key(sourceParent);

      // id refers to the parent.
      return createIndex(row, sourceIndex.column(), reinterpret_cast<void*>(id));
    }
  }
  return QModelIndex();
}

QModelIndex KReparentingProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
  Q_D( const KReparentingProxyModel );

  if (!proxyIndex.isValid())
    return QModelIndex();

  qint64 id = reinterpret_cast<qint64>(proxyIndex.internalPointer());

  Q_ASSERT(d->m_parents.contains(id));
  QModelIndex sourceParent = d->m_parents.value(id);

  QModelIndex sourceIndexFirstColumn = d->m_childIndexes.value(sourceParent).at(proxyIndex.row());

  return sourceIndexFirstColumn.sibling(sourceIndexFirstColumn.row(), proxyIndex.column());
}

int KReparentingProxyModel::columnCount(const QModelIndex& parent) const
{
  Q_D( const KReparentingProxyModel );

  if (!parent.isValid())
    return sourceModel()->columnCount();

  if ( parent.column() > 0 )
  {
    return 0;
  }
  QModelIndex sourceIndex = mapToSource(parent);

  return (d->m_childIndexes.value(sourceIndex).size() > 0)
        ? sourceModel()->columnCount() : 0;
}

QVariant KReparentingProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
  return QAbstractProxyModel::data(proxyIndex, role);
}

QModelIndex KReparentingProxyModel::index(int row, int column, const QModelIndex& parent) const
{
  Q_D( const KReparentingProxyModel );

  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QModelIndex sourceParent = mapToSource(parent);

  // There must have been a mapping made for it.
  Q_ASSERT(d->m_parents.values().contains(sourceParent));

  qint64 id = d->m_parents.key(sourceParent);

  return createIndex(row, column, reinterpret_cast<void*>(id));
}

QModelIndex KReparentingProxyModel::parent(const QModelIndex& child) const
{
  Q_D( const KReparentingProxyModel );

  if (!child.isValid())
    return QModelIndex();

  QModelIndex sourceIndex = mapToSource(child);

  QModelIndex firstColumnChild = sourceIndex;
  if (sourceIndex.column() > 0)
    firstColumnChild = sourceIndex.sibling(sourceIndex.row(), 0);

  QHashIterator<QPersistentModelIndex, QList<QPersistentModelIndex> > it(d->m_childIndexes);

  while (it.hasNext())
  {
    it.next();

    if (it.value().contains(firstColumnChild))
    {

      return mapFromSource(it.key());
    }
  }
  return QModelIndex();
}

int KReparentingProxyModel::rowCount(const QModelIndex& parent) const
{
  Q_D( const KReparentingProxyModel );

  if ( parent.column() > 0 )
    return 0;

  QModelIndex sourceIndex = mapToSource(parent);

  return d->m_childIndexes.value(sourceIndex).size();
}

bool KReparentingProxyModel::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0;
}

void KReparentingProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  Q_D( KReparentingProxyModel );

  beginResetModel();

  // TODO: disconnect signals.

  QAbstractProxyModel::setSourceModel(sourceModel);

  QHash<QModelIndex, QModelIndexList> mappings = d->recreateMappings(QModelIndex(), 0, sourceModel->rowCount() - 1, KReparentingProxyModelPrivate::MapDescendants);
  d->mergeDescendants(mappings, QModelIndex(), 0);

  connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
          SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
          SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
          SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
  connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
          SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));

  // TODO: Uncomment for Qt4.6
//   connect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//           SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
//   connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
//           SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
  connect(sourceModel, SIGNAL(modelAboutToBeReset()),
          SLOT(sourceModelAboutToBeReset()));
  connect(sourceModel, SIGNAL(modelReset()),
          SLOT(sourceModelReset()));
  connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex & )));
  connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
          SLOT(sourceLayoutAboutToBeChanged()));
  connect(sourceModel, SIGNAL(layoutChanged()),
          SLOT(sourceLayoutChanged()));

  endResetModel();
}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
  Q_Q(KReparentingProxyModel);

  // We can't figure out the structure until the indexes are in the model.
  // Store the signal until the new rows are actually there in sourceRowsInserted.
  PendingInsertion pendingInsertion(parent, start, end);
  m_pendingInsertions.insert(parent, pendingInsertion);
}

QHash<QModelIndex, QModelIndexList> KReparentingProxyModelPrivate::mergeDescendants(QHash<QModelIndex, QModelIndexList> mappings, const QModelIndex &parent, int start)
{
  QModelIndexList childIndexes = mappings.take(parent);
  if (!childIndexes.isEmpty())
  {
    if (!m_parents.values().contains(parent))
    {
      m_parents.insert(newId(), QPersistentModelIndex(parent));
    }
  }
  int row = start;
  foreach (const QModelIndex &idx, childIndexes)
  {
    m_childIndexes[parent].insert(row++, QPersistentModelIndex(idx));
    mappings = mergeDescendants(mappings, idx, 0);
  }
  return mappings;
}

void KReparentingProxyModelPrivate::handleInsertion(const PendingInsertion &pendingInsertion)
{
  Q_Q(KReparentingProxyModel);
  QModelIndex parent = pendingInsertion.index;
  int start = pendingInsertion.start;
  int end = pendingInsertion.end;

  QHash<QModelIndex, QModelIndexList> newItemMappings = recreateMappings(parent, start, end, KReparentingProxyModelPrivate::MapDescendants);

  const int column = 0;

  QModelIndexList newItemList = newItemMappings.value(parent);
  int proxyStart = 0;

  // A single insertion in the source model might be multiple insertions in the proxy model.
  forever
  {
    if (newItemList.size() < 0)
    {
      if (!newItemMappings.contains(parent.parent()))
        break;

      newItemList = newItemMappings.value(parent.parent());
      continue;
    }

    proxyStart = 0;

    QModelIndex proxyParent = q->mapFromSource(parent);
    if (start > 0)
    {
      QModelIndex lastDesc = q->mapFromSource(getLastDescendant(q->sourceModel()->index(start - 1, column, parent)));


      while (lastDesc.parent() != proxyParent)
      {
        lastDesc = lastDesc.parent();
      }
      proxyStart = lastDesc.row() + 1;
    }

    q->beginInsertRows(proxyParent, proxyStart, proxyStart + newItemList.size() - 1);

    newItemMappings = mergeDescendants(newItemMappings, parent, proxyStart);

    q->endInsertRows();

    if (!newItemMappings.contains(parent.parent()))
      break;

    newItemList = newItemMappings.value(parent.parent());
  }

  return;
}

void KReparentingProxyModelPrivate::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
  Q_Q(KReparentingProxyModel);
  if (m_pendingInsertions.contains(parent))
  {
    PendingInsertion pendingInsertion = m_pendingInsertions.value(parent);
    handleInsertion(pendingInsertion);

    if (q->sourceModel()->rowCount(parent) <= (end + 1))
      return;

    // The presence of new rows might affect the structure of indexes below.
    verifyStructure(parent, end + 1);
  }
}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  Q_Q(KReparentingProxyModel);

  QHash<QModelIndex, QModelIndexList> mappings;
  recreateMappings(parent, start, end);

  QHashIterator<QModelIndex, QModelIndexList> it(mappings);
  while (it.hasNext())
  {
    it.next();
    QModelIndexList removedList = it.value();
    PendingRemoval pendingRemoval;
    pendingRemoval.index = it.key();
    pendingRemoval.start = q->mapFromSource(removedList.at(0)).row();
    pendingRemoval.end = pendingRemoval.start + removedList.size() - 1;
    m_pendingRemovals.insert(parent, pendingRemoval);
  }
}

void KReparentingProxyModelPrivate::handleRemoval(const PendingRemoval &pendingRemoval)
{
  Q_Q(KReparentingProxyModel);
  q->beginRemoveRows(pendingRemoval.index, pendingRemoval.start, pendingRemoval.end);
  m_childIndexes.remove(pendingRemoval.index);
  // Remove stuff from m_parents.
  q->endRemoveRows();
}

void KReparentingProxyModelPrivate::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
  foreach(const PendingRemoval &pendingRemoval, m_pendingRemovals)
  {
    if ( parent == pendingRemoval.index
      && start <= pendingRemoval.start
      && end <= pendingRemoval.end)
    {
      handleRemoval(pendingRemoval);
      // The removal of rows might affect the structure of indexes below.
      verifyStructure(parent, end + 1);
    }
  }

}

void KReparentingProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex& destParent, int destRow)
{

}

void KReparentingProxyModelPrivate::sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex& destParent, int destRow)
{

}

void KReparentingProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
  Q_Q(KReparentingProxyModel);

  //TODO: Move persistent indexes.

  q->layoutAboutToBeChanged();
}

void KReparentingProxyModelPrivate::sourceLayoutChanged()
{
  Q_Q(KReparentingProxyModel);

  //TODO: Move persistent indexes.

  q->layoutChanged();
}

void KReparentingProxyModelPrivate::sourceModelAboutToBeReset()
{
  Q_Q(KReparentingProxyModel);
  q->beginResetModel();
}

void KReparentingProxyModelPrivate::sourceModelReset()
{
  Q_Q(KReparentingProxyModel);

  m_parents.clear();
  m_childIndexes.clear();
  m_nextId = 0;
  m_pendingInsertions.clear();
  m_pendingRemovals.clear();
  q->endResetModel();
}

void KReparentingProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{

}

#include "kreparentingproxymodel.moc"
