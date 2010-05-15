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

#include "kselectionproxymodel.h"

#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtGui/QItemSelectionRange>

#include "kmodelindexproxymapper.h"

#include "kdebug.h"

/**
  Return true if @p idx is a descendant of one of the indexes in @p list.
  Note that this returns false if @p list contains @p idx.
*/
template<typename ModelIndex>
bool isDescendantOf(const QList<ModelIndex> &list, const QModelIndex &idx)
{
    QModelIndex parent = idx.parent();
    while (parent.isValid()) {
        if (list.contains(parent))
            return true;
        parent = parent.parent();
    }
    return false;
}

static bool isDescendantOf(const QModelIndex &ancestor, const QModelIndex &descendant)
{
    if (!descendant.isValid())
        return false;

    if (ancestor == descendant)
        return false;

    QModelIndex parent = descendant.parent();
    while (parent.isValid()) {
        if (parent == ancestor)
            return true;

        parent = parent.parent();
    }
    return false;
}

static QModelIndex childOfParent(const QModelIndex& ancestor, const QModelIndex& descendant)
{
    QModelIndex child = descendant;
    while (child.isValid() && child.parent() != ancestor) {
        child = child.parent();
    }
    return child;
}

static int _getRootListRow(const QList<QModelIndexList> &rootAncestors, const QModelIndex &index)
{
    QModelIndex commonParent = index;
    QModelIndex youngestAncestor;

    int firstCommonParent = -1;
    int bestParentRow = -1;
    while (commonParent.isValid()) {
        youngestAncestor = commonParent;
        commonParent = commonParent.parent();

        for (int i = 0; i < rootAncestors.size(); ++i) {
            const QModelIndexList ancestorList = rootAncestors.at(i);

            const int parentRow = ancestorList.indexOf(commonParent);

            if (parentRow < 0)
                continue;

            if (parentRow > bestParentRow) {
                firstCommonParent = i;
                bestParentRow = parentRow;
            }
        }

        if (firstCommonParent >= 0)
            break;
    }

    // If @p list is non-empty, the invalid QModelIndex() will at least be found in ancestorList.
    Q_ASSERT(firstCommonParent >= 0);

    const QModelIndexList firstAnsList = rootAncestors.at(firstCommonParent);

    const QModelIndex eldestSibling = firstAnsList.value(bestParentRow + 1);

    if (eldestSibling.isValid()) {
        // firstCommonParent is a sibling of one of the ancestors of @p index.
        // It is the first index to share a common parent with one of the ancestors of @p index.
        if (eldestSibling.row() >= youngestAncestor.row())
            return firstCommonParent;
    }

    int siblingOffset = 1;

    // The same commonParent might be common to several root indexes.
    // If this is the last in the list, it's the only match. We instruct the model
    // to insert the new index after it ( + siblingOffset).
    if (rootAncestors.size() <= firstCommonParent + siblingOffset) {
        return firstCommonParent + siblingOffset;
    }

    // A
    // - B
    //   - C
    //   - D
    //   - E
    // F
    //
    // F is selected, then C then D. When inserting D into the model, the commonParent is B (the parent of C).
    // The next existing sibling of B is F (in the proxy model). bestParentRow will then refer to an index on
    // the level of a child of F (which doesn't exist - Boom!). If it doesn't exist, then we've already found
    // the place to insert D
    QModelIndexList ansList = rootAncestors.at(firstCommonParent + siblingOffset);
    if (ansList.size() <= bestParentRow) {
        return firstCommonParent + siblingOffset;
    }

    QModelIndex nextParent = ansList.at(bestParentRow);
    while (nextParent == commonParent) {
        if (ansList.size() < bestParentRow + 1)
            // If the list is longer, it means that at the end of it is a descendant of the new index.
            // We insert the ancestors items first in that case.
            break;

        const QModelIndex nextSibling = ansList.value(bestParentRow + 1);

        if (!nextSibling.isValid()) {
            continue;
        }

        if (youngestAncestor.row() <= nextSibling.row()) {
            break;
        }

        siblingOffset++;

        if (rootAncestors.size() <= firstCommonParent + siblingOffset)
            break;

        ansList = rootAncestors.at(firstCommonParent + siblingOffset);

        // In the scenario above, E is selected after D, causing this loop to be entered,
        // and requiring a similar result if the next sibling in the proxy model does not have children.
        if (ansList.size() <= bestParentRow) {
            break;
        }

        nextParent = ansList.at(bestParentRow);
    }

    return firstCommonParent + siblingOffset;
}

/**
  Determines the correct location to insert @p index into @p list.
*/
template<typename ModelIndex>
static int getRootListRow(const QList<ModelIndex> &list, const QModelIndex &index)
{
    if (!index.isValid())
        return -1;

    if (list.isEmpty())
        return 0;

    // What's going on?
    // Consider a tree like
    //
    // A
    // - B
    // - - C
    // - - - D
    // - E
    // - F
    // - - G
    // - - - H
    // - I
    // - - J
    // - K
    //
    // If D, E and J are already selected, and H is newly selected, we need to put H between E and J in the proxy model.
    // To figure that out, we create a list for each already selected index of its ancestors. Then,
    // we climb the ancestors of H until we reach an index with siblings which have a descendant
    // selected (F above has siblings B, E and I which have descendants which are already selected).
    // Those child indexes are traversed to find the right sibling to put F beside.
    //
    // i.e., new items are inserted in the expected location.

    QList<QModelIndexList> rootAncestors;
    foreach(const QModelIndex &root, list) {
        QModelIndexList ancestors;
        ancestors << root;
        QModelIndex parent = root.parent();
        while (parent.isValid()) {
            ancestors.prepend(parent);
            parent = parent.parent();
        }
        ancestors.prepend(QModelIndex());
        rootAncestors << ancestors;
    }
    return _getRootListRow(rootAncestors, index);
}

class KSelectionProxyModelPrivate
{
public:
    KSelectionProxyModelPrivate(KSelectionProxyModel *model)
            : q_ptr(model),
            m_startWithChildTrees(false),
            m_omitChildren(false),
            m_omitDescendants(false),
            m_includeAllSelected(false),
            m_rowsRemoved(false),
            m_resetting(false),
            m_ignoreNextLayoutAboutToBeChanged(false),
            m_ignoreNextLayoutChanged(false) {

    }

