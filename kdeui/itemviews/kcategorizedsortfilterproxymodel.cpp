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
  */

#include "kcategorizedsortfilterproxymodel.h"
#include "kcategorizedsortfilterproxymodel_p.h"

#include <limits.h>

#include <QItemSelection>
#include <QStringList>
#include <QSize>

KCategorizedSortFilterProxyModel::KCategorizedSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private(this))
{
    // For keeping safety, when we create the KCategorizedSortFilterProxyModel
    // object we set an empty model as the source model

    setSourceModel(dummyModel);
}

KCategorizedSortFilterProxyModel::~KCategorizedSortFilterProxyModel()
{
    delete d;
}

void KCategorizedSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (this->sourceModel())
    {
        disconnect(this->sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   d, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        disconnect(this->sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   d, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

        disconnect(this->sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   d, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                   d, SLOT(sourceRowsInserted(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   d, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                   d, SLOT(sourceColumnsInserted(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   d, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   d, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   d, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                   d, SLOT(sourceColumnsRemoved(QModelIndex,int,int)));

        disconnect(this->sourceModel(), SIGNAL(layoutAboutToBeChanged()),
                   d, SLOT(sourceLayoutAboutToBeChanged()));

        disconnect(this->sourceModel(), SIGNAL(layoutChanged()),
                   d, SLOT(sourceLayoutChanged()));

        disconnect(this->sourceModel(), SIGNAL(modelReset()), d, SLOT(sourceReset()));
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            d, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

    connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            d, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));

    connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            d, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            d, SLOT(sourceRowsInserted(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            d, SLOT(sourceColumnsAboutToBeInserted(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            d, SLOT(sourceColumnsInserted(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            d, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            d, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            d, SLOT(sourceColumnsAboutToBeRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            d, SLOT(sourceColumnsRemoved(QModelIndex,int,int)));

    connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
            d, SLOT(sourceLayoutAboutToBeChanged()));

    connect(sourceModel, SIGNAL(layoutChanged()),
            d, SLOT(sourceLayoutChanged()));

    connect(sourceModel, SIGNAL(modelReset()), d, SLOT(sourceReset()));

    reset();
}

void KCategorizedSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    d->sortColumn = column;
    d->sortOrder = order;

    QModelIndexList sourceIndexes = d->storePersistentIndexes();

    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->sourceIndexMap.constBegin();

    for (; it != d->sourceIndexMap.constEnd(); it++)
    {
        QModelIndex sourceParent = it.key();
        Private::InternalInformation *info = it.value();

        d->sortSourceRows(info->sourceRows, sourceParent);
        d->buildSourceToProxyMapping(info->sourceRows, info->proxyRows);
    }

    d->updatePersistentIndexes(sourceIndexes);

    emit layoutChanged();
}

QModelIndex KCategorizedSortFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    return d->sourceToProxy(sourceIndex);
}

QModelIndex KCategorizedSortFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    return d->proxyToSource(proxyIndex);
}

QItemSelection KCategorizedSortFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    return QSortFilterProxyModel::mapSelectionToSource(proxySelection);
}

QItemSelection KCategorizedSortFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    return QSortFilterProxyModel::mapSelectionFromSource(sourceSelection);
}

QRegExp KCategorizedSortFilterProxyModel::filterRegExp() const
{
    return d->filterRegExp;
}

void KCategorizedSortFilterProxyModel::setFilterRegExp(const QRegExp &regExp)
{
    d->filterRegExp = regExp;
    d->filterChanged();
}

void KCategorizedSortFilterProxyModel::setFilterRegExp(const QString &pattern)
{
    d->filterRegExp.setPatternSyntax(QRegExp::RegExp);
    d->filterRegExp.setPattern(pattern);
    d->filterChanged();
}

int KCategorizedSortFilterProxyModel::filterKeyColumn() const
{
    return d->filterColumn;
}

void KCategorizedSortFilterProxyModel::setFilterKeyColumn(int column)
{
    d->filterColumn = column;
    d->filterChanged();
}

Qt::CaseSensitivity KCategorizedSortFilterProxyModel::filterCaseSensitivity() const
{
    return d->filterRegExp.caseSensitivity();
}

void KCategorizedSortFilterProxyModel::setFilterCaseSensitivity(Qt::CaseSensitivity cs)
{
    if (cs == d->filterRegExp.caseSensitivity())
    {
        return;
    }

    d->filterRegExp.setCaseSensitivity(cs);
    d->filterChanged();
}

Qt::CaseSensitivity KCategorizedSortFilterProxyModel::sortCaseSensitivity() const
{
    return d->sortCaseSensitivity;
}

void KCategorizedSortFilterProxyModel::setSortCaseSensitivity(Qt::CaseSensitivity cs)
{
    if (cs == d->sortCaseSensitivity)
    {
        return;
    }

    d->sortCaseSensitivity = cs;
    sort(d->sortColumn, d->sortOrder);
}

bool KCategorizedSortFilterProxyModel::isSortLocaleAware() const
{
    return d->sortLocaleAware;
}

void KCategorizedSortFilterProxyModel::setSortLocaleAware(bool on)
{
    if (on == d->sortLocaleAware)
    {
        return;
    }

    d->sortLocaleAware = on;
    sort(d->sortColumn, d->sortOrder);
}

bool KCategorizedSortFilterProxyModel::dynamicSortFilter() const
{
    return d->dynamicSortFilter;
}

void KCategorizedSortFilterProxyModel::setDynamicSortFilter(bool enable)
{
    d->dynamicSortFilter = enable;
}

int KCategorizedSortFilterProxyModel::sortRole() const
{
    return d->sortRole;
}

void KCategorizedSortFilterProxyModel::setSortRole(int role)
{
    if (role == d->sortRole)
    {
        return;
    }

    d->sortRole = role;
    sort(d->sortColumn, d->sortOrder);
}

int KCategorizedSortFilterProxyModel::filterRole() const
{
    return d->filterRole;
}

void KCategorizedSortFilterProxyModel::setFilterRole(int role)
{
    if (role == d->filterRole)
    {
        return;
    }

    d->filterRole = role;
    d->filterChanged();
}

QModelIndex KCategorizedSortFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
    {
        return QModelIndex();
    }

    QModelIndex sourceParent = d->proxyToSource(parent);
    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->createInternalInformation(sourceParent);
    if (it.value()->sourceRows.count() <= row || it.value()->sourceColumns.count() <= column)
    {
        return QModelIndex();
    }

    return createIndex(row, column, *it);
}

bool KCategorizedSortFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return true;
    }

    if (!sourceModel()->hasChildren(sourceParent))
    {
        return false;
    }

    Private::InternalInformation *info = d->createInternalInformation(sourceParent).value();

    return info->sourceRows.count() && info->sourceColumns.count();
}

QVariant KCategorizedSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    QModelIndex sourceIndex = d->proxyToSource(index);

    if (index.isValid() && !sourceIndex.isValid())
    {
        return QVariant();
    }

    return sourceModel()->data(sourceIndex, role);
}

