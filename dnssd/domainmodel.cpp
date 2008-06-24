/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Jakub Stachowski <qbast@go2.pl>
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

#include "domainmodel.h"
#include "domainbrowser.h"
#include <qstringlist.h>

namespace DNSSD
{

struct DomainModelPrivate
{
        DomainBrowser* m_browser;
};


DomainModel::DomainModel(DomainBrowser* browser, QObject* parent) 
    :  QAbstractItemModel(parent), d(new DomainModelPrivate)
{
    d->m_browser=browser;
    browser->setParent(this);
    connect(browser, SIGNAL(domainAdded(const QString&)), this,
	SIGNAL(layoutChanged()));
    connect(browser, SIGNAL(domainRemoved(const QString&)), this,
	SIGNAL(layoutChanged()));
    browser->startBrowse();
}

DomainModel::~DomainModel()
{
    delete d;
}

int DomainModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}
int DomainModel::rowCount(const QModelIndex& parent ) const
{
    return (parent.isValid()) ? 0 : d->m_browser->domains().size();
}

QModelIndex DomainModel::parent(const QModelIndex& index ) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QModelIndex DomainModel::index(int row, int column, const QModelIndex& parent  ) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

bool DomainModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) return false;
    if (column!=0) return false;
    if (row<0 || row>=rowCount(parent)) return false;
    return true;
}

QVariant DomainModel::data(const QModelIndex& index, int role  ) const
{
    if (!index.isValid()) return QVariant();
    if (!hasIndex(index.row(), index.column(), index.parent())) return QVariant();
    const QStringList domains=d->m_browser->domains();
    if (role==Qt::DisplayRole) return domains[index.row()];
    return QVariant();
}

}
