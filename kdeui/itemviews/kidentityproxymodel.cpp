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

#include "kidentityproxymodel.h"

#include <QtGui/QItemSelection>

class KIdentityProxyModelPrivate
{
  KIdentityProxyModelPrivate(KIdentityProxyModel *qq)
    : q_ptr(qq),
      m_ignoreNextLayoutAboutToBeChanged(false),
      m_ignoreNextLayoutChanged(false)
  {

  }

  Q_DECLARE_PUBLIC(KIdentityProxyModel)
  KIdentityProxyModel * const q_ptr;

  bool m_ignoreNextLayoutAboutToBeChanged;
  bool m_ignoreNextLayoutChanged;
  QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
  QModelIndexList m_proxyIndexes;

  void _k_sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void _k_sourceRowsInserted(const QModelIndex &parent, int start, int end);
  void _k_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void _k_sourceRowsRemoved(const QModelIndex &parent, int start, int end);
  void _k_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void _k_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

  void _k_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
  void _k_sourceColumnsInserted(const QModelIndex &parent, int start, int end);
  void _k_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
  void _k_sourceColumnsRemoved(const QModelIndex &parent, int start, int end);
  void _k_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
  void _k_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

  void _k_sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void _k_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

  void _k_sourceLayoutAboutToBeChanged();
  void _k_sourceLayoutChanged();
  void _k_sourceChildrenLayoutsAboutToBeChanged(const QModelIndex &parent1, const QModelIndex &parent2);
  void _k_sourceChildrenLayoutsChanged(const QModelIndex &parent1, const QModelIndex &parent2);
  void _k_sourceModelAboutToBeReset();
  void _k_sourceModelReset();
  void _k_sourceModelDestroyed();

};

/*!
    \class KIdentityProxyModel

    \brief The KIdentityProxyModel proxies its source model unmodified

    \ingroup model-view

    KIdentityProxyModel can be used to forward the structure of a source model exactly, with no sorting, filtering or other transformation. This is similar in concept to an identity matrix where A.I = A.

    Because it does no sorting or filtering, this class is most suitable to proxy models which transform the data() of the source model. For example, a proxy model could be created to define the font used, or the background colour, or the tooltip etc. This removes the need to implement all data handling in the same class that creates the structure of the model, and can also be used to create re-usable components.

    This also provides a way to change the data in the case where a source model is supplied by a third party which can not be modified.

    \code
      class DateFormatProxyModel : public QIdentityProxyModel
      {
        // ...

        void setDateFormatString(const QString &formatString)
        {
          m_formatString = formatString;
        }

        QVariant data(const QModelIndex &index, int role)
        {
          if (role != Qt::DisplayRole)
            return KIdentityProxyModel::data(index, role);

          const QDateTime dateTime = sourceModel()->data(SourceClass::DateRole).toDateTime();

          return dateTime.toString(m_formatString);
        }

      private:
        QString m_formatString;
      };
    \endcode

    \since 4.6
    \author Stephen Kelly <stephen@kdab.com>

*/

KIdentityProxyModel::KIdentityProxyModel(QObject* parent)
  : QAbstractProxyModel(parent), d_ptr(new KIdentityProxyModelPrivate(this))
{

}

KIdentityProxyModel::KIdentityProxyModel(KIdentityProxyModelPrivate* privateClass, QObject* parent)
  : QAbstractProxyModel(parent), d_ptr(privateClass)
{

}

KIdentityProxyModel::~KIdentityProxyModel()
{
    delete d_ptr;
}

int KIdentityProxyModel::columnCount(const QModelIndex& parent) const
{
    if (!sourceModel())
      return 0;
    return sourceModel()->columnCount(mapToSource(parent));
}

bool KIdentityProxyModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!sourceModel())
      return false;
    return sourceModel()->dropMimeData(data, action, row, column, mapToSource(parent));
}

QModelIndex KIdentityProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel())
      return QModelIndex();
    if (row < 0 || column < 0 || row >= sourceModel()->rowCount(parent) || column >= sourceModel()->columnCount(parent))
      return QModelIndex();
    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex = sourceModel()->index(row, column, sourceParent);
    Q_ASSERT(sourceIndex.isValid());
    return mapFromSource(sourceIndex);
}

bool KIdentityProxyModel::insertColumns(int column, int count, const QModelIndex& parent)
{
    if (!sourceModel())
      return false;
    return sourceModel()->insertColumns(column, count, mapToSource(parent));
}

bool KIdentityProxyModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!sourceModel())
      return false;
    return sourceModel()->insertRows(row, count, mapToSource(parent));
}

QModelIndex KIdentityProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid())
        return QModelIndex();
    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

QItemSelection KIdentityProxyModel::mapSelectionFromSource(const QItemSelection& selection) const
{
    QItemSelection proxySelection;

    if (!sourceModel())
      return proxySelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it)
    {
        const QItemSelectionRange range(mapFromSource(it->topLeft()), mapFromSource(it->bottomRight()));
        proxySelection.append(range);
    }

    return proxySelection;
}

