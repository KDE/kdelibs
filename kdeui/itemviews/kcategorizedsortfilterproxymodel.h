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

#ifndef KCATEGORIZEDSORTFILTERPROXYMODEL_H
#define KCATEGORIZEDSORTFILTERPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <kdeui_export.h>

class QItemSelection;


/**
  * This class replaces the original QSortFilterProxyModel for the
  * KCategorizedView class usage.
  *
  * @author Rafael Fernández López
  */
class KDEUI_EXPORT KCategorizedSortFilterProxyModel
    : public QSortFilterProxyModel
{
    Q_OBJECT

    friend class LessThan;

public:
    enum AdditionalRoles {
        // Note: use printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        CategoryRole = 0x17CE990A
    };

    KCategorizedSortFilterProxyModel(QObject *parent = 0);
    virtual ~KCategorizedSortFilterProxyModel();

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QItemSelection mapSelectionFromSource(const QItemSelection &sourceSelection) const;
    virtual QItemSelection mapSelectionToSource(const QItemSelection &proxySelection) const;

    virtual QRegExp filterRegExp() const;
    virtual void setFilterRegExp(const QRegExp &regExp);
    virtual void setFilterRegExp(const QString &pattern);

    virtual int filterKeyColumn() const;
    virtual void setFilterKeyColumn(int column);

    virtual Qt::CaseSensitivity filterCaseSensitivity() const;
    virtual void setFilterCaseSensitivity(Qt::CaseSensitivity cs);

    virtual Qt::CaseSensitivity sortCaseSensitivity() const;
    virtual void setSortCaseSensitivity(Qt::CaseSensitivity cs);

    virtual bool isSortLocaleAware() const;
    virtual void setSortLocaleAware(bool on);

    virtual bool dynamicSortFilter() const;
    virtual void setDynamicSortFilter(bool enable);

    virtual int sortRole() const;
    virtual void setSortRole(int role);

    virtual int filterRole() const;
    virtual void setFilterRole(int role);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::EditRole) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    virtual QModelIndex buddy(const QModelIndex &index) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QSize span(const QModelIndex &index) const;

    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::DropActions supportedDropActions() const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual void fetchMore(const QModelIndex &parent);
    virtual bool canFetchMore(const QModelIndex &parent) const;

    int sortColumn() const;
    Qt::SortOrder sortOrder() const;

    bool isCategorizedModel() const;
    void setCategorizedModel(bool categorizedModel);

public Q_SLOTS:
    void setFilterWildcard(const QString &pattern);
    void setFilterFixedString(const QString &pattern);
    void clear();
    void invalidate();

protected:
    virtual bool lessThanGeneralPurpose(const QModelIndex &left, const QModelIndex &right) const = 0;
    virtual bool lessThanCategoryPurpose(const QModelIndex &left, const QModelIndex &right) const = 0;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;

private:
    class Private;
    Private* const d;
};


#endif // KCATEGORIZEDSORTFILTERPROXYMODEL_H