bool KCategorizedSortFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QModelIndex sourceIndex = d->proxyToSource(index);

    if (index.isValid() && !sourceIndex.isValid())
    {
        return false;
    }

    return sourceModel()->setData(sourceIndex, value, role);
}

QVariant KCategorizedSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->createInternalInformation(QModelIndex());
    int sourceSection;

    if (section < 0)
    {
        return QVariant();
    }

    if (orientation == Qt::Vertical)
    {
        if (section >= it.value()->sourceRows.count())
        {
            return QVariant();
        }

        sourceSection = it.value()->sourceRows.at(section);
    }
    else
    {
        if (section >= it.value()->sourceColumns.count())
        {
            return QVariant();
        }

        sourceSection = it.value()->sourceColumns.at(section);
    }

    return sourceModel()->headerData(sourceSection, orientation, role);
}

bool KCategorizedSortFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->createInternalInformation(QModelIndex());
    int sourceSection;

    if (section < 0)
    {
        return false;
    }

    if (orientation == Qt::Vertical)
    {
        if (section >= it.value()->sourceRows.count())
        {
            return false;
        }

        sourceSection = it.value()->sourceRows.at(section);
    }
    else
    {
        if (section >= it.value()->sourceColumns.count())
        {
            return false;
        }

        sourceSection = it.value()->sourceColumns.at(section);
    }

    return sourceModel()->setHeaderData(sourceSection, orientation, value, role);
}

QModelIndex KCategorizedSortFilterProxyModel::buddy(const QModelIndex &index) const
{
    if (!d->indexValid(index))
    {
        return QModelIndex();
    }

    QModelIndex sourceIndex = d->proxyToSource(index);
    QModelIndex sourceBuddy = sourceModel()->buddy(sourceIndex);

    if (sourceIndex == sourceBuddy)
    {
        return index;
    }

    return d->sourceToProxy(sourceBuddy);
}

Qt::ItemFlags KCategorizedSortFilterProxyModel::flags(const QModelIndex &index) const
{
    QModelIndex sourceIndex;

    if (d->indexValid(index))
    {
        sourceIndex = d->proxyToSource(index);
    }

    return sourceModel()->flags(sourceIndex);
}

QModelIndexList KCategorizedSortFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    return QSortFilterProxyModel::match(start, role, value, hits, flags);
}

QModelIndex KCategorizedSortFilterProxyModel::parent(const QModelIndex &index) const
{
    if (!d->indexValid(index))
    {
        return QModelIndex();
    }

    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->indexToIterator(index);

    QModelIndex sourceParent = it.key();
    QModelIndex proxyParent = d->sourceToProxy(sourceParent);

    return proxyParent;
}

QSize KCategorizedSortFilterProxyModel::span(const QModelIndex &index) const
{
    QModelIndex sourceIndex = d->proxyToSource(index);

    if (index.isValid() && !sourceIndex.isValid())
    {
        return QSize();
    }

    return sourceModel()->span(sourceIndex);
}

QStringList KCategorizedSortFilterProxyModel::mimeTypes() const
{
    return sourceModel()->mimeTypes();
}

QMimeData *KCategorizedSortFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexes;

    for (int i = 0; i < indexes.count(); i++)
    {
        sourceIndexes << d->proxyToSource(indexes.at(i));
    }

    return sourceModel()->mimeData(sourceIndexes);
}

bool KCategorizedSortFilterProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if ((row == -1) && (column == -1))
    {
        return sourceModel()->dropMimeData(data, action, -1, -1, d->proxyToSource(parent));
    }

    QModelIndex proxyIndex = index(row, column, parent);
    QModelIndex sourceIndex = d->proxyToSource(proxyIndex);

    return sourceModel()->dropMimeData(data, action, sourceIndex.row(), sourceIndex.column(), sourceIndex.parent());
}

Qt::DropActions KCategorizedSortFilterProxyModel::supportedDropActions() const
{
    return sourceModel()->supportedDropActions();
}

int KCategorizedSortFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return 0;
    }

    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->createInternalInformation(sourceParent);

    return it.value()->sourceRows.count();
}

int KCategorizedSortFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return 0;
    }

    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = d->createInternalInformation(sourceParent);

    return it.value()->sourceColumns.count();
}

bool KCategorizedSortFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0)
    {
        return false;
    }

    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return false;
    }

    Private::InternalInformation *info = d->createInternalInformation(sourceParent).value();

    if (row > info->sourceRows.count())
    {
        return false;
    }

    int sourceRow = (row >= info->sourceRows.count() ? info->sourceRows.count()
                                                     : info->sourceRows.at(row));

    return sourceModel()->insertRows(sourceRow, count, sourceParent);
}

bool KCategorizedSortFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (column < 0 || count <= 0)
    {
        return false;
    }

    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return false;
    }

    Private::InternalInformation *info = d->createInternalInformation(sourceParent).value();

    if (column > info->sourceColumns.count())
    {
        return false;
    }

    int sourceColumn = (column >= info->sourceColumns.count() ? info->sourceColumns.count()
                                                              : info->sourceColumns.at(column));

    return sourceModel()->insertColumns(sourceColumn, count, sourceParent);
}

bool KCategorizedSortFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0)
    {
        return false;
    }

    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return false;
    }

    Private::InternalInformation *info = d->createInternalInformation(sourceParent).value();

    if (row + count > info->sourceRows.count())
    {
        return false;
    }

    if ((count == 1) || ((sortColumn() < 0) && (info->proxyRows.count() == info->sourceRows.count())))
    {
        int sourceRow = info->sourceRows.at(row);
        return sourceModel()->removeRows(sourceRow, count, sourceParent);
    }

    // Remove corresponding source intervals
    // ### if this proves to be slow, switch it to a single-row removal
    QVector<int> rows;

    for (int i = row; i < row + count; i++)
    {
        rows << info->sourceRows.at(i);
    }

    qSort(rows.begin(), rows.end());

    int pos = rows.count() - 1;
    bool ok = true;

    while (pos >= 0)
    {
        const int sourceEnd = rows.at(pos--);
        int sourceStart = sourceEnd;

        while ((pos >= 0) && (rows.at(pos) == (sourceStart - 1)))
        {
            sourceStart--;
            pos--;
        }

        ok = ok && sourceModel()->removeRows(sourceStart, sourceEnd - sourceStart + 1, sourceParent);
    }

    return ok;
}