    Q_DECLARE_PUBLIC(KSelectionProxyModel)
    KSelectionProxyModel *q_ptr;

    QItemSelectionModel *m_selectionModel;
    QList<QPersistentModelIndex> m_rootIndexList;

    KModelIndexProxyMapper *m_indexMapper;

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
    void emitContinuousRanges(const QModelIndex &sourceFirst, const QModelIndex &sourceLast,
                              const QModelIndex &proxyFirst, const QModelIndex &proxyLast);
    void sourceDataChanged(const QModelIndex &topLeft , const QModelIndex &bottomRight);

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void sourceModelDestroyed();

    void resetInternalData();

    /**
      Returns the range in the proxy model corresponding to the range in the source model
      covered by @sourceParent, @p start and @p end.
    */
    QPair<int, int> getRootRange(const QModelIndex &sourceParent, int start, int end) const;

    /**
      When items are inserted or removed in the m_startWithChildTrees configuration,
      this method helps find the startRow for use emitting the signals from the proxy.
    */
    int getProxyInitialRow(const QModelIndex &parent) const;

    /**
      Returns a selection in which no descendants of selected indexes are also themselves selected.
      For example,
      @code
        A
        - B
        C
        D
      @endcode
      If A, B and D are selected in @p selection, the returned selection contains only A and D.
    */
    QItemSelection getRootRanges(const QItemSelection &selection) const;

    /**
      Returns the indexes in @p selection which are not already part of the proxy model.
    */
    QModelIndexList getNewIndexes(const QItemSelection &selection) const;

    /**
      If m_startWithChildTrees is true, this method returns the row in the proxy model to insert newIndex
      items.

      This is a special case because the items above rootListRow in the list are not in the model, but
      their children are. Those children must be counted.

      If m_startWithChildTrees is false, this method returns @p rootListRow.
    */
    int getTargetRow(int rootListRow);

    /**
      Regroups @p list into contiguous groups with the same parent.
    */
    QList<QPair<QModelIndex, QModelIndexList> > regroup(const QModelIndexList &list) const;

    /**
      Inserts the indexes in @p list into the proxy model.
    */
    void insertionSort(const QModelIndexList &list);

    /**
      Returns true if @p sourceIndex or one of its ascendants is already part of the proxy model.
    */
    bool isInModel(const QModelIndex &sourceIndex) const;

    /**
      Converts an index in the selection model to an index in the source model.
    */
    QModelIndex selectionIndexToSourceIndex(const QModelIndex &index) const;

    /**
      Returns the total number of children (but not descendants) of all of the indexes in @p list.
    */
    template<typename ModelIndex>
    int childrenCount(const QList<ModelIndex> &list) const
    {
        Q_Q(const KSelectionProxyModel);
        int count = 0;

        foreach(const ModelIndex &idx, list) {
            count += q->sourceModel()->rowCount(idx);
        }

        return count;
    }


    // Used to map children of indexes in the source model to indexes in the proxy model.
    // TODO: Find out if this breaks when indexes are modified because of higher siblings move/insert/remove
    mutable QHash< void *, QPersistentModelIndex> m_map;

    bool m_startWithChildTrees;
    bool m_omitChildren;
    bool m_omitDescendants;
    bool m_includeAllSelected;
    bool m_rowsRemoved;
    bool m_resetting;
    bool m_ignoreNextLayoutAboutToBeChanged;
    bool m_ignoreNextLayoutChanged;

    struct PendingMove {
        PendingMove() : doMove(false), doInsert(false), doRemove(false) {}
        bool doMove;
        bool doInsert;
        bool doRemove;
    };

    QStack<PendingMove> m_pendingMoves;

    KSelectionProxyModel::FilterBehavior m_filterBehavior;

    QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
    QModelIndexList m_proxyIndexes;

};

void KSelectionProxyModelPrivate::emitContinuousRanges(const QModelIndex &sourceFirst, const QModelIndex &sourceLast,
                                                       const QModelIndex &proxyFirst, const QModelIndex &proxyLast)
{
    Q_Q(KSelectionProxyModel);

    const int proxyRangeSize = proxyLast.row() - proxyFirst.row();
    const int sourceRangeSize = sourceLast.row() - sourceFirst.row();

    if (proxyRangeSize == sourceRangeSize)
    {
        emit q->dataChanged(proxyFirst, proxyLast);
        return;
    }


    // TODO: Loop to skip descendant ranges.
//     int lastRow;
//
//     const QModelIndex sourceHalfWay = sourceFirst.sibling(sourceFirst.row() + (sourceRangeSize / 2));
//     const QModelIndex proxyHalfWay = proxyFirst.sibling(proxyFirst.row() + (proxyRangeSize / 2));
//     const QModelIndex mappedSourceHalfway = q->mapToSource(proxyHalfWay);
//
//     const int halfProxyRange = mappedSourceHalfway.row() - proxyFirst.row();
//     const int halfSourceRange = sourceHalfWay.row() - sourceFirst.row();
//
//     if (proxyRangeSize == sourceRangeSize)
//     {
//         emit q->dataChanged(proxyFirst, proxyLast.sibling(proxyFirst.row() + proxyRangeSize, proxyLast.column()));
//         return;
//     }

    emit q->dataChanged(proxyFirst, proxyLast);
}

