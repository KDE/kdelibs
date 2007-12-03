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
  * This class lets you categorize a view. It is meant to be used along with
  * KCategorizedView class.
  *
  * In general terms all you need to do is to reimplement subSortLessThan() and
  * compareCategories() methods. In order to make categorization work, you need
  * to also call setCategorizedModel() class to enable it, since the categorization
  * is disabled by default.
  *
  * @see KCategorizedView
  *
  * @author Rafael Fernández López <ereslibre@kde.org>
  */
class KDEUI_EXPORT KCategorizedSortFilterProxyModel
    : public QSortFilterProxyModel
{
public:
    enum AdditionalRoles {
        // Note: use printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        CategoryRole = 0x17CE990A ///< This role is used for asking the category to a given index
    };

    KCategorizedSortFilterProxyModel(QObject *parent = 0);
    virtual ~KCategorizedSortFilterProxyModel();

    /**
      * Overridden from QSortFilterProxyModel. Sorts the source model using
      * @p column for the given @p order.
      */
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    /**
      * @return whether the model is categorized or not.
      */
    bool isCategorizedModel() const;

    /**
      * Enables or disables the categorization feature.
      *
      * @param categorizedModel whether to enable or disable the categorization feature.
      */
    void setCategorizedModel(bool categorizedModel);

    /**
      * @return the column being used for sorting.
      */
    int sortColumn() const;

    /**
      * @return the sort order being used for sorting.
      */
    Qt::SortOrder sortOrder() const;

protected:
    /**
      * Overridden from QSortFilterProxyModel. If you are subclassing
      * KCategorizedSortFilterProxyModel, you will probably not need to reimplement this
      * method. 
      *
      * It calls compareCategories() to sort by category.  If the both items are in the
      * same category (i.e. compareCategories returns 0), then subSortLessThan is called.
      *
      * @return Returns true if the item @p left is less than the item @p right when sorting.
      *
      * @warning You usually won't need to reimplement this method when subclassing
      *          from KCategorizedSortFilterProxyModel.
      */
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    /**
      * This method has a similar purpose as lessThan() has on QSortFilterProxyModel.
      * It is used for sorting items that in the same category.
      *
      * @return Returns true if the item @p left is less than the item @p right when sorting.
      */
    virtual bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const;

    /**
      * @return A negative value if the category of @p left should be placed before the
      *         category of @p right. 0 if @p left and @p right are on the same category, and
      *         a positive value if the category of @p left should be placed after the
      *         category of @p right.
      */
    virtual int compareCategories(const QModelIndex &left, const QModelIndex &right) const = 0;

private:
    class Private;
    Private *const d;
};


#endif // KCATEGORIZEDSORTFILTERPROXYMODEL_H