bool KCategorizedSortFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (column < 0 || count <= 0)
    {
        return false;
    }

    QModelIndex sourceParent = d->proxyToSource(parent);

    if (parent.isValid() && !sourceParent.isValid())
    {
        return false;
    }

    Private::InternalInformation *info = d->createInternalInformation(sourceParent).value();

    if (column + count > info->sourceColumns.count())
    {
        return false;
    }

    if ((count == 1) || ((d->sortColumn < 0) && (info->proxyColumns.count() == info->sourceColumns.count())))
    {
        int sourceColumn = info->sourceColumns.at(column);
        return sourceModel()->removeColumns(sourceColumn, count, sourceParent);
    }

    QVector<int> columns;

    for (int i = column; i < column + count; i++)
    {
        columns << info->sourceColumns.at(i);
    }

    qSort(columns.begin(), columns.end());

    int pos = columns.count() - 1;
    bool ok = true;

    while (pos >= 0)
    {
        const int sourceEnd = columns.at(pos--);
        int sourceStart = sourceEnd;

        while ((pos >= 0) && (columns.at(pos) == (sourceStart - 1)))
        {
            sourceStart--;
            pos--;
        }

        ok = ok && sourceModel()->removeColumns(sourceStart, sourceEnd - sourceStart + 1, sourceParent);
    }

    return ok;
}

void KCategorizedSortFilterProxyModel::fetchMore(const QModelIndex &parent)
{
    QModelIndex sourceParent;

    if (d->indexValid(parent))
    {
        sourceParent = d->proxyToSource(parent);
    }

    sourceModel()->fetchMore(sourceParent);
}

bool KCategorizedSortFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
    QModelIndex sourceParent;

    if (d->indexValid(parent))
    {
        sourceParent = d->proxyToSource(parent);
    }

    return sourceModel()->canFetchMore(sourceParent);
}

int KCategorizedSortFilterProxyModel::sortColumn() const
{
    return d->sortColumn;
}

Qt::SortOrder KCategorizedSortFilterProxyModel::sortOrder() const
{
    return d->sortOrder;
}

bool KCategorizedSortFilterProxyModel::isCategorizedModel() const
{
    return d->categorizedModel;
}

void KCategorizedSortFilterProxyModel::setCategorizedModel(bool categorizedModel)
{
    d->categorizedModel = categorizedModel;
}

void KCategorizedSortFilterProxyModel::setFilterWildcard(const QString &pattern)
{
    d->filterRegExp.setPatternSyntax(QRegExp::Wildcard);
    d->filterRegExp.setPattern(pattern);
    d->filterChanged();
}

void KCategorizedSortFilterProxyModel::setFilterFixedString(const QString &pattern)
{
    d->filterRegExp.setPatternSyntax(QRegExp::FixedString);
    d->filterRegExp.setPattern(pattern);
    d->filterChanged();
}

void KCategorizedSortFilterProxyModel::clear()
{
    emit layoutAboutToBeChanged();

    // store the persistent indexes
    QModelIndexList sourceIndexes = d->storePersistentIndexes();

    qDeleteAll(d->sourceIndexMap);
    d->sourceIndexMap.clear();

    // update the persistent indexes
    d->updatePersistentIndexes(sourceIndexes);

    emit layoutChanged();
}

void KCategorizedSortFilterProxyModel::invalidate()
{
    emit layoutAboutToBeChanged();

    // store the persistent indexes
    QModelIndexList sourceIndexes = d->storePersistentIndexes();

    qDeleteAll(d->sourceIndexMap);
    d->sourceIndexMap.clear();

    // update the persistent indexes
    d->updatePersistentIndexes(sourceIndexes);

    emit layoutChanged();
}

bool KCategorizedSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (d->filterRegExp.isEmpty() || d->filterColumn == -1)
    {
        return true;
    }

    QModelIndex sourceIndex = sourceModel()->index(sourceRow, d->filterColumn, sourceParent);

    if (!sourceIndex.isValid()) // the column may not exist
    {
        return true;
    }

    QString key = sourceModel()->data(sourceIndex, d->filterRole).toString();
    return key.contains(d->filterRegExp);
}

bool KCategorizedSortFilterProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceColumn);
    Q_UNUSED(sourceParent);

    return true;
}

void KCategorizedSortFilterProxyModel::Private::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid())
    {
        return;
    }

    QModelIndex sourceParent = topLeft.parent();
    QMap<QModelIndex, Private::InternalInformation*>::const_iterator it = createInternalInformation(sourceParent);

    if (it == sourceIndexMap.constEnd())
    {
        // This index is not mapped
        return;
    }

    Private::InternalInformation *info = it.value();

    QVector<int> sourceRowsRemove;
    QVector<int> sourceRowsInsert;
    QVector<int> sourceRowsChange;
    QVector<int> sourceRowsResort;

    int end = qMin(bottomRight.row(), info->proxyRows.count() - 1);

    for (int sourceRow = topLeft.row(); sourceRow <= end; sourceRow++)
    {
        if (dynamicSortFilter)
        {
            if (info->proxyRows.at(sourceRow) != -1)
            {
                if (!p->filterAcceptsRow(sourceRow, sourceParent))
                {
                    // This source row didn't pass the filter, remove it
                    sourceRowsRemove << sourceRow;
                }
                else if (sortColumn >= topLeft.column() &&
                         sortColumn <= bottomRight.column())
                {
                    // This source row has changed in a way that may affect sorted order
                    sourceRowsResort << sourceRow;
                }
                else
                {
                    // This source row has changed, not affected filtering nor sorting
                    sourceRowsChange << sourceRow;
                }
            }
            else
            {
                if (p->filterAcceptsRow(sourceRow, sourceParent))
                {
                    sourceRowsInsert << sourceRow;
                }
            }
        }
        else
        {
            if (info->proxyRows.at(sourceRow) != -1)
            {
                sourceRowsChange << sourceRow;
            }
        }
    }

    if (!sourceRowsRemove.isEmpty())
    {
        removeSourceItems(info->proxyRows, info->sourceRows,
                          sourceRowsRemove, sourceParent, Qt::Vertical);
    }

    if (!sourceRowsResort.isEmpty())
    {
        emit p->layoutAboutToBeChanged();

        QModelIndexList sourceIndexes = storePersistentIndexes();

        removeSourceItems(info->proxyRows, info->sourceRows,
                          sourceRowsResort, sourceParent, Qt::Vertical, false);

        sortSourceRows(sourceRowsResort, sourceParent);

        insertSourceItems(info->proxyRows, info->sourceRows,
                          sourceRowsResort, sourceParent, Qt::Vertical, false);

        updatePersistentIndexes(sourceIndexes);

        // We want to emit dataChanged for the rows too
        sourceRowsChange += sourceRowsResort;

        int proxyStartRow;
        int proxyEndRow;
        proxyItemRange(info->proxyRows, sourceRowsChange, proxyStartRow, proxyEndRow);
        QModelIndex proxyTopLeft = p->createIndex(proxyStartRow, info->proxyColumns.at(topLeft.column()), *it);
        QModelIndex proxyBottomRight = p->createIndex(proxyEndRow, info->proxyColumns.at(bottomRight.column()), *it);

        emit p->dataChanged(proxyTopLeft, proxyBottomRight);

        emit p->layoutChanged();
    }

    if (!sourceRowsChange.isEmpty())
    {
        int proxyStartRow;
        int proxyEndRow;

        proxyItemRange(info->proxyRows, sourceRowsChange, proxyStartRow, proxyEndRow);

        // FIXME: Find the proxy column range too
        if (proxyEndRow >= 0)
        {
            QModelIndex proxyTopLeft = p->createIndex(proxyStartRow, info->proxyColumns.at(topLeft.column()), *it);
            QModelIndex proxyBottomRight = p->createIndex(proxyEndRow, info->proxyColumns.at(bottomRight.column()), *it);

            emit p->dataChanged(proxyTopLeft, proxyBottomRight);
        }
    }

    if (!sourceRowsInsert.isEmpty())
    {
        sortSourceRows(sourceRowsInsert, sourceParent);
        insertSourceItems(info->proxyRows, info->sourceRows,
                          sourceRowsInsert, sourceParent, Qt::Vertical);
    }
}

