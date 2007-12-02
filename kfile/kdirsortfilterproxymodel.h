/*
   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>
   Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
   Copyright (C) 2006 by Martin Pool <mbp@canonical.com>

   Separated from Dolphin by Nick Shaforostoff <shafff@ukr.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDIRSORTFILTERPROXYMODEL_H
#define KDIRSORTFILTERPROXYMODEL_H

#include <QtCore/QFileInfo>

#include <kcategorizedsortfilterproxymodel.h>

#include <kfile_export.h>

/**
 * @brief Acts as proxy model for KDirModel to sort and filter
 *        KFileItems.
 *
 * A natural sorting is done. This means that items like:
 * - item_10.png
 * - item_1.png
 * - item_2.png
 *
 * are sorted like
 * - item_1.png
 * - item_2.png
 * - item_10.png
 *
 * It is assured that directories are always sorted before files.
 * Don't use it with non-KDirModel derivatives.
 *
 * @author Dominic Battre, Martin Pool and Peter Penz
 */
class KFILE_EXPORT KDirSortFilterProxyModel
    : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:
    KDirSortFilterProxyModel(QObject* parent = 0);
    virtual ~KDirSortFilterProxyModel();

    /** Reimplemented from QAbstractItemModel. Returns true for directories. */
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

    /**
     * Reimplemented from QAbstractItemModel.
     * Returns true for 'empty' directories so they can be populated later.
     */
    virtual bool canFetchMore(const QModelIndex& parent) const;

    /**
     * Does a natural comparing of the strings. -1 is returned if \a a
     * is smaller than \a b. +1 is returned if \a a is greater than \a b. 0
     * is returned if both values are equal.
     */
    static int naturalCompare(const QString& a, const QString& b);

    /**
     * Returns the permissions in "points". This is useful for sorting by
     * permissions.
     */
    static int pointsForPermissions(const QFileInfo &info);

protected:
    /**
     * Reimplemented from QAbstractItemModel to use naturalCompare.
     */
    virtual bool subsortLessThan(const QModelIndex& left,
                                 const QModelIndex& right) const;

    virtual int compareCategories(const QModelIndex &left, const QModelIndex &right) const;
private:
    class KDirSortFilterProxyModelPrivate;
    KDirSortFilterProxyModelPrivate* const d;
};


#endif
