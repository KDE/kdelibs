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


#include "kdescendantsproxymodel.h"

#include "kdebug.h"

/**
  @internal

  Private implementation of DescendantsProxyModel.
*/
class KDescendantsProxyModelPrivate
{
  public:

  KDescendantsProxyModelPrivate(KDescendantsProxyModel *model)
    : q_ptr(model),
      m_displayAncestorData( false ),
      m_ancestorSeparator( QLatin1String( " / " ) )
    {
    }

  Q_DECLARE_PUBLIC( KDescendantsProxyModel )
  KDescendantsProxyModel *q_ptr;
  /**
  @internal

  Returns the @p row -th descendant of sourceParent.

  For example, if the source model looks like:
  @code
  -> Item 0-0 (this is row-depth)
  -> -> Item 0-1
  -> -> Item 1-1
  -> -> -> Item 0-2
  -> -> -> Item 1-2
  -> Item 1-0
  @endcode

  Then findSourceIndexForRow(3, index(Item 0-0)) will return an index for Item 1-2

  @returns The model index in the source model.

  */
  QModelIndex findSourceIndexForRow( int row, QModelIndex sourceParent) const;

  /**
  @internal

  Returns true if @p sourceIndex has a ancestor that is m_rootDescendIndex.
  Note that isDescended(m_rootDescendIndex) is false;
  @param sourceIndex The index in the source model.
  @returns Whether sourceIndex is an ancestor of rootIndex
  */
  bool isDescended(const QModelIndex &sourceIndex) const;

  /**
  @internal

  Returns the number of descendants below @p sourceIndex.

  For example, if the source model looks like:
  @code
  -> Item 0-0 (this is row-depth)
  -> -> Item 0-1
  -> -> Item 1-1
  -> -> -> Item 0-2
  -> -> -> Item 1-2
  -> Item 1-0
  @endcode

  The descendant count of the rootIndex would be 6,
  of Item 0-0 would be 4,
  of Item 1-1 would be 2,
  of Item 1-2 would be 0
  etc.

  */
  int descendantCount(const QModelIndex &sourceIndex, int ignoreTerminals=ObserveTerminals) const;

  enum TerminalIgnorance
  {
    ObserveTerminals,
    IgnoreTerminals
  };

  /**
  @internal

  Returns the row of @p sourceIndex below the rootDescendIndex.

  For example, if the source model looks like:
  @code
  -> Item 0-0 (this is row-depth)
  -> -> Item 0-1
  -> -> Item 1-1
  -> -> -> Item 0-2
  -> -> -> Item 1-2
  -> Item 1-0
  @endcode

  Then descendedRow(index(Item 0-0) would be 0,
  descendedRow(index(Item 0-1) would be 1,
  descendedRow(index(Item 0-2) would be 3,
  descendedRow(index(Item 1-0) would be 5
  etc.

  @returns The row in the proxy model where @p sourceIndex is represented.

  */
  int descendedRow(const QModelIndex &sourceIndex) const;

  QModelIndexList matchDescendants(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags, int until, const bool matchAll) const;

  enum Operation
  {
    InsertOperation,
    RemoveOperation
  };

  void insertOrRemoveRows(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, int type);

  void sourceRowsAboutToBeInserted(const QModelIndex &, int start, int end);
  void sourceRowsInserted(const QModelIndex &, int start, int end);
  void sourceRowsAboutToBeRemoved(const QModelIndex &, int start, int end);
  void sourceRowsRemoved(const QModelIndex &, int start, int end);
  void sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
  void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);
  void sourceModelAboutToBeReset();
  void sourceModelReset();
  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();
  void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  QPersistentModelIndex m_rootDescendIndex;
  // Hmm, if I make this QHash<QPersistentModelIndex, int> instead then moves are
  // automatically handled. Nope, they're not. deeper levels than base would
  // still need to be updated or calculated.

  mutable QHash<qint64, int> m_descendantsCount;

  bool m_displayAncestorData;
  QString m_ancestorSeparator;

  QList<QPersistentModelIndex> m_terminalIndexes;
  void descendNewIndexes();

};

