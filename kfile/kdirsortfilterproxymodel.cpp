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

#include "kdirsortfilterproxymodel.h"

#include <kdirmodel.h>
#include <kfileitem.h>
#include <kdatetime.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kdebug.h>

// TODO KDE 4.1: bring Nepomuk stuff from Dolphin to kdelibs/nepomuk
// in the form of a separate subclass

class KDirSortFilterProxyModel::KDirSortFilterProxyModelPrivate
{
public:
    KDirSortFilterProxyModelPrivate() : m_sortFoldersFirst(true) {}

    bool m_sortFoldersFirst;
};

KDirSortFilterProxyModel::KDirSortFilterProxyModel(QObject* parent)
    : KCategorizedSortFilterProxyModel(parent), d(new KDirSortFilterProxyModelPrivate)
{
    setDynamicSortFilter(true);

    // sort by the user visible string for now
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(KDirModel::Name, Qt::AscendingOrder);

    setSupportedDragActions(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction | Qt::IgnoreAction);
}

KDirSortFilterProxyModel::~KDirSortFilterProxyModel()
{
    delete d;
}

bool KDirSortFilterProxyModel::hasChildren(const QModelIndex& parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return sourceModel()->hasChildren(sourceParent);
}

bool KDirSortFilterProxyModel::canFetchMore(const QModelIndex& parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return sourceModel()->canFetchMore(sourceParent);
}

int KDirSortFilterProxyModel::pointsForPermissions(const QFileInfo &info)
{
    int points = 0;

    QFile::Permission permissionsCheck[] = { QFile::ReadUser,
                                             QFile::WriteUser,
                                             QFile::ExeUser,
                                             QFile::ReadGroup,
                                             QFile::WriteGroup,
                                             QFile::ExeGroup,
                                             QFile::ReadOther,
                                             QFile::WriteOther,
                                             QFile::ExeOther };

    for (int i = 0; i < 9; i++) {
        points += info.permission(permissionsCheck[i]) ? 1 : 0;
    }

    return points;
}

void KDirSortFilterProxyModel::setSortFoldersFirst(bool foldersFirst)
{
    d->m_sortFoldersFirst = foldersFirst;
}

bool KDirSortFilterProxyModel::sortFoldersFirst() const
{
    return d->m_sortFoldersFirst;
}

bool KDirSortFilterProxyModel::subSortLessThan(const QModelIndex& left,
                                               const QModelIndex& right) const
{
    KDirModel* dirModel = static_cast<KDirModel*>(sourceModel());

    const KFileItem leftFileItem  = dirModel->itemForIndex(left);
    const KFileItem rightFileItem = dirModel->itemForIndex(right);

    const bool isLessThan = (sortOrder() == Qt::AscendingOrder);

    // Folders go before files if the corresponding setting is set.
    if (d->m_sortFoldersFirst) {
        if (leftFileItem.isDir() && !rightFileItem.isDir()) {
            return isLessThan;
        } else if (!leftFileItem.isDir() && rightFileItem.isDir()) {
            return !isLessThan;
        }
    }


    // Hidden elements go before visible ones.
    if (leftFileItem.isHidden() && !rightFileItem.isHidden()) {
        return isLessThan;
    } else if (!leftFileItem.isHidden() && rightFileItem.isHidden()) {
        return !isLessThan;
    }

    switch (left.column()) {
    case KDirModel::Name: {
        return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
    }

    case KDirModel::Size: {
        // If we have two folders, what we have to measure is the number of
        // items that contains each other
        if (leftFileItem.isDir() && rightFileItem.isDir()) {
            QVariant leftValue = dirModel->data(left, KDirModel::ChildCountRole);
            int leftCount = (leftValue.type() == QVariant::Int) ? leftValue.toInt() : KDirModel::ChildCountUnknown;

            QVariant rightValue = dirModel->data(right, KDirModel::ChildCountRole);
            int rightCount = (rightValue.type() == QVariant::Int) ? rightValue.toInt() : KDirModel::ChildCountUnknown;

            // In the case they two have the same child items, we sort them by
            // their names. So we have always everything ordered. We also check
            // if we are taking in count their cases.
            if (leftCount == rightCount) {
                return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
            }

            // If one of them has unknown child items, place them on the end. If we
            // were comparing two unknown childed items, the previous comparation
            // sorted them by naturalCompare between them. This case is when we
            // have an unknown childed item, and another known.
            if (leftCount == KDirModel::ChildCountUnknown) {
                return false;
            }

            if (rightCount == KDirModel::ChildCountUnknown) {
                return true;
            }

            // If they had different number of items, we sort them depending
            // on how many items had each other.
            return leftCount < rightCount;
        }

        // If what we are measuring is two files and they have the same size,
        // sort them by their file names.
        if (leftFileItem.size() == rightFileItem.size()) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        // If their sizes are different, sort them by their sizes, as expected.
        return leftFileItem.size() < rightFileItem.size();
    }

    case KDirModel::ModifiedTime: {
        KDateTime leftModifiedTime = leftFileItem.time(KFileItem::ModificationTime).toLocalZone();
        KDateTime rightModifiedTime = rightFileItem.time(KFileItem::ModificationTime).toLocalZone();

        if (leftModifiedTime == rightModifiedTime) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        return leftModifiedTime < rightModifiedTime;
    }

    case KDirModel::Permissions: {
        // ### You can't use QFileInfo on urls!! Use the KFileItem instead.
        QFileInfo leftFileInfo(leftFileItem.url().pathOrUrl());
        QFileInfo rightFileInfo(rightFileItem.url().pathOrUrl());

        int leftPermissionsPoints = pointsForPermissions(leftFileInfo);
        int rightPermissionsPoints = pointsForPermissions(rightFileInfo);

        if (leftPermissionsPoints == rightPermissionsPoints) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        return leftPermissionsPoints > rightPermissionsPoints;
    }

    case KDirModel::Owner: {
        if (leftFileItem.user() == rightFileItem.user()) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        return KStringHandler::naturalCompare(leftFileItem.user(), rightFileItem.user()) < 0;
    }

    case KDirModel::Group: {
        if (leftFileItem.group() == rightFileItem.group()) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        return KStringHandler::naturalCompare(leftFileItem.group(), rightFileItem.group()) < 0;
    }

    case KDirModel::Type: {
        if (leftFileItem.mimetype() == rightFileItem.mimetype()) {
            return KStringHandler::naturalCompare(leftFileItem.name(), rightFileItem.name(), sortCaseSensitivity()) < 0;
        }

        return KStringHandler::naturalCompare(leftFileItem.mimeComment(), rightFileItem.mimeComment()) < 0;
    }

    }

    // We have set a SortRole and trust the ProxyModel to do
    // the right thing for now.
    return KCategorizedSortFilterProxyModel::subSortLessThan(left, right);
}