void KSelectionProxyModelPrivate::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_Q(KSelectionProxyModel);

    const QModelIndex proxyTopLeft = q->mapFromSource(topLeft);
    const QModelIndex proxyBottomRight = q->mapFromSource(bottomRight);

    const QModelIndex proxyRangeParent = proxyTopLeft.parent();

    if (!m_omitChildren && m_omitDescendants && m_startWithChildTrees && m_includeAllSelected)
    {
        // ChildrenOfExactSelection
        if (proxyTopLeft.isValid())
            emitContinuousRanges(topLeft, bottomRight, proxyTopLeft, proxyBottomRight);
        return;
    }

    if ((m_omitChildren && !m_startWithChildTrees && m_includeAllSelected)
          || (!proxyRangeParent.isValid() && !m_startWithChildTrees))
    {
        // Exact selection and SubTreeRoots and SubTrees in top level
        // Emit continuous ranges.
        QList<int> changedRows;
        for (int row = topLeft.row(); row < bottomRight.row(); ++row )
        {
          const QModelIndex index = q->sourceModel()->index(row, topLeft.column(), topLeft.parent());
          const int idx = m_rootIndexList.indexOf(index);
          if (idx != -1)
          {
            changedRows.append(idx);
          }
        }
        if (changedRows.isEmpty())
          return;
        int first = changedRows.first();
        int previous = first;
        QList<int>::const_iterator it = changedRows.constBegin();
        const QList<int>::const_iterator end = changedRows.constEnd();
        for( ; it != end; ++it )
        {
            if (*it == previous + 1) {
                ++previous;
            } else {
                const QModelIndex _top = q->index(first, topLeft.column() );
                const QModelIndex _bottom = q->index(previous, bottomRight.column() );
                emit q->dataChanged(_top, _bottom);
                previous = first = *it;
            }
        }
        if (first != previous)
        {
            const QModelIndex _top = q->index(first, topLeft.column() );
            const QModelIndex _bottom = q->index(previous, bottomRight.column() );
            emit q->dataChanged(_top, _bottom);
        }
        return;
    }
    if (proxyRangeParent.isValid()) {
        if (m_omitChildren && !m_startWithChildTrees && !m_includeAllSelected)
            // SubTreeRoots
            return;
        if (!proxyTopLeft.isValid())
            return;
        // SubTrees and SubTreesWithoutRoots
        emit q->dataChanged(proxyTopLeft, proxyBottomRight);
        return;
    }

    if (m_startWithChildTrees && !m_omitChildren && !m_includeAllSelected && !m_omitDescendants)
    {
        // SubTreesWithoutRoots
        if (proxyTopLeft.isValid())
            emit q->dataChanged(proxyTopLeft, proxyBottomRight);
        return;
    }
}

void KSelectionProxyModelPrivate::sourceLayoutAboutToBeChanged()
{
    Q_Q(KSelectionProxyModel);

    if (m_ignoreNextLayoutAboutToBeChanged) {
        m_ignoreNextLayoutAboutToBeChanged = false;
        return;
    }

    emit q->layoutAboutToBeChanged();

    if (!m_selectionModel->hasSelection())
        return;

    QPersistentModelIndex srcPersistentIndex;
    foreach(const QPersistentModelIndex &proxyPersistentIndex, q->persistentIndexList()) {
        m_proxyIndexes << proxyPersistentIndex;
        Q_ASSERT(proxyPersistentIndex.isValid());
        srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
        Q_ASSERT(srcPersistentIndex.isValid());
        m_layoutChangePersistentIndexes << srcPersistentIndex;
    }
}

void KSelectionProxyModelPrivate::sourceLayoutChanged()
{
    Q_Q(KSelectionProxyModel);

    if (m_ignoreNextLayoutChanged) {
        m_ignoreNextLayoutChanged = false;
        return;
    }

    if (!m_selectionModel->hasSelection()) {
        emit q->layoutChanged();
        return;
    }

    for (int i = 0; i < m_proxyIndexes.size(); ++i)
        q->changePersistentIndex(m_proxyIndexes.at(i), q->mapFromSource(m_layoutChangePersistentIndexes.at(i)));


    m_layoutChangePersistentIndexes.clear();
    m_proxyIndexes.clear();

    emit q->layoutChanged();
}

void KSelectionProxyModelPrivate::resetInternalData()
{
    m_rootIndexList.clear();
    m_layoutChangePersistentIndexes.clear();
    m_pendingMoves.clear();
}

void KSelectionProxyModelPrivate::sourceModelDestroyed()
{
    Q_Q(KSelectionProxyModel);
    // There is very little we can do here.
    resetInternalData();
    m_resetting = false;
    q->endResetModel();
}

void KSelectionProxyModelPrivate::sourceModelAboutToBeReset()
{
    Q_Q(KSelectionProxyModel);

    // Deselecting an index in the selectionModel will cause it to
    // be removed from m_rootIndexList, so we don't need to clear
    // the list here manually.
    // We also don't need to notify that an index is about to be removed.
    m_selectionModel->clearSelection();

    q->beginResetModel();
    m_resetting = true;
}

void KSelectionProxyModelPrivate::sourceModelReset()
{
    Q_Q(KSelectionProxyModel);

    // No need to try to refill this. When the model is reset it doesn't have a meaningful selection anymore,
    // but when it gets one we'll be notified anyway.
    resetInternalData();
    m_selectionModel->clearSelection();
    m_resetting = false;
    q->endResetModel();
}

QPair<int, int> KSelectionProxyModelPrivate::getRootRange(const QModelIndex &sourceParent, int start, int end) const
{
    int listStart = -1;
    int listEnd = -1;

    int tracker = 0;
    foreach(const QModelIndex &idx, m_rootIndexList) {
        if (listStart == -1) {
            if (idx.row() > start && idx.parent() == sourceParent) {
                listStart = tracker;
            }
        }
        if (idx.row() < end && m_rootIndexList.value(tracker - 1).parent() == sourceParent) {
            listEnd = tracker - 1;
            break;
        }
        tracker++;

    }
    return qMakePair(listStart, listEnd);
}

int KSelectionProxyModelPrivate::getProxyInitialRow(const QModelIndex &parent) const
{
    Q_Q(const KSelectionProxyModel);
    int parentPosition = m_rootIndexList.indexOf(parent);

    QModelIndex parentAbove;
    while (parentPosition > 0) {
        parentPosition--;

        parentAbove = m_rootIndexList.at(parentPosition);
        Q_ASSERT(parentAbove.isValid());

        int rows = q->sourceModel()->rowCount(parentAbove);
        if (rows > 0) {
            QModelIndex sourceIndexAbove = q->sourceModel()->index(rows - 1, 0, parentAbove);
            Q_ASSERT(sourceIndexAbove.isValid());
            QModelIndex proxyChildAbove = q->mapFromSource(sourceIndexAbove);
            Q_ASSERT(proxyChildAbove.isValid());
            return proxyChildAbove.row() + 1;
        }
    }
    return 0;
}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    if (!m_selectionModel->hasSelection() || (m_omitChildren && !m_startWithChildTrees))
        return;

    if (isInModel(parent) && !(m_startWithChildTrees && m_omitDescendants)) {
        // The easy case.
        q->beginInsertRows(q->mapFromSource(parent), start, end);
        return;
    }

    if (m_startWithChildTrees && m_rootIndexList.contains(parent)) {
        // Another fairly easy case.
        // The children of the indexes in m_rootIndexList are in the model, but their parents
        // are not, so we can't simply mapFromSource(parent) and get the row() to figure out
        // where the new rows are supposed to go.
        // Instead, we look at the 'higher' siblings of parent for their child items.
        // The lowest child item becomes the closest sibling of the new items.

        // sourceModel:
        // A
        // -> B
        // C
        // -> D
        // E
        // F

        // A, C, E and F are selected, then proxyModel is:
        //
        // B
        // D

        // If F gets a new child item, it must be inserted into the proxy model below D.
        // To find out what the proxyRow is, we find the proxyRow of D which is already in the model,
        // and +1 it.

        int proxyStartRow = getProxyInitialRow(parent);

        proxyStartRow += start;

        q->beginInsertRows(QModelIndex(), proxyStartRow, proxyStartRow + (end - start));
        return;
    }

}

