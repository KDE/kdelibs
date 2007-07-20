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

#include <QSortFilterProxyModel>
#include <kfile_export.h>
//#include <kio/kio_export.h>

/**
 * @brief Acts as proxy model for KDirModel to sort and filter
 *        KFileItems.
 *
 * A natural sorting is done. This means that items like:
 * - item_10.png
 * - item_1.png
 * - item_2.png
 * are sorted like
 * - item_1.png
 * - item_2.png
 * - item_10.png
 *
 * It is assured that directories are always sorted before files.
 *
 * Dont use it with non-KDirModel derivatives -- contract-based programming here :)
 *
 * TODO KDE 4.1: bring nepomuk stuff from dolphin to kdelibs/nepomuk
 * in the form of separate subclass
 *
 * @author Dominic Battre, Martin Pool and Peter Penz
 */
class KFILE_EXPORT /*KIO_EXPORT*/ KDirSortFilterProxyModel : public QSortFilterProxyModel
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


protected:
    /**
     * Reimplemented from QAbstractItemModel to use naturalCompare.
     */
    virtual bool lessThan(const QModelIndex& left,
                          const QModelIndex& right) const;

public:
    /**
     * Made public for the case when this nice algorithm may be needed somewhere else.
     */
    static int naturalCompare(const QString& a, const QString& b);

};


#endif
