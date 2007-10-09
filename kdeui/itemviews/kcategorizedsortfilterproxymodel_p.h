/**
  * This file is part of the KDE project
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  *
  * ATTENTION: This code is based on the class QSortFilterProxyModel created
  *            by Trolltech ASA.
  */

#ifndef KCATEGORIZEDSORTFILTERPROXYMODEL_P_H
#define KCATEGORIZEDSORTFILTERPROXYMODEL_P_H

#include <QVector>
#include <QPair>
#include <QStringList>
#include <QModelIndexList>
#include <QHash>

#include <kglobal.h>


/**
  * @short This is an empty model for keeping safety on the KCategorizedSortFilterProxyModel
  *        class. When an instance is created of the KCategorizedSortFilterProxyModel class
  *        a KEmptyItemModel static object is set as source model, so the source model
  *        pointer is never 0
  *
  * @internal
  */
class KEmptyItemModel
    : public QAbstractItemModel
{
public:
    explicit KEmptyItemModel(QObject *parent = 0)
        : QAbstractItemModel(parent)
    {
    }

    QModelIndex index(int, int, const QModelIndex &) const
    {
        return QModelIndex();
    }

    QModelIndex parent(const QModelIndex &) const
    {
        return QModelIndex();
    }

    int rowCount(const QModelIndex &) const
    {
        return 0;
    }

    int columnCount(const QModelIndex &) const
    {
        return 0;
    }

    bool hasChildren(const QModelIndex &) const
    {
        return false;
    }

    QVariant data(const QModelIndex &, int) const
    {
        return QVariant();
    }
};


/**
  * @internal
  * Comes from qabstractitemmodel_p.h
  */
class QPersistentModelIndexData
{
public:
    QPersistentModelIndexData() : model(0) {}
    QPersistentModelIndexData(const QModelIndex &idx) : index(idx), model(idx.model()) {}
    QModelIndex index;
    const QAbstractItemModel *model;
    static QPersistentModelIndexData *create(const QModelIndex &index);
    static void destroy(QPersistentModelIndexData *data);
};


/**
  * @internal
  */
K_GLOBAL_STATIC(KEmptyItemModel, dummyModel) ///< Declaration of the static object


/**
  * @internal
  * @short This class will be called when sorting
  */
class LessThan
{
public:
    enum Purpose
    {
        GeneralPurpose = 0,
        CategoryPurpose
    };

    inline LessThan(const QModelIndex &parent,
                    const QAbstractItemModel *sourceModel,
                    const KCategorizedSortFilterProxyModel *proxyModel,
                    Purpose purpose)
        : parent(parent)
        , sourceModel(sourceModel)
        , proxyModel(proxyModel)
        , purpose(purpose)
    {
    }

    inline bool operator()(int leftRow,
                           int rightRow) const
    {
        QModelIndex leftIndex = sourceModel->index(leftRow, proxyModel->sortColumn(), parent);
        QModelIndex rightIndex = sourceModel->index(rightRow, proxyModel->sortColumn(), parent);

        if (purpose == GeneralPurpose)
        {
            return proxyModel->sortOrder() == Qt::AscendingOrder ?
                   proxyModel->lessThanGeneralPurpose(leftIndex, rightIndex) :
                   !proxyModel->lessThanGeneralPurpose(leftIndex, rightIndex);
        }

        return proxyModel->sortOrder() == Qt::AscendingOrder ?
               proxyModel->lessThanCategoryPurpose(leftIndex, rightIndex) :
               !proxyModel->lessThanCategoryPurpose(leftIndex, rightIndex);
    }

private:
    const QModelIndex parent;
    const QAbstractItemModel *sourceModel;
    const KCategorizedSortFilterProxyModel *proxyModel;
    const Purpose purpose;
};


/**
  * @internal
  */