QItemSelection KIdentityProxyModel::mapSelectionToSource(const QItemSelection& selection) const
{
    QItemSelection sourceSelection;

    if (!sourceModel())
      return sourceSelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it)
    {
        const QItemSelectionRange range(mapToSource(it->topLeft()), mapToSource(it->bottomRight()));
        sourceSelection.append(range);
    }

    return sourceSelection;
}

struct SourceModelIndex
{
    SourceModelIndex(int _r, int _c, void *_p, QAbstractItemModel *_m)
      : r(_r), c(_c), p(_p), m(_m)
    {

    }

    operator QModelIndex() { return reinterpret_cast<QModelIndex&>(*this); }

    int r, c;
    void *p;
    const QAbstractItemModel *m;
};

QModelIndex KIdentityProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();
    return SourceModelIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer(), sourceModel());
}

QModelIndexList KIdentityProxyModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
    if (!sourceModel())
        return QModelIndexList();

    const QModelIndexList sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
    QModelIndexList::const_iterator it = sourceList.constBegin();
    const QModelIndexList::const_iterator end = sourceList.constEnd();
    QModelIndexList proxyList;
    for ( ; it != end; ++it)
      proxyList.append(mapFromSource(*it));
    return proxyList;
}

QModelIndex KIdentityProxyModel::parent(const QModelIndex& child) const
{
    if (!sourceModel())
        return QModelIndex();

    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

bool KIdentityProxyModel::removeColumns(int column, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;

    return sourceModel()->removeColumns(column, count, mapToSource(parent));
}

bool KIdentityProxyModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;

    return sourceModel()->removeRows(row, count, mapToSource(parent));
}

int KIdentityProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;
    return sourceModel()->rowCount(mapToSource(parent));
}

void KIdentityProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    beginResetModel();

    if (sourceModel) {
        disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(_k_sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(_k_sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceColumnsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(_k_sourceColumnsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(_k_sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(_k_sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(modelAboutToBeReset()),
                   this, SLOT(_k_sourceModelAboutToBeReset()));
        disconnect(sourceModel, SIGNAL(internalDataReset()),
                   this, SLOT(resetInternalData()));
        disconnect(sourceModel, SIGNAL(modelReset()),
                   this, SLOT(_k_sourceModelReset()));
        disconnect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(_k_sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   this, SLOT(_k_sourceHeaderDataChanged(Qt::Orientation,int,int)));
        disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(_k_sourceLayoutAboutToBeChanged()));
        disconnect(sourceModel, SIGNAL(layoutChanged()),
                   this, SLOT(_k_sourceLayoutChanged()));
//         disconnect(sourceModel, SIGNAL(childrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(_k_sourceChildrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)));
//         disconnect(sourceModel, SIGNAL(childrenLayoutsChanged(QModelIndex,QModelIndex)),
//                    this, SLOT(_k_sourceChildrenLayoutsChanged(QModelIndex,QModelIndex)));
        disconnect(sourceModel, SIGNAL(destroyed()),
                   this, SLOT(_k_sourceModelDestroyed()));
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(_k_sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                SLOT(_k_sourceRowsInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(_k_sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(_k_sourceRowsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(_k_sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(_k_sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(_k_sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                SLOT(_k_sourceColumnsInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(_k_sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                SLOT(_k_sourceColumnsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(_k_sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(_k_sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(modelAboutToBeReset()),
                SLOT(_k_sourceModelAboutToBeReset()));
        connect(sourceModel, SIGNAL(internalDataReset()),
                SLOT(resetInternalData()));
        connect(sourceModel, SIGNAL(modelReset()),
                SLOT(_k_sourceModelReset()));
        connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                SLOT(_k_sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                SLOT(_k_sourceHeaderDataChanged(Qt::Orientation,int,int)));
        connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                SLOT(_k_sourceLayoutAboutToBeChanged()));
        connect(sourceModel, SIGNAL(layoutChanged()),
                SLOT(_k_sourceLayoutChanged()));
        // Hopefully this will be in Qt4.8
//         connect(sourceModel, SIGNAL(childrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)),
//                 SLOT(_k_sourceChildrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)));
//         connect(sourceModel, SIGNAL(childrenLayoutsChanged(QModelIndex,QModelIndex)),
//                 SLOT(_k_sourceChildrenLayoutsChanged(QModelIndex,QModelIndex)));
        connect(sourceModel, SIGNAL(destroyed()),
                SLOT(_k_sourceModelDestroyed()));
    }

    endResetModel();
}

void KIdentityProxyModelPrivate::_k_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    q->beginInsertColumns(q->mapFromSource(parent), start, end);
}

void KIdentityProxyModelPrivate::_k_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(KIdentityProxyModel);
    q->beginMoveColumns(q->mapFromSource(sourceParent), sourceStart, sourceEnd, q->mapFromSource(destParent), dest);
}

void KIdentityProxyModelPrivate::_k_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    q->beginRemoveColumns(q->mapFromSource(parent), start, end);
}

void KIdentityProxyModelPrivate::_k_sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    q->endInsertColumns();
}

void KIdentityProxyModelPrivate::_k_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)
    q->endMoveColumns();
}

void KIdentityProxyModelPrivate::_k_sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    q->endRemoveColumns();
}

void KIdentityProxyModelPrivate::_k_sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_Q(KIdentityProxyModel);
    q->dataChanged(q->mapFromSource(topLeft), q->mapFromSource(bottomRight));
}

void KIdentityProxyModelPrivate::_k_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_Q(KIdentityProxyModel);
    q->headerDataChanged(orientation, first, last);
}

void KIdentityProxyModelPrivate::_k_sourceLayoutAboutToBeChanged()
{
  if (m_ignoreNextLayoutAboutToBeChanged)
    return;

  Q_Q(KIdentityProxyModel);

  foreach(const QPersistentModelIndex &proxyPersistentIndex, q->persistentIndexList()) {
      m_proxyIndexes << proxyPersistentIndex;
      Q_ASSERT(proxyPersistentIndex.isValid());
      const QPersistentModelIndex srcPersistentIndex = q->mapToSource(proxyPersistentIndex);
      Q_ASSERT(srcPersistentIndex.isValid());
      m_layoutChangePersistentIndexes << srcPersistentIndex;
  }

  q->layoutAboutToBeChanged();
}

void KIdentityProxyModelPrivate::_k_sourceLayoutChanged()
{
  if (m_ignoreNextLayoutChanged)
    return;

  Q_Q(KIdentityProxyModel);

  for (int i = 0; i < m_proxyIndexes.size(); ++i) {
      q->changePersistentIndex(m_proxyIndexes.at(i), q->mapFromSource(m_layoutChangePersistentIndexes.at(i)));
  }

  m_layoutChangePersistentIndexes.clear();
  m_proxyIndexes.clear();

  q->layoutChanged();
}


void KIdentityProxyModelPrivate::_k_sourceChildrenLayoutsAboutToBeChanged(const QModelIndex &parent1, const QModelIndex &parent2)
{
    Q_Q(KIdentityProxyModel);

    m_ignoreNextLayoutAboutToBeChanged = true;

    //saved_persistent_indexes.clear();
    const QModelIndex proxyParent1 = q->mapFromSource(parent1);
    const QModelIndex proxyParent2 = q->mapFromSource(parent2);
    //emit q->childrenLayoutsAboutToBeChanged(proxyParent1, proxyParent2);
    emit q->layoutAboutToBeChanged();

    if (q->persistentIndexList().isEmpty())
        return;

//     if (it1 != source_index_mapping.constEnd())
//         saved_persistent_indexes = store_persistent_indexes(parent1);
//     if (it2 != source_index_mapping.constEnd())
//         saved_persistent_indexes += store_persistent_indexes(parent2);
}

void KIdentityProxyModelPrivate::_k_sourceChildrenLayoutsChanged(const QModelIndex &parent1, const QModelIndex &parent2)
{
    Q_Q(KIdentityProxyModel);

    m_ignoreNextLayoutChanged = true;

//     update_persistent_indexes(saved_persistent_indexes);
//     saved_persistent_indexes.clear();

    const QModelIndex proxyParent1 = q->mapFromSource(parent1);
    const QModelIndex proxyParent2 = q->mapFromSource(parent2);
//     emit q->childrenLayoutsChanged(proxyParent1, proxyParent2);
    emit q->layoutChanged();
}

void KIdentityProxyModelPrivate::_k_sourceModelAboutToBeReset()
{
    Q_Q(KIdentityProxyModel);
    q->beginResetModel();
}

void KIdentityProxyModelPrivate::_k_sourceModelReset()
{
    Q_Q(KIdentityProxyModel);
    q->endResetModel();
}

void KIdentityProxyModelPrivate::_k_sourceModelDestroyed()
{
    Q_Q(KIdentityProxyModel);
    q->endResetModel();
}

void KIdentityProxyModelPrivate::_k_sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    q->beginInsertRows(q->mapFromSource(parent), start, end);
}

void KIdentityProxyModelPrivate::_k_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(KIdentityProxyModel);
    q->beginMoveRows(q->mapFromSource(sourceParent), sourceStart, sourceEnd, q->mapFromSource(destParent), dest);
}

void KIdentityProxyModelPrivate::_k_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    q->beginRemoveRows(q->mapFromSource(parent), start, end);
}

void KIdentityProxyModelPrivate::_k_sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    q->endInsertRows();
}

void KIdentityProxyModelPrivate::_k_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)
    q->endMoveRows();
}

void KIdentityProxyModelPrivate::_k_sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(KIdentityProxyModel);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    q->endRemoveRows();
}

void KIdentityProxyModel::resetInternalData()
{

}

#include "kidentityproxymodel.moc"