KDescendantsProxyModel::KDescendantsProxyModel( QObject *parent )
      : QAbstractProxyModel( parent ),
        d_ptr( new KDescendantsProxyModelPrivate(this) )
{
  Q_D(KDescendantsProxyModel);

  d->m_rootDescendIndex = QModelIndex();
}

void KDescendantsProxyModel::setRootIndex(const QModelIndex &index)
{
  Q_D(KDescendantsProxyModel);

  if (index.isValid())
    Q_ASSERT(index.model() == sourceModel());

  d->m_rootDescendIndex = index;
  d->m_descendantsCount.clear();
  reset();
}

KDescendantsProxyModel::~KDescendantsProxyModel()
{
  Q_D(KDescendantsProxyModel);
  d->m_descendantsCount.clear();
}

QModelIndex KDescendantsProxyModelPrivate::findSourceIndexForRow( int row, QModelIndex idx ) const
{
    Q_Q( const KDescendantsProxyModel );
    int childCount = q->sourceModel()->rowCount(idx);
    for (int childRow = 0; childRow < childCount; childRow++)
    {
      QModelIndex childIndex = q->sourceModel()->index(childRow, 0, idx);
      if (row == 0)
      {
        return childIndex;
      }
      row--;
      if (q->sourceModel()->hasChildren(childIndex))
      {
        int childDesc = descendantCount(childIndex);
        if (childDesc > row)
        {
          return findSourceIndexForRow(row, childIndex);
        }
        row -= childDesc;
      }
    }

    // This should probably never happen if you use descendantCount before calling this method.
    return QModelIndex();
}

void KDescendantsProxyModel::setSourceModel(QAbstractItemModel * sourceModel)
{
  Q_D(KDescendantsProxyModel);
  QAbstractProxyModel::setSourceModel( sourceModel );
  connect( sourceModel, SIGNAL(modelReset()), SLOT( sourceModelReset() ) );
  connect( sourceModel, SIGNAL(modelAboutToBeReset()), SLOT(sourceModelAboutToBeReset() ) );
  connect( sourceModel, SIGNAL(layoutChanged()), SLOT(sourceLayoutChanged()) );
  connect( sourceModel, SIGNAL(layoutAboutToBeChanged()), SLOT(sourceLayoutAboutToBeChanged()) );
  connect( sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex & ) ) );
  connect( sourceModel, SIGNAL(rowsInserted(const QModelIndex, int, int)),
          SLOT(sourceRowsInserted(const QModelIndex, int, int)) );
  connect( sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex, int, int)),
          SLOT(sourceRowsAboutToBeInserted(const QModelIndex, int, int)) );
  connect( sourceModel, SIGNAL(rowsRemoved(const QModelIndex, int, int)),
          SLOT(sourceRowsRemoved(const QModelIndex, int, int)) );
  connect( sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex, int, int)),
          SLOT(sourceRowsAboutToBeRemoved(const QModelIndex, int, int)) );

          
  // Uncomment for Qt4.6          
//   connect( sourceModel, SIGNAL(rowsMoved(const QModelIndex, int, int, const QModelIndex, int)),
//           SLOT(sourceRowsMoved(const QModelIndex, int, int, const QModelIndex, int)) );
//   connect( sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex, int, int, const QModelIndex, int)),
//           SLOT(sourceRowsAboutToBeMoved(const QModelIndex, int, int, const QModelIndex, int) ) );

  d->m_descendantsCount.clear();
  reset();
}

bool KDescendantsProxyModelPrivate::isDescended(const QModelIndex &sourceIndex) const
{
  Q_Q(const KDescendantsProxyModel);

  if (sourceIndex == m_rootDescendIndex)
  {
    return false;
  }

  QModelIndex parentIndex = q->sourceModel()->parent(sourceIndex);

  if (parentIndex == m_rootDescendIndex)
  {
    return true;
  }
  bool found = false;

  forever
  {
    parentIndex = parentIndex.parent();
    if (parentIndex == m_rootDescendIndex)
    {
      found = true;
      break;
    }
    if (!parentIndex.isValid())
      break;
  }
  return found;
}