class KCategorizedSortFilterProxyModel::Private
    : public QObject
{
    Q_OBJECT

public:
    Private(KCategorizedSortFilterProxyModel *parent);
    ~Private();

    QAbstractItemModel *staticEmptyModel()
    {
        return dummyModel;
    }

    class InternalInformation;

    // Methods

    void sourceItemsInserted(const QModelIndex &sourceParent, int start, int end, Qt::Orientation orientation);

    QMap<QModelIndex, InternalInformation*>::const_iterator createInternalInformation(const QModelIndex &sourceParent) const;
    void removeInternalInformation(const QModelIndex &sourceParent);
    void updateChildrenMapping(const QModelIndex &sourceParent, InternalInformation *parentInfo, Qt::Orientation orientation, int start, int end, int itemCount, bool remove);
    void buildSourceToProxyMapping(const QVector<int> &proxyToSource, QVector<int> &sourceToProxy) const;
    void sortSourceRows(QVector<int> &sourceRows, const QModelIndex &sourceParent) const;
    void insertSourceItems(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation, bool emitSignal = true);
    void removeSourceItems(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation, bool emitSignal = true);
    void proxyItemRange(const QVector<int> &sourceToProxy, const QVector<int> &sourceItems, int &proxyLow, int &proxyHigh) const;
    QVector<QPair<int, int > > proxyIntervalsForSourceItems(const QVector<int> &sourceToProxy, const QVector<int> &sourceItems) const;
    void removeProxyInterval(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, int proxyStart, int proxyEnd, const QModelIndex &proxyParent, Qt::Orientation orientation, bool emitSignal = true);
    void sourceItemsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end, Qt::Orientation orientation);
    void sourceItemsRemoved(const QModelIndex &sourceParent, int start, int end, Qt::Orientation orientation);

    void filterChanged();
    void handleFilterChanged(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QModelIndex &sourceParent, Qt::Orientation orientation);

    QModelIndexList storePersistentIndexes();
    void updatePersistentIndexes(const QModelIndexList &sourceIndexes);

    QVector<QPair<int, QVector<int > > > proxyIntervalsForSourceItemsToAdd(const QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation) const;
    QMap<QModelIndex, InternalInformation*>::const_iterator indexToIterator(const QModelIndex &proxyIndex) const;

    QModelIndex sourceToProxy(const QModelIndex &sourceIndex) const;
    QModelIndex proxyToSource(const QModelIndex &proxyIndex) const;

    bool indexValid(const QModelIndex &index) const;

    // Attributes

    KCategorizedSortFilterProxyModel *p;

    int sortColumn;
    Qt::SortOrder sortOrder;
    Qt::CaseSensitivity sortCaseSensitivity;
    int sortRole;
    bool sortLocaleAware : 1;

    bool dynamicSortFilter : 1;
    bool categorizedModel : 1;

    int filterColumn;
    QRegExp filterRegExp;
    int filterRole;

    mutable QMap<QModelIndex, InternalInformation*> sourceIndexMap;

    QList<QPersistentModelIndex> persistentIndexes;

    // Emulation of persistent.indexes from qabstractitemmodel_p.h
    QVector<QPersistentModelIndexData*> persistentIndexesData;

public Q_SLOTS:
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceColumnsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceColumnsRemoved(const QModelIndex &parent, int start, int end);
    void sourceLayoutAboutToBeChanged();
    void sourceLayoutChanged();
    void sourceReset();
};


/**
  * @internal
  * @short We will have an instance of this class on every QModelIndex generated
  *        by the proxy model, accessed through the internalPointer() method
  */
class KCategorizedSortFilterProxyModel::Private::InternalInformation
{
public:
    InternalInformation()
    {
    }

    ~InternalInformation()
    {
    }

    QMap<QModelIndex, InternalInformation*>::const_iterator mapIterator;
    QVector<QModelIndex> mappedChildren;
    QVector<int> sourceRows;
    QVector<int> proxyRows;
    QVector<int> sourceColumns;
    QVector<int> proxyColumns;
};


#endif