void KSelectionProxyModelPrivate::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);
    Q_UNUSED(end);
    Q_UNUSED(start);

    if (!m_selectionModel->hasSelection() || (m_omitChildren && !m_startWithChildTrees))
        return;

    if (isInModel(parent) && !(m_startWithChildTrees && m_omitDescendants)) {
        q->endInsertRows();
        return;
    }

    if (m_startWithChildTrees && m_rootIndexList.contains(parent)) {
        q->endInsertRows();
        return;
    }
}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);

    if (!m_selectionModel->hasSelection() || (m_omitChildren && !m_startWithChildTrees))
        return;

    const QModelIndex firstAffectedIndex = q->sourceModel()->index(start, 0, parent);

    const QModelIndex proxyParent = q->mapFromSource(parent);
    const QModelIndex proxyFirstAffectedIndex = q->mapFromSource(firstAffectedIndex);
    if (proxyParent.isValid()) {
        // Get the easy case out of the way first.
        if (proxyFirstAffectedIndex.isValid()) {
            m_rowsRemoved = true;
            q->beginRemoveRows(proxyParent, start, end);
            return;
        }
    }

    QModelIndexList removedSourceIndexes;
    removedSourceIndexes << firstAffectedIndex;
    for (int row = start + 1; row <= end; row++) {
        QModelIndex sourceChildIndex = q->sourceModel()->index(row, 0, parent);
        removedSourceIndexes << sourceChildIndex;
    }

    int proxyStart = -1;
    int proxyEnd = -1;

    // If we are going to remove a root index and all of its descendants, we need to start
    // at the top-most affected one.
    for (int row = 0; row < m_rootIndexList.size(); ++row) {
        const QModelIndex rootIndex = m_rootIndexList.at(row);
        if (isDescendantOf(removedSourceIndexes, rootIndex) || removedSourceIndexes.contains(rootIndex)) {
            proxyStart = row;
            break;
        }
    }

    // proxyEnd points to the last affected index in m_rootIndexList affected by the removal.
    for (int row = m_rootIndexList.size() - 1; row >= 0; --row) {
        const QModelIndex rootIndex = m_rootIndexList.at(row);

        if (isDescendantOf(removedSourceIndexes, rootIndex) || removedSourceIndexes.contains(rootIndex)) {
            proxyEnd = row;
            break;
        }
    }

    if (proxyStart == -1) {
        if (!m_startWithChildTrees) {
            return;
        }
        // No index in m_rootIndexList was a descendant of a deleted row.
        // Probably children of an index in m_rootIndex are being removed.
        if (!m_rootIndexList.contains(parent)) {
            return;
        }

        int parentPosition = -1;
        if (!parent.isValid()) {
            proxyStart = start;
        } else {
            parentPosition = m_rootIndexList.indexOf(parent);
            proxyStart = getTargetRow(parentPosition) + start;
        }

        proxyEnd = proxyStart + (end - start);

        // Descendants of children that are being removed must also be removed if they are also selected.
        for (int position = parentPosition + 1; position < m_rootIndexList.size(); ++position) {
            const QModelIndex nextParent = m_rootIndexList.at(position);
            if (isDescendantOf(parent, nextParent)) {
                if (end > childOfParent(parent, nextParent).row()) {
                    // All descendants of rows to be removed are accounted for.
                    break;
                }

                proxyEnd += q->sourceModel()->rowCount(nextParent);
            } else {
                break;
            }
        }
    } else {
        if (m_startWithChildTrees) {
            proxyStart = getTargetRow(proxyStart);

            const QModelIndex lastAffectedSourceParent = m_rootIndexList.at(proxyEnd);
            const QModelIndex lastAffectedSourceChild = q->sourceModel()->index(q->sourceModel()->rowCount(lastAffectedSourceParent) - 1, 0, lastAffectedSourceParent);

            const QModelIndex lastAffectedProxyChild = q->mapFromSource(lastAffectedSourceChild);

            proxyEnd = lastAffectedProxyChild.row();
        }
    }

    if (proxyStart == -1 || proxyEnd == -1)
        return;

    m_rowsRemoved = true;
    q->beginRemoveRows(QModelIndex(), proxyStart, proxyEnd);
}

void KSelectionProxyModelPrivate::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KSelectionProxyModel);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    if (!m_selectionModel->hasSelection() || (m_omitChildren && !m_startWithChildTrees))
        return;

    QMutableListIterator<QPersistentModelIndex> it(m_rootIndexList);
    while (it.hasNext()) {
        const QPersistentModelIndex idx = it.next();
        if (!idx.isValid()) {
            emit q->rootIndexAboutToBeRemoved(idx);
            it.remove();
        }
    }

    if (m_rowsRemoved)
        q->endRemoveRows();
    m_rowsRemoved = false;

}