int KDescendantsProxyModelPrivate::descendedRow(const QModelIndex &sourceIndex) const
{
  Q_Q(const KDescendantsProxyModel);
  QModelIndex parentIndex = sourceIndex.parent();
  int row = sourceIndex.row();

  for (int childRow = 0; childRow < sourceIndex.row(); childRow++ )
  {
    QModelIndex childIndex = q->sourceModel()->index( childRow, sourceIndex.column(), parentIndex );
    if (q->sourceModel()->hasChildren(childIndex))
      row += descendantCount(childIndex);
  }

  if (parentIndex == m_rootDescendIndex)
  {
    // Return 0 instead of -1 for an invalid index.
    if (row < 0)
    {
      return 0;
    }
    return row;
  }
  else if(!parentIndex.isValid())
  {
    // Should never happen.
    // Someone must have called this with sourceIndex outside of m_rootDescendIndex
    return 0;
  }
  else {
  int dr = descendedRow(parentIndex);
    return row + dr + 1;
  }
}

QModelIndex KDescendantsProxyModel::mapFromSource(const QModelIndex & sourceIndex) const
{
  Q_D(const KDescendantsProxyModel);

  if (sourceIndex == d->m_rootDescendIndex)
  {
    return QModelIndex();
  }

  if ( d->isDescended( sourceIndex ) )
  {
    int row = d->descendedRow( sourceIndex );
    if (row < 0)
      return QModelIndex();
    return createIndex( row, sourceIndex.column() );
  } else {
    return QModelIndex();
  }
}

void KDescendantsProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &sourceParentIndex, int start, int end)
{
  insertOrRemoveRows(sourceParentIndex, start, end, InsertOperation);
}

void KDescendantsProxyModelPrivate::insertOrRemoveRows(const QModelIndex &sourceParentIndex, int start, int end, int operationType)
{

  Q_Q(KDescendantsProxyModel);

  int c = descendedRow(sourceParentIndex);

// Can't simply get the descendantCount of sourceModel()->index(start, 0, sourceParent),
// because start might not exist as a child of sourceParent yet.
// Maybe I should special case (!sourceParent.hasChildren) instead.

  // Only the first column can have child items. It is only those we need to count.
  const int column = 0;
  for (int childRow = 0; childRow < start; childRow++)
  {
    QModelIndex childIndex = q->sourceModel()->index( childRow, column, sourceParentIndex );
//     kDebug() << childIndex << descendantCount(childIndex);
    if (q->sourceModel()->hasChildren(childIndex))
      c += descendantCount(childIndex);
  }

//   @code
//   -> Item 0-0 (this is row-depth)
//   -> -> Item 0-1
//   -> -> Item 1-1
//   -> -> -> Item 0-2
//   -> -> -> Item 1-2
//   -> Item 1-0
//   @endcode
//
// If the sourceModel reports that a row is inserted between Item 0-2 Item 1-2,
// this methods receives a sourceParent of Item 1-1, and a start of 1.
// It has a descendedRow of 2. The proxied start is the number of rows above parent,
// and the start below parent. The parent itself must also be accounted for if it
// is part of the model.

// Check if it's descended instead?

  int proxy_start = c + start;
  int proxy_end = c + end;

  if (isDescended(sourceParentIndex))
  {
    proxy_start++;
    proxy_end++;
  }

  if (operationType == InsertOperation)
    q->beginInsertRows(m_rootDescendIndex, proxy_start, proxy_end);
  else if (operationType == RemoveOperation)
  {
    // need to notify that we're also removing the descendants.
    for (int childRow = start; childRow <= end; childRow++)
    {
      QModelIndex childIndex = q->sourceModel()->index(childRow,column,sourceParentIndex);
      if (q->sourceModel()->hasChildren(childIndex))
        proxy_end += descendantCount(childIndex);
    }

    q->beginRemoveRows(m_rootDescendIndex, proxy_start, proxy_end);
  }
}

void KDescendantsProxyModelPrivate::sourceRowsInserted(const QModelIndex &sourceParentIndex, int start, int end)
{
  Q_Q(KDescendantsProxyModel);

  m_descendantsCount.clear();

  // Don't count the descendants of newly inserted indexes until after endInsertRows.
  // The reason for this is that at the time we emitted beginInsertRows, we couldn't count the descendants
  // of new items (they weren't known to the sourceModel), and now we can.
  const int column = 0;
  for (int childRow = start; childRow <= end; childRow++)
  {
    QModelIndex childIndex = q->sourceModel()->index( childRow, column, sourceParentIndex );
    m_terminalIndexes << QPersistentModelIndex(childIndex);
  }

  q->endInsertRows();
  descendNewIndexes();
}

