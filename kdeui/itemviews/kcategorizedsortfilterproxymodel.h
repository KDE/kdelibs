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

public:
    enum AdditionalRoles {
        // Note: use printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        CategoryRole = 0x17CE990A
    };

    KCategorizedSortFilterProxyModel(QObject *parent = 0);
    virtual ~KCategorizedSortFilterProxyModel();

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    bool isCategorizedModel() const;
    void setCategorizedModel(bool categorizedModel);
    int sortColumn() const;
    Qt::SortOrder sortOrder() const;

protected:
    /**
      * Overridden from QSortFilterProxyModel. This calls compareCategories to sort by category
      * then calls subsortLessThan to then subsort.
      */
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    /**
      * For two indexes that are known to be in the same category, return true if @p left should
      * be sorted before @p right.
      */
    virtual bool subsortLessThan(const QModelIndex &left, const QModelIndex &right) const;
    /**
      * For two indexes, return -1 if @p left comes in a category before @p right,
      * 0 if they are in the same category, and 1 if left comes in a category after right
      */
    virtual int compareCategories(const QModelIndex &left, const QModelIndex &right) const;

private:
    class Private;
    Private* const d;
};


#endif // KCATEGORIZEDSORTFILTERPROXYMODEL_H