void KCategorizedSortFilterProxyModel::Private::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Private::InternalInformation *info = createInternalInformation(QModelIndex()).value();

    int proxyStart = (orientation == Qt::Vertical) ? info->proxyRows.at(first)
                                                   : info->proxyColumns.at(first);

    int proxyEnd = (orientation == Qt::Vertical) ? info->proxyRows.at(last)
                                                 : info->proxyColumns.at(last);

    emit p->headerDataChanged(orientation, proxyStart, proxyEnd);
}

void KCategorizedSortFilterProxyModel::Private::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void KCategorizedSortFilterProxyModel::Private::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    sourceItemsInserted(parent, start, end, Qt::Vertical);
}

void KCategorizedSortFilterProxyModel::Private::sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void KCategorizedSortFilterProxyModel::Private::sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
    sourceItemsInserted(parent, start, end, Qt::Horizontal);
}

void KCategorizedSortFilterProxyModel::Private::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    sourceItemsAboutToBeRemoved(parent, start, end, Qt::Vertical);
}

void KCategorizedSortFilterProxyModel::Private::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    sourceItemsRemoved(parent, start, end, Qt::Vertical);
}

void KCategorizedSortFilterProxyModel::Private::sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    sourceItemsAboutToBeRemoved(parent, start, end, Qt::Horizontal);
}

void KCategorizedSortFilterProxyModel::Private::sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
    sourceItemsRemoved(parent, start, end, Qt::Horizontal);
}

void KCategorizedSortFilterProxyModel::Private::sourceLayoutAboutToBeChanged()
{
    persistentIndexes.clear();

    if (persistentIndexesData.isEmpty())
        return;

    emit p->layoutAboutToBeChanged();

    QModelIndexList sourceIndexes = storePersistentIndexes();
    QModelIndexList::const_iterator it;

    for (it = sourceIndexes.constBegin(); it != sourceIndexes.constEnd(); it++)
    {
        persistentIndexes << (*it);
    }
}

void KCategorizedSortFilterProxyModel::Private::sourceLayoutChanged()
{
    if (persistentIndexes.isEmpty())
    {
        p->invalidate();

        return;
    }

    QModelIndexList sourceIndexes;
    QList<QPersistentModelIndex>::const_iterator it;
    it = persistentIndexes.constBegin();

    for (; it != persistentIndexes.constEnd(); it++)
    {
        sourceIndexes << (*it);
    }

    qDeleteAll(sourceIndexMap);
    sourceIndexMap.clear();

    updatePersistentIndexes(sourceIndexes);
    persistentIndexes.clear();

    emit p->layoutChanged();
}

void KCategorizedSortFilterProxyModel::Private::sourceReset()
{
    p->reset();
}


// =============================================================================


KCategorizedSortFilterProxyModel::Private::Private(KCategorizedSortFilterProxyModel *parent)
    : QObject(parent)
    , p(parent)
    , sortColumn(-1)
    , sortOrder(Qt::AscendingOrder)
    , sortCaseSensitivity(Qt::CaseSensitive)
    , sortRole(Qt::DisplayRole)
    , sortLocaleAware(false)
    , dynamicSortFilter(false)
    , categorizedModel(false)
    , filterColumn(0)
    , filterRole(Qt::DisplayRole)
{
}

KCategorizedSortFilterProxyModel::Private::~Private()
{
}

