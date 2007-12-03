/**
  * This file is part of the KDE project
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
  * Copyright (C) 2007 John Tapsell <tapsell@kde.org>
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
}

KCategorizedSortFilterProxyModel::~KCategorizedSortFilterProxyModel()
{
    delete d;
}

void KCategorizedSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    d->sortColumn = column;
    d->sortOrder = order;

    QSortFilterProxyModel::sort(column, order);
}

bool KCategorizedSortFilterProxyModel::lessThan (const QModelIndex & left, const QModelIndex & right) const
{
    if (d->categorizedModel)
    {
        int compare = compareCategories(left, right);

        if (compare > 0) // left is greater than right
        {
            return false;
        }
        else if (compare < 0) // left is less than right
        {
            return true;
        }
    }

    return subsortLessThan(left, right);
}

bool KCategorizedSortFilterProxyModel::subsortLessThan (const QModelIndex & left, const QModelIndex & right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}

int KCategorizedSortFilterProxyModel::compareCategories(const QModelIndex &left, const QModelIndex &right) const
{
    return 0;
}

bool KCategorizedSortFilterProxyModel::isCategorizedModel() const
{
    return d->categorizedModel;
}

void KCategorizedSortFilterProxyModel::setCategorizedModel(bool categorizedModel)
{
    if (categorizedModel == d->categorizedModel)
    {
        return;
    }

    d->categorizedModel = categorizedModel;
    sort(d->sortColumn, d->sortOrder); // items need to be sorted again
}

int KCategorizedSortFilterProxyModel::sortColumn() const
{
    return d->sortColumn;
}

Qt::SortOrder KCategorizedSortFilterProxyModel::sortOrder() const
{
    return d->sortOrder;
}


#include "kcategorizedsortfilterproxymodel.moc"