void KDescendantsProxyModelPrivate::descendNewIndexes()
{
  Q_Q(KDescendantsProxyModel);

  QMutableListIterator<QPersistentModelIndex> i(m_terminalIndexes);
  while (i.hasNext())
  {
    QModelIndex idx = i.next();

    int descCount = descendantCount(idx, IgnoreTerminals);

    if (descCount <= 0)
    {
      i.remove();
      continue;
    }

    // if descCount > 0, this is a new index which has new child indexes.

    int proxyStart = descendedRow(idx);
    // Need to ignore terminals so we know how many rows will be inserted by removing them.
    int proxyEnd = proxyStart + descendantCount(idx, IgnoreTerminals);

    if (isDescended(idx))
    {
      proxyStart++;
      proxyEnd++;
    }
    q->beginInsertRows(QModelIndex(), proxyStart, proxyEnd - 1 );
    i.remove();
    m_descendantsCount.clear();
    q->endInsertRows();
  }
}


void KDescendantsProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow)
{
  Q_Q(KDescendantsProxyModel);
  int c = descendedRow(parent);
  int d = descendedRow(destParent);
  // TODO: Uncomment for Qt 4.6
//   q->beginMoveRows(QModelIndex(), c+1+start, c+1+end, QModelIndex(), d+1+destRow);
}

void KDescendantsProxyModelPrivate::sourceRowsMoved(const QModelIndex &sourceParentIndex, int start, int end, const QModelIndex &destParentIndex, int destRow)
{
  Q_Q(KDescendantsProxyModel);
  Q_UNUSED(sourceParentIndex);
  Q_UNUSED(start);
  Q_UNUSED(end);
  Q_UNUSED(destParentIndex);
  Q_UNUSED(destRow);

  m_descendantsCount.clear();

  // TODO: Uncomment for Qt 4.6
//   q->endMoveRows();
}


void KDescendantsProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
  insertOrRemoveRows(parent, start, end, RemoveOperation);
}

void KDescendantsProxyModelPrivate::sourceRowsRemoved(const QModelIndex &sourceParentIndex, int start, int end)
{
  Q_Q(KDescendantsProxyModel);
  Q_UNUSED(sourceParentIndex);
  Q_UNUSED(start);
  Q_UNUSED(end);

  m_descendantsCount.clear();
  q->endRemoveRows();
}

void KDescendantsProxyModelPrivate::sourceModelAboutToBeReset()
{
  Q_Q(KDescendantsProxyModel);

  // TODO: Uncomment for Qt 4.6
//   q->beginResetModel();
}

void KDescendantsProxyModelPrivate::sourceModelReset()
{
  Q_Q(KDescendantsProxyModel);

  m_descendantsCount.clear();
  // TODO: Uncomment for Qt 4.6
//   q->endResetModel();
  q->reset();
}

void KDescendantsProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
  Q_Q(KDescendantsProxyModel);

  q->layoutAboutToBeChanged();
}

void KDescendantsProxyModelPrivate::sourceLayoutChanged()
{
  Q_Q(KDescendantsProxyModel);

  m_descendantsCount.clear();
  q->layoutChanged();
}

QModelIndex KDescendantsProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
  Q_D(const KDescendantsProxyModel);

  if (!proxyIndex.isValid())
    return d->m_rootDescendIndex;

  if (proxyIndex.column() >= sourceModel()->columnCount())
    return QModelIndex();

  QModelIndex idx = d->findSourceIndexForRow( proxyIndex.row(), d->m_rootDescendIndex );

  if (proxyIndex.column() > 0)
  {
    return sourceModel()->index(idx.row(), proxyIndex.column(), idx.parent());
  }
  return idx;
}