void KCategorizedSortFilterProxyModel::Private::sourceItemsInserted(const QModelIndex &sourceParent,
                                                                    int start,
                                                                    int end,
                                                                    Qt::Orientation orientation)
{
    // If we have invalid indexes, return
    if ((start < 0) || (end < 0))
        return;

    // Search the parent on the source indexes map
    QMap<QModelIndex, InternalInformation*>::const_iterator it = sourceIndexMap.constFind(sourceParent);

    // If the parent wasn't found on the map, create the mapping for it
    if (it == sourceIndexMap.constEnd())
    {
        if (sourceParent.isValid())
        {
            QModelIndex sourceGrandParent = sourceParent.parent();
            it = sourceIndexMap.constFind(sourceGrandParent);

            // If we don't have mapping for the grand parent, return
            if (it == sourceIndexMap.constEnd())
            {
                return;
            }

            InternalInformation *grandParentInfo = it.value();

            // NOTE: This shouldn't check the orientation and ask for sourceParent
            //       row or column depending on that ?

            // If the parent is filtered, return
            if (grandParentInfo->proxyRows.at(sourceParent.row()) == -1)
            {
                return;
            }

            // !NOTE
        }

        // Create the internal information
        it = createInternalInformation(sourceParent);
        InternalInformation *info = it.value();

        QModelIndex proxyParent = sourceToProxy(sourceParent);

        // NOTE: This shouldn't check the orientation and emit rows or columns
        //       signals depending on that ?

        // Emit row signals
        if (info->sourceRows.count() > 0)
        {
            emit p->beginInsertRows(proxyParent, 0, info->sourceRows.count() - 1);
            emit p->endInsertRows();
        }

        // Emit column signals
        if (info->sourceColumns.count() > 0)
        {
            emit p->beginInsertColumns(proxyParent, 0, info->sourceColumns.count() - 1);
            emit p->endInsertColumns();
        }

        // !NOTE

        return;
    }

    // Source parent was found, read its info
    InternalInformation *info = it.value();
    QVector<int> &sourceToProxy = (orientation == Qt::Vertical) ? info->proxyRows : info->proxyColumns;
    QVector<int> &proxyToSource = (orientation == Qt::Vertical) ? info->sourceRows : info->sourceColumns;

    int itemCount = end - start + 1;
    int oldItemCount = sourceToProxy.size();

    // We have inserted childs for the source parent, so it is needed to update its childs
    updateChildrenMapping(sourceParent, info, orientation, start, end, itemCount, false);

    // Make room for new elements with a dummy default value
    sourceToProxy.insert(start, itemCount, -1);

    if (start < oldItemCount)
    {
        int proxyCount = proxyToSource.size();

        for (int i = 0; i < proxyCount; i++)
        {
            int sourceItem = proxyToSource.at(i);

            if (sourceItem >= start)
            {
                proxyToSource.replace(i, sourceItem + itemCount);
            }
        }

        buildSourceToProxyMapping(proxyToSource, sourceToProxy);
    }

    QVector<int> sourceItems;
    for (int i = start; i <= end; i++)
    {
        if ((orientation == Qt::Vertical) ? p->filterAcceptsRow(i, sourceParent)
                                          : p->filterAcceptsColumn(i, sourceParent))
        {
            sourceItems << i;
        }
    }

    if (orientation == Qt::Vertical)
    {
        sortSourceRows(sourceItems, sourceParent);
    }

    insertSourceItems(sourceToProxy, proxyToSource, sourceItems, sourceParent, orientation);
}

QMap<QModelIndex, KCategorizedSortFilterProxyModel::Private::InternalInformation*>::const_iterator KCategorizedSortFilterProxyModel::Private::createInternalInformation(const QModelIndex &sourceParent) const
{
    QMap<QModelIndex, InternalInformation*>::const_iterator it = sourceIndexMap.constFind(sourceParent);

    // If source parent was already mapped, return it
    if (it != sourceIndexMap.constEnd())
    {
        return it;
    }

    InternalInformation *info = new InternalInformation;

    if (!p->sourceModel())
    {
        return QMap<QModelIndex, InternalInformation*>::const_iterator(sourceIndexMap.insert(sourceParent, info));
    }

    // Ignore those rows filtered
    int sourceRows = p->sourceModel()->rowCount(sourceParent);
    for (int i = 0; i < sourceRows; i++)
    {
        if (p->filterAcceptsRow(i, sourceParent))
        {
            info->sourceRows << i;
        }
    }

    // Ignore those columns filtered
    int sourceColumns = p->sourceModel()->columnCount(sourceParent);
    for (int i = 0; i < sourceColumns; i++)
    {
        if (p->filterAcceptsColumn(i, sourceParent))
        {
            info->sourceColumns << i;
        }
    }

    sortSourceRows(info->sourceRows, sourceParent);

    info->proxyRows.resize(sourceRows);
    buildSourceToProxyMapping(info->sourceRows, info->proxyRows);

    info->proxyColumns.resize(sourceColumns);
    buildSourceToProxyMapping(info->sourceColumns, info->proxyColumns);

    it = QMap<QModelIndex, InternalInformation*>::const_iterator(sourceIndexMap.insert(sourceParent, info));

    info->mapIterator = it;

    if (sourceParent.isValid())
    {
        QModelIndex sourceGrandParent = sourceParent.parent();
        QMap<QModelIndex, InternalInformation*>::const_iterator git = createInternalInformation(sourceGrandParent);
        git.value()->mappedChildren.append(sourceParent);
    }

    return it;
}

void KCategorizedSortFilterProxyModel::Private::removeInternalInformation(const QModelIndex &sourceParent)
{
    if (InternalInformation *info = sourceIndexMap.take(sourceParent))
    {
        for (int i = 0; i < info->mappedChildren.size(); i++)
        {
            removeInternalInformation(info->mappedChildren.at(i));
        }

        delete info;
    }
}

void KCategorizedSortFilterProxyModel::Private::updateChildrenMapping(const QModelIndex &sourceParent, InternalInformation *parentInfo, Qt::Orientation orientation, int start, int end, int itemCount, bool remove)
{
    QVector<QPair<QModelIndex, InternalInformation*> > movedSourceIndexMappings;
    QVector<QModelIndex>::iterator it = parentInfo->mappedChildren.begin();

    for (; it != parentInfo->mappedChildren.end();)
    {
        const QModelIndex sourceChildIndex = *it;
        const int pos = (orientation == Qt::Vertical) ? sourceChildIndex.row()
                                                      : sourceChildIndex.column();

        if (pos < start)
        {
            // The index is not affected by the change

            it++;
        }
        else if (remove && pos <= end)
        {
            // The index is affected, and is being removed

            it = parentInfo->mappedChildren.erase(it);
            removeInternalInformation(sourceChildIndex);
        }
        else
        {
            // The affected set of items is above the current one, we need to update
            // its indexes

            QModelIndex newIndex;
            const int newPos = remove ? pos - itemCount
                                      : pos + itemCount;

            if (orientation == Qt::Vertical)
            {
                newIndex = p->sourceModel()->index(newPos,
                                                   sourceChildIndex.column(),
                                                   sourceParent);
            }
            else
            {
                newIndex = p->sourceModel()->index(sourceChildIndex.row(),
                                                   newPos,
                                                   sourceParent);
            }

            *it = newIndex;
            it++;

            // Update internal information
            InternalInformation *info = sourceIndexMap.take(sourceChildIndex);

            movedSourceIndexMappings << QPair<QModelIndex, InternalInformation*>(newIndex, info);
        }
    }

    QVector<QPair<QModelIndex, InternalInformation*> >::iterator it2 = movedSourceIndexMappings.begin();
    for (; it2 != movedSourceIndexMappings.end(); it2++)
        (*it2).second->mapIterator = sourceIndexMap.insert((*it2).first, (*it2).second);
}

void KCategorizedSortFilterProxyModel::Private::buildSourceToProxyMapping(const QVector<int> &proxyToSource, QVector<int> &sourceToProxy) const
{
    sourceToProxy.fill(-1);

    for (int i = 0; i < proxyToSource.size(); i++)
    {
        sourceToProxy[proxyToSource.at(i)] = i;
    }
}