void KSelectionProxyModelPrivate::sourceRowsAboutToBeMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    Q_Q(KSelectionProxyModel);

    // layout{,AboutToBe}Changed signals are emitted by QAIM for backward compatibility reasons when rows are moved.
    // Processing them is expensive so we do our best to avoid them.
    m_ignoreNextLayoutAboutToBeChanged = true;

    if (!m_selectionModel->hasSelection())
        return;

    PendingMove pendingMove;
    if (m_includeAllSelected && !m_startWithChildTrees) {
        int destMoveRow = -1;

        QList<QPersistentModelIndex>::const_iterator it = m_rootIndexList.constBegin();
        const QList<QPersistentModelIndex>::const_iterator end = m_rootIndexList.constEnd();

        int startMoveRow = -1;
        int endMoveRow = -1;
        int count = 0;
        int row;
        for ( ; it != end; ++it, ++count) {
            row = it->row();
            if ((row >= srcStart && row <= srcEnd) && q->sourceModel()->parent(*it) == srcParent) {
                if (startMoveRow == -1) {
                    startMoveRow = count;
                    endMoveRow = count;

                    if (destMoveRow == -1) {
                        if (destParent.isValid())
                            destMoveRow = getRootListRow(m_rootIndexList, destParent);
                        else
                            destMoveRow = it->row();
                    }
                } else
                    ++endMoveRow;
            } else {
                if (row < destRow && q->sourceModel()->parent(*it) == destParent) {
                    if (destMoveRow == -1)
                        if (destParent.isValid())
                            destMoveRow = getRootListRow(m_rootIndexList, destParent);
                        else
                            destMoveRow = it->row();
                    else
                        ++destMoveRow;
                }
            }
        }
        if (startMoveRow != -1 && (destMoveRow < startMoveRow || destMoveRow > endMoveRow)) {
            Q_ASSERT(destMoveRow != -1);
            q->beginMoveRows(QModelIndex(), startMoveRow, endMoveRow, QModelIndex(), destMoveRow);
            pendingMove.doMove = true;
        }
        m_pendingMoves.push(pendingMove);
        return;
    }

    const bool srcInModel = (!m_startWithChildTrees && isInModel(srcParent)) || (m_startWithChildTrees && m_rootIndexList.contains(srcParent));
    const bool destInModel = (!m_startWithChildTrees && isInModel(destParent)) || (m_startWithChildTrees && m_rootIndexList.contains(destParent));

    if (srcInModel) {
        if (destInModel) {
            // The easy case.
            const bool allowMove = q->beginMoveRows(q->mapFromSource(srcParent), srcStart, srcEnd, q->mapFromSource(destParent), destRow);
            Q_UNUSED(allowMove);   // prevent warning in release builds.
            Q_ASSERT(allowMove);

            pendingMove.doMove = true;
            m_pendingMoves.push(pendingMove);
        } else {
            // source is in the proxy, but dest isn't.
            // Emit a remove
            q->beginRemoveRows(srcParent, srcStart, srcEnd);
            pendingMove.doRemove = true;
            m_pendingMoves.push(pendingMove);
        }
    } else {
        if (destInModel) {
            // dest is in proxy, but source is not.
            // Emit an insert
            q->beginInsertRows(destParent, destRow, destRow + (srcEnd - srcStart));
            pendingMove.doInsert = true;
            m_pendingMoves.push(pendingMove);
        }
    }
    m_pendingMoves.push(pendingMove);
}

void KSelectionProxyModelPrivate::sourceRowsMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    Q_Q(KSelectionProxyModel);
    Q_UNUSED(srcParent)
    Q_UNUSED(srcStart);
    Q_UNUSED(srcEnd);
    Q_UNUSED(destParent)
    Q_UNUSED(destRow);

    m_ignoreNextLayoutChanged = true;

    if (!m_selectionModel->hasSelection())
        return;

    const PendingMove pendingMove = m_pendingMoves.pop();

    if (pendingMove.doMove)
        q->endMoveRows();

    if (pendingMove.doRemove)
        q->endRemoveRows();

    if (pendingMove.doInsert)
        q->endInsertRows();
}

QModelIndexList KSelectionProxyModelPrivate::getNewIndexes(const QItemSelection &selection) const
{
    QModelIndexList indexes;

    foreach(const QItemSelectionRange &range, selection) {
        QModelIndex newIndex = range.topLeft();

        if (newIndex.column() != 0)
            continue;

        for (int row = newIndex.row(); row <= range.bottom(); ++row) {
            static const int column = 0;
            newIndex = newIndex.sibling(row, column);

            const QModelIndex sourceNewIndex = selectionIndexToSourceIndex(newIndex);

            Q_ASSERT(sourceNewIndex.isValid());

            const int startRow = m_rootIndexList.indexOf(sourceNewIndex);
            if (startRow > 0) {
                continue;
            }

            indexes << sourceNewIndex;
        }
    }
    return indexes;
}