QVariant KDescendantsProxyModel::data(const QModelIndex & index, int role) const
{
  Q_D(const KDescendantsProxyModel );

  if (!index.isValid())
    return QVariant();

  QModelIndex sourceIndex = mapToSource( index );

  if ((d->m_displayAncestorData) && ( role == Qt::DisplayRole ) )
  {
    if (!sourceIndex.isValid())
    {
      return QVariant();
    }
    QString displayData = sourceIndex.data().toString();
    sourceIndex = sourceIndex.parent();
    while (sourceIndex.isValid())
    {
      displayData.prepend(d->m_ancestorSeparator);
      displayData.prepend(sourceIndex.data().toString());
      sourceIndex = sourceIndex.parent();
    }
    return displayData;
  } else {
    return sourceIndex.data(role);
  }

}

bool KDescendantsProxyModel::hasChildren ( const QModelIndex & parent ) const
{
  return rowCount(parent) > 0;
}

int KDescendantsProxyModel::rowCount(const QModelIndex & proxyIndex) const
{
  Q_D(const KDescendantsProxyModel );

  if (proxyIndex.column() > 0)
    return 0;

  QModelIndex sourceIndex = mapToSource(proxyIndex);

  if (sourceIndex == d->m_rootDescendIndex)
  {
    int c = d->descendantCount(sourceIndex);
    return c;
  }
  return 0;
}

void KDescendantsProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  Q_Q( KDescendantsProxyModel );
  int topRow = topLeft.row();
  int bottomRow = bottomRight.row();

  for(int i = topRow; i <= bottomRow; ++i)
  {
    QModelIndex sourceTopLeft = q->sourceModel()->index(i, topLeft.column(), topLeft.parent());
    QModelIndex proxyTopLeft = q->mapFromSource(sourceTopLeft);
    // TODO. If an index does not have any descendants, then we can emit in blocks of rows.
    // As it is we emit once for each row.
    QModelIndex sourceBottomRight = q->sourceModel()->index(i, bottomRight.column(), bottomRight.parent());
    QModelIndex proxyBottomRight = q->mapFromSource(sourceBottomRight);
    emit q->dataChanged(proxyTopLeft, proxyBottomRight);
  }
}

int KDescendantsProxyModelPrivate::descendantCount(const QModelIndex &sourceIndex, int ignoreTerminals) const
{
  if (sourceIndex.column() > 0)
    return 0;

  if (ObserveTerminals == ignoreTerminals)
  {
    if (m_terminalIndexes.contains(sourceIndex))
    {
      return 0;
    }
  }

  Q_Q( const KDescendantsProxyModel );
  if (m_descendantsCount.contains(sourceIndex.internalId()))
  {
    return m_descendantsCount.value(sourceIndex.internalId());
  }

  int sourceIndexRowCount = q->sourceModel()->rowCount(sourceIndex);
  if (sourceIndexRowCount == 0)
    return 0;
  int c = 0;
  c += sourceIndexRowCount;

  int childRow = 0;
  QModelIndex childIndex = q->sourceModel()->index(childRow, 0, sourceIndex);
  while (childIndex.isValid())
  {
    c += descendantCount(childIndex);
    childRow++;
    childIndex = q->sourceModel()->index(childRow, 0, sourceIndex);
  }

  m_descendantsCount.insert( sourceIndex.internalId(), c );

  return c;
}

QModelIndex KDescendantsProxyModel::index(int r, int c, const QModelIndex& parent) const
{
  Q_D(const KDescendantsProxyModel );

  if ( (r < 0) || (c < 0) || (c >= sourceModel()->columnCount() ) )
    return QModelIndex();

  if ( r >= d->descendantCount(parent) )
    return QModelIndex();

  // TODO: Use is decended instead?
  if (parent.isValid())
    return QModelIndex();

  return createIndex(r, c);
}

QModelIndex KDescendantsProxyModel::parent(const QModelIndex& proxyIndex) const
{
  Q_UNUSED(proxyIndex);

  return QModelIndex();
}

int KDescendantsProxyModel::columnCount(const QModelIndex &index) const
{
  return sourceModel()->columnCount();
}

void KDescendantsProxyModel::setDisplayAncestorData( bool display )
{
  Q_D(KDescendantsProxyModel);
  d->m_displayAncestorData = display;
}