void KCategorizedSortFilterProxyModel::Private::sortSourceRows(QVector<int> &sourceRows, const QModelIndex &sourceParent) const
{
    if ((sortColumn < 0) || (!sourceRows.count()))
    {
        return;
    }

    if (!categorizedModel)
    {
        LessThan lt(sourceParent, p->sourceModel(), p, LessThan::CategoryPurpose);
        qStableSort(sourceRows.begin(), sourceRows.end(), lt);

        return;
    }

    QVector<int> sourceRowSortedList(sourceRows);

    LessThan ltgp(sourceParent, p->sourceModel(), p, LessThan::GeneralPurpose);
    qStableSort(sourceRowSortedList.begin(), sourceRowSortedList.end(), ltgp);

    // Explore categories
    QString prevCategory = p->sourceModel()->data(p->sourceModel()->index(sourceRowSortedList.at(0), sortColumn, sourceParent), KCategorizedSortFilterProxyModel::CategoryRole).toString();

    QString lastCategory = prevCategory;
    QMap<QString, QVector<int> > categoriesRows;
    QVector<int> modelRowList;
    QStringList categories;
    foreach (int row, sourceRowSortedList)
    {
        lastCategory = p->sourceModel()->data(p->sourceModel()->index(row, sortColumn, sourceParent), KCategorizedSortFilterProxyModel::CategoryRole).toString();

        if (prevCategory != lastCategory)
        {
            if (categoriesRows.contains(prevCategory) && modelRowList.count())
            {
                categoriesRows[prevCategory] << modelRowList;
            }
            else if (modelRowList.count())
            {
                categoriesRows.insert(prevCategory, modelRowList);
            }

            if (!categories.contains(prevCategory) && modelRowList.count())
            {
                categories << prevCategory;
            }

            modelRowList.clear();
        }

        if ((categoriesRows.contains(lastCategory) &&
             !categoriesRows[lastCategory].contains(row)) ||
            !categoriesRows.contains(lastCategory))
        {
            modelRowList << row;
        }

        prevCategory = lastCategory;
    }

    if (categoriesRows.contains(prevCategory) && modelRowList.count())
    {
        categoriesRows[prevCategory] << modelRowList;
    }
    else if (modelRowList.count())
    {
        categoriesRows.insert(prevCategory, modelRowList);
    }

    if (!categories.contains(prevCategory) && modelRowList.count())
    {
        categories << prevCategory;
    }

    sourceRows.clear();

    LessThan ltcp(sourceParent, p->sourceModel(), p, LessThan::CategoryPurpose);
    foreach (const QString &key, categories)
    {
        QVector<int> &rowList = categoriesRows[key];
        qStableSort(rowList.begin(), rowList.end(), ltcp);
        sourceRows << rowList;
    }

}

void KCategorizedSortFilterProxyModel::Private::insertSourceItems(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation, bool emitSignal)
{
    QModelIndex proxyParent = this->sourceToProxy(sourceParent);

    // If source parent is not mapped, return
    if (!proxyParent.isValid() && sourceParent.isValid())
    {
        return;
    }

    QVector<QPair<int, QVector<int> > > proxyIntervals;

    proxyIntervals = proxyIntervalsForSourceItemsToAdd(proxyToSource, sourceItems, sourceParent, orientation);

    for (int i = proxyIntervals.size() - 1; i >= 0; i--)
    {
        QPair<int, QVector<int> > interval = proxyIntervals.at(i);
        int proxyStart = interval.first;
        QVector<int> sourceItems = interval.second;
        int proxyEnd = proxyStart + sourceItems.size() - 1;

        if (emitSignal)
        {
            if (orientation == Qt::Vertical)
            {
                emit p->beginInsertRows(proxyParent, proxyStart, proxyEnd);
            }
            else
            {
                emit p->beginInsertColumns(proxyParent, proxyStart, proxyEnd);
            }
        }

        for (int i = 0; i < sourceItems.size(); i++)
        {
            proxyToSource.insert(proxyStart + i, sourceItems.at(i));
        }

        buildSourceToProxyMapping(proxyToSource, sourceToProxy);

        if (emitSignal)
        {
            if (orientation == Qt::Vertical)
            {
                emit p->endInsertRows();
            }
            else
            {
                emit p->endInsertColumns();
            }
        }
    }
}

void KCategorizedSortFilterProxyModel::Private::removeSourceItems(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation, bool emitSignal)
{
    QModelIndex proxyParent = this->sourceToProxy(sourceParent);

    if (!proxyParent.isValid() && sourceParent.isValid())
    {
        // This index has been already removed
        return;
    }

    QVector<QPair<int, int> > proxyIntervals;
    proxyIntervals = proxyIntervalsForSourceItems(sourceToProxy, sourceItems);

    for (int i = proxyIntervals.size() - 1; i >= 0; i--)
    {
        QPair<int, int> interval = proxyIntervals.at(i);
        int proxyStart = interval.first;
        int proxyEnd = interval.second;
        removeProxyInterval(sourceToProxy, proxyToSource, proxyStart, proxyEnd, proxyParent, orientation, emitSignal);
    }
}

void KCategorizedSortFilterProxyModel::Private::proxyItemRange(const QVector<int> &sourceToProxy, const QVector<int> &sourceItems, int &proxyLow, int &proxyHigh) const
{
    proxyLow = INT_MAX;
    proxyHigh = INT_MIN;

    foreach (int sourceItem, sourceItems)
    {
        int proxyItem = sourceToProxy.at(sourceItem);

        proxyLow = qMin(proxyLow, proxyItem);

        proxyHigh = qMax(proxyHigh, proxyItem);
    }
}

QVector<QPair<int, int > > KCategorizedSortFilterProxyModel::Private::proxyIntervalsForSourceItems(const QVector<int> &sourceToProxy, const QVector<int> &sourceItems) const
{
    QVector<QPair<int, int> > proxyIntervals;

    if (sourceItems.isEmpty())
    {
        return proxyIntervals;
    }

    int sourceItemsIndex = 0;

    while (sourceItemsIndex < sourceItems.size())
    {
        int firstProxyItem = sourceToProxy.at(sourceItems.at(sourceItemsIndex));
        int lastProxyItem = firstProxyItem;
        sourceItemsIndex++;

        // Find end of interval
        while ((sourceItemsIndex < sourceItems.size()) &&
               (sourceToProxy.at(sourceItems.at(sourceItemsIndex)) == lastProxyItem + 1))
        {
            lastProxyItem++;
            sourceItemsIndex++;
        }

        // Add interval to result
        proxyIntervals << QPair<int, int>(firstProxyItem, lastProxyItem);
    }

    qStableSort(proxyIntervals.begin(), proxyIntervals.end());

    return proxyIntervals;
}