void KSelectionProxyModelPrivate::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_Q(KSelectionProxyModel);

    if (!q->sourceModel() || (selected.isEmpty() && deselected.isEmpty()))
        return;

    // Any deselected indexes in the m_rootIndexList are removed. Then, any
    // indexes in the selected range which are not a descendant of one of the already selected indexes
    // are inserted into the model.
    //
    // All ranges from the selection model need to be split into individual rows. Ranges which are contiguous in
    // the selection model may not be contiguous in the source model if there's a sort filter proxy model in the chain.
    //
    // Some descendants of deselected indexes may still be selected. The ranges in m_selectionModel->selection()
    // are examined. If any of the ranges are descendants of one of the
    // indexes in deselected, they are added to the ranges to be inserted into the model.
    //
    // The new indexes are inserted in sorted order.

    QModelIndexList removeIndexes;

    static const int column = 0;
    foreach(const QItemSelectionRange &range, deselected) {
        QModelIndex removeIndex = range.topLeft();

        if (removeIndex.column() != 0)
            continue;

        for (int row = removeIndex.row(); row <= range.bottom(); ++row) {
            removeIndex = removeIndex.sibling(row, column);

            removeIndexes << removeIndex;

            const QModelIndex sourceRemoveIndex = selectionIndexToSourceIndex(removeIndex);

            const int startRow = m_rootIndexList.indexOf(sourceRemoveIndex);

            if (startRow < 0)
                continue;

            if (m_startWithChildTrees) {
                int _start = 0;
                for (int i = 0 ; i < startRow; ++i) {
                    _start += q->sourceModel()->rowCount(m_rootIndexList.at(i));
                }
                const int rowCount = q->sourceModel()->rowCount(m_rootIndexList.at(startRow));
                if (rowCount <= 0) {
                    // It doesn't have any children in the model, but we need to remove it from storage anyway.
                    emit q->rootIndexAboutToBeRemoved(m_rootIndexList.at(startRow));
                    m_rootIndexList.removeAt(startRow);
                    continue;
                }

                q->beginRemoveRows(QModelIndex(), _start, _start + rowCount - 1);
                emit q->rootIndexAboutToBeRemoved(m_rootIndexList.at(startRow));
                m_rootIndexList.removeAt(startRow);
                q->endRemoveRows();
            } else {
                q->beginRemoveRows(QModelIndex(), startRow, startRow);
                emit q->rootIndexAboutToBeRemoved(m_rootIndexList.at(startRow));
                m_rootIndexList.removeAt(startRow);
                q->endRemoveRows();
            }
        }
    }

    QItemSelection newRanges;
    if (!m_includeAllSelected) {
        newRanges = getRootRanges(selected);
        QMutableListIterator<QItemSelectionRange> i(newRanges);
        while (i.hasNext()) {
            const QItemSelectionRange range = i.next();
            const QModelIndex topLeft = range.topLeft();
            if (isDescendantOf(m_rootIndexList, topLeft)) {
                i.remove();
            }
        }
    } else {
        newRanges = selected;
    }

    QModelIndexList newIndexes;

    newIndexes << getNewIndexes(newRanges);

    QItemSelection additionalRanges;
    if (!m_includeAllSelected) {
        foreach(const QItemSelectionRange &range, m_selectionModel->selection()) {
            const QModelIndex topLeft = range.topLeft();
            if (isDescendantOf(removeIndexes, topLeft)) {
                if (!isDescendantOf(m_rootIndexList, topLeft) && !isDescendantOf(newIndexes, topLeft))
                    additionalRanges << range;
            }

            const int row = m_rootIndexList.indexOf(topLeft);
            if (row >= 0) {
                if (isDescendantOf(newIndexes, topLeft)) {
                    q->beginRemoveRows(QModelIndex(), row, row);
                    emit q->rootIndexAboutToBeRemoved(m_rootIndexList.at(row));
                    m_rootIndexList.removeAt(row);
                    q->endRemoveRows();
                }
            }
        }
        // A
        // - B
        // - - C
        // - - D
        // - E
        // If A, B, C and E are selected, and A is deselected, B and E need to be inserted into the model, but not C.
        // Currently B, C and E are in additionalRanges. Ranges which are descendant of other ranges in the list need
        // to be removed.

        additionalRanges = getRootRanges(additionalRanges);

    }

    // TODO: Use QSet<QModelIndex> instead to simplify this stuff.
    foreach(const QModelIndex &idx, getNewIndexes(additionalRanges)) {
        Q_ASSERT(idx.isValid());
        if (!newIndexes.contains(idx))
            newIndexes << idx;
    }
    if (newIndexes.size() > 0)
        insertionSort(newIndexes);
}

QList<QPair<QModelIndex, QModelIndexList> > KSelectionProxyModelPrivate::regroup(const QModelIndexList &list) const
{
    QList<QPair<QModelIndex, QModelIndexList> > groups;

    // TODO: implement me.
    Q_UNUSED(list);
//   foreach (const QModelIndex idx, list)
//   {
//
//   }

    return groups;
}

int KSelectionProxyModelPrivate::getTargetRow(int rootListRow)
{
    Q_Q(KSelectionProxyModel);
    if (!m_startWithChildTrees)
        return rootListRow;

    --rootListRow;
    while (rootListRow >= 0) {
        const QModelIndex idx = m_rootIndexList.at(rootListRow);
        Q_ASSERT(idx.isValid());
        const int rowCount = q->sourceModel()->rowCount(idx);
        if (rowCount > 0) {
            static const int column = 0;
            const QModelIndex srcIdx = q->sourceModel()->index(rowCount - 1, column, idx);
            const QModelIndex proxyLastChild = q->mapFromSource(srcIdx);
            return proxyLastChild.row() + 1;
        }
        --rootListRow;
    }
    return 0;
}

void KSelectionProxyModelPrivate::insertionSort(const QModelIndexList &list)
{
    Q_Q(KSelectionProxyModel);

    // TODO: regroup indexes in list into contiguous ranges with the same parent.
//   QList<QPair<QModelIndex, QModelIndexList> > regroup(list);
    // where pair.first is a parent, and pair.second is a list of contiguous indexes of that parent.
    // That will allow emitting new rows in ranges rather than one at a time.

    foreach(const QModelIndex &newIndex, list) {
        if (m_startWithChildTrees) {
            const int rootListRow = getRootListRow(m_rootIndexList, newIndex);
            Q_ASSERT(q->sourceModel() == newIndex.model());
            const int rowCount = q->sourceModel()->rowCount(newIndex);
            const int startRow = getTargetRow(rootListRow);
            if (rowCount > 0) {
                if (!m_resetting)
                    q->beginInsertRows(QModelIndex(), startRow, startRow + rowCount - 1);
                Q_ASSERT(newIndex.isValid());
                m_rootIndexList.insert(rootListRow, newIndex);
                emit q->rootIndexAdded(newIndex);
                if (!m_resetting) {
                    q->endInsertRows();
                }
            } else {
                // Even if the newindex doesn't have any children to put into the model yet,
                // We still need to make sure it's future children are inserted into the model.
                m_rootIndexList.insert(rootListRow, newIndex);
                emit q->rootIndexAdded(newIndex);
            }
        } else {
            const int row = getRootListRow(m_rootIndexList, newIndex);

            if (!m_resetting)
                q->beginInsertRows(QModelIndex(), row, row);
            Q_ASSERT(newIndex.isValid());
            m_rootIndexList.insert(row, newIndex);
            emit q->rootIndexAdded(newIndex);
            if (!m_resetting) {
                q->endInsertRows();
            }
        }
    }
}

QItemSelection KSelectionProxyModelPrivate::getRootRanges(const QItemSelection &selection) const
{
    QModelIndexList parents;
    QItemSelection rootSelection;
    QListIterator<QItemSelectionRange> i(selection);
    while (i.hasNext()) {
        parents << i.next().topLeft();
    }

    i.toFront();

    while (i.hasNext()) {
        QItemSelectionRange range = i.next();
        if (isDescendantOf(parents, range.topLeft()))
            continue;
        rootSelection << range;
    }
    return rootSelection;
}

QModelIndex KSelectionProxyModelPrivate::selectionIndexToSourceIndex(const QModelIndex &index) const
{
    return m_indexMapper->mapRightToLeft(index);
}

bool KSelectionProxyModelPrivate::isInModel(const QModelIndex &sourceIndex) const
{
    if (m_rootIndexList.contains(sourceIndex)) {
        if (m_startWithChildTrees)
            return false;
        return true;
    }

    QModelIndex seekIndex = sourceIndex;
    while (seekIndex.isValid()) {
        if (m_rootIndexList.contains(seekIndex)) {
            return true;
        }

        seekIndex = seekIndex.parent();
    }
    return false;
}