bool KDescendantsProxyModel::displayAncestorData() const
{
  Q_D(const KDescendantsProxyModel );
  return d->m_displayAncestorData;
}

void KDescendantsProxyModel::setAncestorSeparator( const QString &separator )
{
  Q_D(KDescendantsProxyModel);
  d->m_ancestorSeparator = separator;
}

QString KDescendantsProxyModel::ancestorSeparator() const
{
  Q_D(const KDescendantsProxyModel );
  return d->m_ancestorSeparator;
}

Qt::ItemFlags KDescendantsProxyModel::flags( const QModelIndex &index ) const
{
  // if index is invalid, it might be mapped to a valid source index with more flags.
  // Can't allow that...
  if (!index.isValid())
    return 0;
  return QAbstractProxyModel::flags(index);
}

QModelIndexList KDescendantsProxyModelPrivate::matchDescendants(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags, int until, const bool matchAll) const
{
  Q_Q(const KDescendantsProxyModel);
  QModelIndexList matches;

  if (!start.isValid())
    return matches;

  const int column = start.column();
  const int firstRow = 0;
  QModelIndex idx = start;

  while (idx.row() <= until)
  {
    Q_ASSERT(idx.isValid());

    if (q->sourceModel()->hasChildren(idx))
    {
      QModelIndex firstChild = idx.child(firstRow, column);
      matches << q->match(q->mapFromSource(firstChild), role, value, hits, flags);
      if (!matchAll && (matches.size() >= hits))
      {
        return matches.mid(0, hits);
      }
    }
    int row = idx.row();
    if (row == until)
    {
      break;
    }
    idx = idx.sibling(row + 1, column);
  }

  return matches;
}

QModelIndexList KDescendantsProxyModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
  Q_D(const KDescendantsProxyModel );

  // We only really need to do all this for the AmazingCompletionRole, but there's no clean way to
  // determine what that is.

  QModelIndexList sourceList;
  QModelIndexList proxyList;

  QModelIndex beginIndex = start;
  QModelIndex sourceStart = mapToSource(start);
  QModelIndex parent = sourceModel()->parent(sourceStart);

  int parentRowCount = sourceModel()->rowCount(sourceModel()->parent(sourceStart));

  const bool matchAll = (hits == -1);
  const int firstHit = 1;
  const int column = start.column();
  const int proxyRowCount = rowCount();

  Q_ASSERT(sourceStart.column() == start.column());
  sourceList = sourceModel()->match(sourceStart, role, value, firstHit, flags);

  int lastRow;
  if (sourceList.isEmpty())
  {
    lastRow = parentRowCount - 1;
    proxyList = d->matchDescendants(mapToSource(start), role, value, hits, flags, lastRow, matchAll);

    if (matchAll)
      return proxyList;

    return proxyList.mid(0, hits);

  } else {
    forever
    {
      QModelIndex firstIndexHit;
      if (sourceList.isEmpty())
      {
        lastRow = parentRowCount - 1;
      } else {
        firstIndexHit = sourceList.first();

        Q_ASSERT(firstIndexHit.column() == start.column());

        lastRow = firstIndexHit.row() - 1;
      }

      proxyList << d->matchDescendants(sourceStart, role, value, hits, flags, lastRow, matchAll);

      if (sourceList.isEmpty())
        break;

      QModelIndex proxyFirst = mapFromSource(firstIndexHit);
      proxyList << proxyFirst;

      if (!matchAll && ( proxyList.size() >= hits))
      {
        return proxyList.mid(0, hits);
      }

      if (proxyFirst.row() == proxyRowCount - 1)
        break;

      sourceStart = mapToSource(index(proxyFirst.row() + 1, proxyFirst.column()));
      Q_ASSERT(sourceStart.isValid());

      sourceList = sourceModel()->match(sourceStart, role, value, firstHit, flags);
    }
  }

  QModelIndex nextStart = start.sibling(start.row() + parentRowCount + 1, column);
  if (nextStart.isValid())
    proxyList << match(nextStart, role, value, hits, flags);

  if (matchAll)
    return proxyList;

  return proxyList.mid(0, hits);
}

#include "moc_kdescendantsproxymodel.cpp"