void KCategorizedSortFilterProxyModel::Private::removeProxyInterval(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, int proxyStart, int proxyEnd, const QModelIndex &proxyParent, Qt::Orientation orientation, bool emitSignal)
{
    int m_proxyStart = qMin(proxyStart, proxyEnd);
    int m_proxyEnd = qMax(proxyStart, proxyEnd);

    if (emitSignal)
    {
        if (orientation == Qt::Vertical)
        {
            emit p->beginRemoveRows(proxyParent, m_proxyStart, m_proxyEnd);
        }
        else
        {
            emit p->beginRemoveColumns(proxyParent, m_proxyStart, m_proxyEnd);
        }
    }

    proxyToSource.remove(m_proxyStart, m_proxyEnd - m_proxyStart + 1);

    buildSourceToProxyMapping(proxyToSource, sourceToProxy);

    if (emitSignal)
    {
        if (orientation == Qt::Vertical)
        {
            emit p->endRemoveRows();
        }
        else
        {
            emit p->endRemoveColumns();
        }
    }
}

void KCategorizedSortFilterProxyModel::Private::sourceItemsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end, Qt::Orientation orientation)
{
    if ((start < 0) || (end < 0))
    {
        return;
    }

    QMap<QModelIndex, InternalInformation*>::const_iterator it = sourceIndexMap.constFind(sourceParent);

    if (it == sourceIndexMap.constEnd())
    {
        // We do not have mapping for this index
        return;
    }

    InternalInformation *info = it.value();
    QVector<int> &sourceToProxy = (orientation == Qt::Vertical) ? info->proxyRows
                                                                : info->proxyColumns;
    QVector<int> &proxyToSource = (orientation == Qt::Vertical) ? info->sourceRows
                                                                : info->sourceColumns;

    QVector<int> sourceItemsToRemove;
    int proxyCount = proxyToSource.size();

    for (int proxyItem = 0; proxyItem < proxyCount; proxyItem++)
    {
        int sourceItem = proxyToSource.at(proxyItem);

        if ((sourceItem >= start) && (sourceItem <= end))
        {
            sourceItemsToRemove << sourceItem;
        }
    }

    removeSourceItems(sourceToProxy, proxyToSource, sourceItemsToRemove, sourceParent, orientation);
}

void KCategorizedSortFilterProxyModel::Private::sourceItemsRemoved(const QModelIndex &sourceParent, int start, int end, Qt::Orientation orientation)
{
    if ((start < 0) || (end < 0))
    {
        return;
    }

    QMap<QModelIndex, InternalInformation*>::const_iterator it = sourceIndexMap.constFind(sourceParent);

    if (it == sourceIndexMap.constEnd())
    {
        // We have no mapping for this index
        return;
    }

    InternalInformation *info = it.value();
    QVector<int> &sourceToProxy = (orientation == Qt::Vertical) ? info->proxyRows
                                                                : info->proxyColumns;
    QVector<int> &proxyToSource = (orientation == Qt::Vertical) ? info->sourceRows
                                                                : info->sourceColumns;

    emit p->layoutAboutToBeChanged();

    if (end >= sourceToProxy.size())
    {
        end = sourceToProxy.size() - 1;
    }

    // Shrink the source-to-proxy mapping to reflect the new item count
    int itemCount = end - start + 1;
    sourceToProxy.remove(start, itemCount);

    int proxyCount = proxyToSource.size();

    if (proxyCount > sourceToProxy.size())
    {
        // Mapping is in an inconsistent state, redo the whole mapping
        removeInternalInformation(sourceParent);

        return;
    }

    // Adjust "stale" indexes in proxy-to-source mapping
    for (int proxyItem = 0; proxyItem < proxyCount; proxyItem++)
    {
        int sourceItem = proxyToSource.at(proxyItem);

        if (sourceItem >= start)
        {
            proxyToSource.replace(proxyItem, sourceItem - itemCount);
        }
    }

    buildSourceToProxyMapping(proxyToSource, sourceToProxy);

    updateChildrenMapping(sourceParent, info, orientation, start, end, itemCount, true);

    emit p->layoutChanged();
}

void KCategorizedSortFilterProxyModel::Private::filterChanged()
{
    emit p->layoutAboutToBeChanged();

    QMap<QModelIndex, InternalInformation*>::const_iterator it;

    for (it = sourceIndexMap.constBegin(); it != sourceIndexMap.constEnd(); it++)
    {
        QModelIndex sourceParent = it.key();

        InternalInformation *info = it.value();
        handleFilterChanged(info->proxyRows, info->sourceRows, sourceParent, Qt::Vertical);
        handleFilterChanged(info->proxyColumns, info->sourceColumns, sourceParent, Qt::Horizontal);
    }

    emit p->layoutChanged();
}

void KCategorizedSortFilterProxyModel::Private::handleFilterChanged(QVector<int> &sourceToProxy, QVector<int> &proxyToSource, const QModelIndex &sourceParent, Qt::Orientation orientation)
{
    // What items we have to remove
    QVector<int> sourceItemsRemove;
    foreach (int sourceItem, proxyToSource)
    {
        if ((orientation == Qt::Vertical) ? !p->filterAcceptsRow(sourceItem, sourceParent)
                                          : !p->filterAcceptsColumn(sourceItem, sourceParent))
        {
            // This source item does not satisfy the filter, remove it
            sourceItemsRemove << sourceItem;
        }
    }

    // What non-mapped items we have to insert
    QVector<int> sourceItemsInsert;
    int sourceCount = sourceToProxy.size();
    for (int sourceItem = 0; sourceItem < sourceCount; sourceItem++)
    {
        if (sourceToProxy.at(sourceItem) == -1)
        {
            if ((orientation == Qt::Vertical) ? p->filterAcceptsRow(sourceItem, sourceParent)
                                              : p->filterAcceptsColumn(sourceItem, sourceParent))
            {
                // This source item satisfies the filter now, so it must be added
                sourceItemsInsert << sourceItem;
            }
        }
    }

    if (!sourceItemsRemove.isEmpty() || !sourceItemsInsert.isEmpty())
    {
        // Do item removal and insertion
        removeSourceItems(sourceToProxy, proxyToSource, sourceItemsRemove, sourceParent, orientation);

        if (orientation == Qt::Vertical)
        {
            sortSourceRows(sourceItemsInsert, sourceParent);
        }

        insertSourceItems(sourceToProxy, proxyToSource, sourceItemsInsert, sourceParent, orientation);
    }
}

QModelIndexList KCategorizedSortFilterProxyModel::Private::storePersistentIndexes()
{
    QModelIndexList sourceIndexes;
    int persistentCount = persistentIndexesData.count();

    for (int i = 0; i < persistentCount; i++)
    {
        QModelIndex proxyIndex = persistentIndexesData.at(i)->index;
        QModelIndex sourceIndex = proxyToSource(proxyIndex);

        sourceIndexes << sourceIndex;
    }

    return sourceIndexes;
}