KSelectionProxyModel::KSelectionProxyModel(QItemSelectionModel *selectionModel, QObject *parent)
        : QAbstractProxyModel(parent), d_ptr(new KSelectionProxyModelPrivate(this))
{
    Q_D(KSelectionProxyModel);

    d->m_selectionModel = selectionModel;
}

KSelectionProxyModel::~KSelectionProxyModel()
{
    delete d_ptr;
}

void KSelectionProxyModel::setFilterBehavior(FilterBehavior behavior)
{
    Q_D(KSelectionProxyModel);

    beginResetModel();

    d->m_filterBehavior = behavior;

    switch (behavior) {
    case SubTrees: {
        d->m_omitChildren = false;
        d->m_omitDescendants = false;
        d->m_startWithChildTrees = false;
        d->m_includeAllSelected = false;
        break;
    }
    case SubTreeRoots: {
        d->m_omitChildren = true;
        d->m_startWithChildTrees = false;
        d->m_includeAllSelected = false;
        break;
    }
    case SubTreesWithoutRoots: {
        d->m_omitChildren = false;
        d->m_omitDescendants = false;
        d->m_startWithChildTrees = true;
        d->m_includeAllSelected = false;
        break;
    }
    case ExactSelection: {
        d->m_omitChildren = true;
        d->m_startWithChildTrees = false;
        d->m_includeAllSelected = true;
        break;
    }
    case ChildrenOfExactSelection: {
        d->m_omitChildren = false;
        d->m_omitDescendants = true;
        d->m_startWithChildTrees = true;
        d->m_includeAllSelected = true;
        break;
    }
    }

    endResetModel();
}

KSelectionProxyModel::FilterBehavior KSelectionProxyModel::filterBehavior() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_filterBehavior;
}