void KCategorizedSortFilterProxyModel::Private::updatePersistentIndexes(const QModelIndexList &sourceIndexes)
{
    for (int i = 0; i < sourceIndexes.count(); i++)
    {
        QModelIndex sourceIndex = sourceIndexes.at(i);
        createInternalInformation(sourceIndex.parent());
        QModelIndex proxyIndex = sourceToProxy(sourceIndex);
        persistentIndexesData.at(i)->index = proxyIndex;
    }
}

QVector<QPair<int, QVector<int> > > KCategorizedSortFilterProxyModel::Private::proxyIntervalsForSourceItemsToAdd(const QVector<int> &proxyToSource, const QVector<int> &sourceItems, const QModelIndex &sourceParent, Qt::Orientation orientation) const
{
    QVector<QPair<int, QVector<int> > > proxyIntervals;

    if (sourceItems.isEmpty())
        return proxyIntervals;

    int proxyLow = 0;
    int proxyItem = 0;
    int sourceItemsIndex = 0;
    QVector<int> sourceItemsInInterval;
    bool compare = (orientation == Qt::Vertical && sortColumn >= 0);

    while (sourceItemsIndex < sourceItems.size())
    {
        sourceItemsInInterval.clear();
        int firstNewSourceItem = sourceItems.at(sourceItemsIndex);
        sourceItemsInInterval << firstNewSourceItem;
        sourceItemsIndex++;

        proxyLow = 0;
        int proxyHigh = proxyToSource.size() - 1;
        QModelIndex iMain = compare ? p->sourceModel()->index(firstNewSourceItem, sortColumn, sourceParent)
                                : QModelIndex();
        QModelIndex iCurrent;
        //a little trick to avoid checking sortOrder inside the loop
        QModelIndex& i1=iMain;
        QModelIndex& i2=iCurrent;
        if (sortOrder==Qt::DescendingOrder)
        {
            i1=iCurrent;
            i2=iMain;
        }

        // Start the binary search
        while (proxyLow <= proxyHigh)
        {
            proxyItem = (proxyLow + proxyHigh) / 2;

            if (compare)
            {
                iCurrent = p->sourceModel()->index(proxyToSource.at(proxyItem), sortColumn, sourceParent);

                    // The Big Trick (ereslibre)
                if (categorizedModel?(p->lessThanGeneralPurpose(i1, i2)||(  !p->lessThanGeneralPurpose(i2, i1)&&p->lessThanCategoryPurpose(i1, i2)  ))
                                    :(p->lessThan(i1, i2)))
                {
                    proxyHigh = proxyItem - 1;
                }
                else
                {
                    proxyLow = proxyItem + 1;
                }
            }
            else
            {
                if (firstNewSourceItem < proxyToSource.at(proxyItem))
                {
                    proxyHigh = proxyItem - 1;
                }
                else
                {
                    proxyLow = proxyItem + 1;
                }
            }
        }

        proxyItem = proxyLow;

        if (proxyItem >= proxyToSource.size())
        {
            for (; sourceItemsIndex < sourceItems.size(); sourceItemsIndex++)
            {
                sourceItemsInInterval << sourceItems.at(sourceItemsIndex);
            }
        }
        else
        {
            iMain = compare ? p->sourceModel()->index(proxyToSource.at(proxyItem), sortColumn, sourceParent)
                        : QModelIndex();

            for (; sourceItemsIndex < sourceItems.size(); sourceItemsIndex++)
            {
                int newSourceItem = sourceItems.at(sourceItemsIndex);

                if (compare)
                {
                    iCurrent = p->sourceModel()->index(newSourceItem, sortColumn, sourceParent);

                    // The Big Trick (ereslibre)
                    if (categorizedModel?(p->lessThanGeneralPurpose(i1, i2)||(  !p->lessThanGeneralPurpose(i2, i1)&&p->lessThanCategoryPurpose(i1, i2)  ))
                                        :(p->lessThan(i1, i2)))
                    {
                        break;
                    }
                }
                else
                {
                    if (proxyToSource.at(proxyItem) < newSourceItem)
                    {
                        break;
                    }
                }

                sourceItemsInInterval << newSourceItem;
            }
        }

        proxyIntervals << QPair<int, QVector<int> >(proxyItem, sourceItemsInInterval);
    }

    return proxyIntervals;
}

QMap<QModelIndex, KCategorizedSortFilterProxyModel::Private::InternalInformation*>::const_iterator KCategorizedSortFilterProxyModel::Private::indexToIterator(const QModelIndex &proxyIndex) const
{
    const void *p = proxyIndex.internalPointer();
    QMap<QModelIndex, InternalInformation*>::const_iterator it = static_cast<const InternalInformation*>(p)->mapIterator;

    return it;
}

QModelIndex KCategorizedSortFilterProxyModel::Private::sourceToProxy(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
    {
        return QModelIndex();
    }

    QModelIndex sourceParent = sourceIndex.parent();
    QMap<QModelIndex, InternalInformation*>::const_iterator it = createInternalInformation(sourceParent);
    InternalInformation *info = it.value();

    if ((sourceIndex.row() >= info->proxyRows.size()) || (sourceIndex.column() >= info->proxyColumns.size()))
    {
        return QModelIndex();
    }

    int proxyRow = info->proxyRows.at(sourceIndex.row());
    int proxyColumn = info->proxyColumns.at(sourceIndex.column());

    if (proxyRow == -1 || proxyColumn == -1)
    {
        return QModelIndex();
    }

    return p->createIndex(proxyRow, proxyColumn, *it);
}

QModelIndex KCategorizedSortFilterProxyModel::Private::proxyToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
    {
        return QModelIndex();
    }

    QMap<QModelIndex, InternalInformation*>::const_iterator it = indexToIterator(proxyIndex);
    InternalInformation *info = it.value();

    if ((proxyIndex.row() >= info->sourceRows.size()) || (proxyIndex.column() >= info->sourceColumns.size()))
    {
        return QModelIndex();
    }

    int sourceRow = info->sourceRows.at(proxyIndex.row());
    int sourceCol = info->sourceColumns.at(proxyIndex.column());

    return p->sourceModel()->index(sourceRow, sourceCol, it.key());
}

bool KCategorizedSortFilterProxyModel::Private::indexValid(const QModelIndex &index) const
{
    return ((index.row() >= 0) && (index.column() >= 0) && (index.model() == p));
}

#include "kcategorizedsortfilterproxymodel.moc"
#include "kcategorizedsortfilterproxymodel_p.moc"