void KSelectionProxyModel::setSourceModel(QAbstractItemModel *_sourceModel)
{
    Q_D(KSelectionProxyModel);
    if (_sourceModel == sourceModel())
        return;

    connect(d->m_selectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

    beginResetModel();
    d->m_resetting = true;

    if (_sourceModel) {
        disconnect(_sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(_sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(_sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(_sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(_sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(_sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(_sourceModel, SIGNAL(modelAboutToBeReset()),
                   this, SLOT(sourceModelAboutToBeReset()));
        disconnect(_sourceModel, SIGNAL(modelReset()),
                   this, SLOT(sourceModelReset()));
        disconnect(_sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(_sourceModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(sourceLayoutAboutToBeChanged()));
        disconnect(_sourceModel, SIGNAL(layoutChanged()),
                   this, SLOT(sourceLayoutChanged()));
        disconnect(_sourceModel, SIGNAL(destroyed()),
                   this, SLOT(sourceModelDestroyed()));
    }

    // Must be called before QAbstractProxyModel::setSourceModel because it emits some signals.
    d->resetInternalData();
    QAbstractProxyModel::setSourceModel(_sourceModel);
    if (_sourceModel) {
        d->m_indexMapper = new KModelIndexProxyMapper(_sourceModel, d->m_selectionModel->model(), this);
        if (d->m_selectionModel->hasSelection())
            d->selectionChanged(d->m_selectionModel->selection(), QItemSelection());

        connect(_sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(_sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(_sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(_sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        connect(_sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(_sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(_sourceModel, SIGNAL(modelAboutToBeReset()),
                SLOT(sourceModelAboutToBeReset()));
        connect(_sourceModel, SIGNAL(modelReset()),
                SLOT(sourceModelReset()));
        connect(_sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(_sourceModel, SIGNAL(layoutAboutToBeChanged()),
                SLOT(sourceLayoutAboutToBeChanged()));
        connect(_sourceModel, SIGNAL(layoutChanged()),
                SLOT(sourceLayoutChanged()));
        connect(_sourceModel, SIGNAL(destroyed()),
                SLOT(sourceModelDestroyed()));
    }

    d->m_resetting = false;
    endResetModel();
}

QModelIndex KSelectionProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    Q_D(const KSelectionProxyModel);

    if (!proxyIndex.isValid() || !sourceModel())
        return QModelIndex();

    QModelIndex idx = d->m_map.value(proxyIndex.internalPointer());
    idx = idx.sibling(idx.row(), proxyIndex.column());
    return idx;

}

QModelIndex KSelectionProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel())
        return QModelIndex();

    const int row = d->m_rootIndexList.indexOf(sourceIndex);
    if (row != -1) {
        if (!d->m_startWithChildTrees) {
            d->m_map.insert(sourceIndex.internalPointer(), QPersistentModelIndex(sourceIndex));
            return createIndex(row, sourceIndex.column(), sourceIndex.internalPointer());
        }
        const QModelIndex sourceParent = sourceIndex.parent();
        int parentRow = d->m_rootIndexList.indexOf(sourceParent);
        if (parentRow == -1)
            return QModelIndex();

        int proxyRow = sourceIndex.row();
        while (parentRow > 0) {
            --parentRow;
            QModelIndex selectedIndexAbove = d->m_rootIndexList.at(parentRow);
            proxyRow += sourceModel()->rowCount(selectedIndexAbove);
        }

        d->m_map.insert(sourceIndex.internalPointer(), QPersistentModelIndex(sourceIndex));
        return createIndex(proxyRow, sourceIndex.column(), sourceIndex.internalPointer());
    } else if (d->isInModel(sourceIndex)) {
        int targetRow = sourceIndex.row();
        if ((d->m_rootIndexList.contains(sourceIndex.parent()))
                && (d->m_startWithChildTrees)) {
            // loop over m_rootIndexList before sourceIndex, counting children.
            targetRow = 0;
            foreach(const QModelIndex &idx, d->m_rootIndexList) {
                if (idx == sourceIndex.parent())
                    break;
                targetRow += sourceModel()->rowCount(idx);
            }
            targetRow += sourceIndex.row();
        } else if (d->m_startWithChildTrees)
            return QModelIndex();

        d->m_map.insert(sourceIndex.internalPointer(), QPersistentModelIndex(sourceIndex));
        return createIndex(targetRow, sourceIndex.column(), sourceIndex.internalPointer());
    }
    return QModelIndex();
}

int KSelectionProxyModel::rowCount(const QModelIndex &index) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel())
        return 0;

    if (!index.isValid()) {
        if (!d->m_startWithChildTrees) {
            return d->m_rootIndexList.size();
        } else {
            return d->childrenCount(d->m_rootIndexList);
        }
    }

    if (d->m_omitChildren || (d->m_startWithChildTrees && d->m_omitDescendants))
        return 0;

    const QModelIndex srcIndex = mapToSource(index);

    if (!d->isInModel(srcIndex))
        return 0;

    if (d->m_omitDescendants) {
        if (d->m_startWithChildTrees)
            return 0;

        if (d->m_rootIndexList.contains(srcIndex.parent()))
            return 0;
    }

    return sourceModel()->rowCount(srcIndex);
}

QModelIndex KSelectionProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const KSelectionProxyModel);

    if (!hasIndex(row, column, parent) || !sourceModel())
        return QModelIndex();

    if (!parent.isValid()) {
        if (!d->m_startWithChildTrees) {
            const QModelIndex idx = d->m_rootIndexList.at(row);
            d->m_map.insert(idx.internalPointer(), idx);
            return createIndex(row, column, idx.internalPointer());
        }
        int _row = row;
        foreach(const QModelIndex &idx, d->m_rootIndexList) {
            const int idxRowCount = sourceModel()->rowCount(idx);
            if (_row < idxRowCount) {
                const QModelIndex childIndex = sourceModel()->index(_row, column, idx);
                d->m_map.insert(childIndex.internalPointer(), QPersistentModelIndex(childIndex));
                return createIndex(row, childIndex.column(), childIndex.internalPointer());
            }
            _row -= idxRowCount;
        }

        return QModelIndex();
    } else {
        const QModelIndex sourceParent = mapToSource(parent);
        const QModelIndex sourceIndex = sourceModel()->index(row, column, sourceParent);
        return mapFromSource(sourceIndex);
    }
}

QModelIndex KSelectionProxyModel::parent(const QModelIndex &index) const
{
    Q_D(const KSelectionProxyModel);

    if (!sourceModel() || !index.isValid())
        return QModelIndex();

    const QModelIndex sourceIndex = mapToSource(index);
    if (d->m_rootIndexList.contains(sourceIndex.parent()) && (d->m_startWithChildTrees || d->m_omitChildren)) {
        return QModelIndex();
    }

    if (d->m_rootIndexList.contains(sourceIndex))
        return QModelIndex();

    return mapFromSource(sourceIndex.parent());
}

Qt::ItemFlags KSelectionProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !sourceModel())
        return QAbstractProxyModel::flags(index);

    const QModelIndex srcIndex = mapToSource(index);
    Q_ASSERT(srcIndex.isValid());
    return sourceModel()->flags(srcIndex);
}

QVariant KSelectionProxyModel::data(const QModelIndex & index, int role) const
{
    if (!sourceModel())
        return QVariant();

    if (index.isValid()) {
        const QModelIndex idx = mapToSource(index);
        return idx.data(role);
    }
    return sourceModel()->data(index, role);
}

QVariant KSelectionProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel())
        return QVariant();
    return sourceModel()->headerData(section, orientation, role);
}

QMimeData* KSelectionProxyModel::mimeData(const QModelIndexList & indexes) const
{
    if (!sourceModel())
        return QAbstractProxyModel::mimeData(indexes);
    QModelIndexList sourceIndexes;
    foreach(const QModelIndex& index, indexes)
        sourceIndexes << mapToSource(index);
    return sourceModel()->mimeData(sourceIndexes);
}

QStringList KSelectionProxyModel::mimeTypes() const
{
    if (!sourceModel())
        return QAbstractProxyModel::mimeTypes();
    return sourceModel()->mimeTypes();
}

Qt::DropActions KSelectionProxyModel::supportedDropActions() const
{
    if (!sourceModel())
        return QAbstractProxyModel::supportedDropActions();
    return sourceModel()->supportedDropActions();
}

bool KSelectionProxyModel::hasChildren(const QModelIndex & parent) const
{
    Q_D(const KSelectionProxyModel);

    if (d->m_rootIndexList.isEmpty())
      return false;

    if (parent.isValid())
    {
        if (d->m_omitChildren || (d->m_startWithChildTrees && d->m_omitDescendants))
            return false;
        return sourceModel()->hasChildren(mapToSource(parent));
    }

    if (!d->m_startWithChildTrees)
      return true;

    foreach(const QPersistentModelIndex &idx, d->m_rootIndexList) {
      if (sourceModel()->hasChildren(idx))
        return true;
    }

    return false;
}

int KSelectionProxyModel::columnCount(const QModelIndex &index) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->columnCount(mapToSource(index));
}

QItemSelectionModel* KSelectionProxyModel::selectionModel() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_selectionModel;
}

bool KSelectionProxyModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;

    if ((row == -1) && (column == -1))
        return sourceModel()->dropMimeData(data, action, -1, -1, mapToSource(parent));

    int source_destination_row = -1;
    int source_destination_column = -1;
    QModelIndex source_parent;

    if (row == rowCount(parent)) {
        source_parent = mapToSource(parent);
        source_destination_row = sourceModel()->rowCount(source_parent);
    } else {
        const QModelIndex proxy_index = index(row, column, parent);
        const QModelIndex source_index = mapToSource(proxy_index);
        source_destination_row = source_index.row();
        source_destination_column = source_index.column();
        source_parent = source_index.parent();
    }
    return sourceModel()->dropMimeData(data, action, source_destination_row,
                                       source_destination_column, source_parent);
}

QList<QPersistentModelIndex> KSelectionProxyModel::sourceRootIndexes() const
{
    Q_D(const KSelectionProxyModel);
    return d->m_rootIndexList;
}

QModelIndexList KSelectionProxyModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
    if (role < Qt::UserRole)
        return QAbstractProxyModel::match(start, role, value, hits, flags);

    QModelIndexList list;
    QModelIndex proxyIndex;
    foreach(const QModelIndex &idx, sourceModel()->match(mapToSource(start), role, value, hits, flags)) {
        proxyIndex = mapFromSource(idx);
        if (proxyIndex.isValid())
            list << proxyIndex;

    }
    return list;
}

#include "moc_kselectionproxymodel.cpp"
