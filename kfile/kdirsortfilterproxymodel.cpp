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
#include <kdebug.h>

// TODO KDE 4.1: bring Nepomuk stuff from Dolphin to kdelibs/nepomuk
// in the form of a separate subclass

KDirSortFilterProxyModel::KDirSortFilterProxyModel(QObject* parent)
    : KCategorizedSortFilterProxyModel(parent), d(0)
{
    setDynamicSortFilter(true);

    // sort by the user visible string for now
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(KDirModel::Name, Qt::AscendingOrder);
}

KDirSortFilterProxyModel::~KDirSortFilterProxyModel()
{
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

int KDirSortFilterProxyModel::naturalCompare(const QString& a,
                                                const QString& b)
{
    // This method chops the input a and b into pieces of
    // digits and non-digits (a1.05 becomes a | 1 | . | 05)
    // and compares these pieces of a and b to each other
    // (first with first, second with second, ...).
    //
    // This is based on the natural sort order code code by Martin Pool
    // http://sourcefrog.net/projects/natsort/
    // Martin Pool agreed to license this under LGPL or GPL.

    const QChar* currA = a.unicode(); // iterator over a
    const QChar* currB = b.unicode(); // iterator over b

    if (currA == currB) {
        return 0;
    }

    const QChar* begSeqA = currA; // beginning of a new character sequence of a
    const QChar* begSeqB = currB;

    while (!currA->isNull() && !currB->isNull()) {
        // find sequence of characters ending at the first non-character
        while (!currA->isNull() && !currA->isDigit()) {
            ++currA;
        }

        while (!currB->isNull() && !currB->isDigit()) {
            ++currB;
        }

        // compare these sequences
        const QString subA(begSeqA, currA - begSeqA);
        const QString subB(begSeqB, currB - begSeqB);
        const int cmp = QString::localeAwareCompare(subA, subB);
        if (cmp != 0) {
            return cmp;
        }

        if (currA->isNull() || currB->isNull()) {
            break;
        }

        // now some digits follow...
        if ((*currA == '0') || (*currB == '0')) {
            // one digit-sequence starts with 0 -> assume we are in a fraction part
            // do left aligned comparison (numbers are considered left aligned)
            while (1) {
                if (!currA->isDigit() && !currB->isDigit()) {
                    break;
                } else if (!currA->isDigit()) {
                    return -1;
                } else if (!currB->isDigit()) {
                    return + 1;
                } else if (*currA < *currB) {
                    return -1;
                } else if (*currA > *currB) {
                    return + 1;
                }
                ++currA;
                ++currB;
            }
        } else {
            // No digit-sequence starts with 0 -> assume we are looking at some integer
            // do right aligned comparison.
            //
            // The longest run of digits wins. That aside, the greatest
            // value wins, but we can't know that it will until we've scanned
            // both numbers to know that they have the same magnitude.

            int weight = 0;
            while (1) {
                if (!currA->isDigit() && !currB->isDigit()) {
                    if (weight != 0) {
                        return weight;
                    }
                    break;
                } else if (!currA->isDigit()) {
                    return -1;
                } else if (!currB->isDigit()) {
                    return + 1;
                } else if ((*currA < *currB) && (weight == 0)) {
                    weight = -1;
                } else if ((*currA > *currB) && (weight == 0)) {
                    weight = + 1;
                }
                ++currA;
                ++currB;
            }
        }

        begSeqA = currA;
        begSeqB = currB;
    }

    if (currA->isNull() && currB->isNull()) {
        return 0;
    }

    return currA->isNull() ? -1 : + 1;
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

bool KDirSortFilterProxyModel::subsortLessThan(const QModelIndex& left,
                                               const QModelIndex& right) const
{
    KDirModel* dirModel = static_cast<KDirModel*>(sourceModel());

    const KFileItem leftFileItem  = dirModel->itemForIndex(left);
    const KFileItem rightFileItem = dirModel->itemForIndex(right);

    // On our priority, folders go above regular files.
    if (leftFileItem.isDir() && !rightFileItem.isDir()) {
        return true;
    } else if (!leftFileItem.isDir() && rightFileItem.isDir()) {
        return false;
    }

    // Hidden elements go before visible ones, if they both are
    // folders or files.
    if (leftFileItem.isHidden() && !rightFileItem.isHidden()) {
        return true;
    } else if (!leftFileItem.isHidden() && rightFileItem.isHidden()) {
        return false;
    }

    switch (left.column()) {
    case KDirModel::Name: {
        // So we are in the same priority, what counts now is their names.
        const QVariant leftData  = dirModel->data(left, KDirModel::Name);
        const QVariant rightData = dirModel->data(right, KDirModel::Name);
        const QString leftValueString(leftData.toString());
        const QString rightValueString(rightData.toString());

        return sortCaseSensitivity() ?
               (naturalCompare(leftValueString, rightValueString) < 0) :
               (naturalCompare(leftValueString.toLower(), rightValueString.toLower()) < 0);
    }

    case KDirModel::Size: {
        // If we have two folders, what we have to measure is the number of
        // items that contains each other
        if (leftFileItem.isDir() && rightFileItem.isDir()) {
            QVariant leftValue = dirModel->data(left, KDirModel::ChildCountRole);
            int leftCount = leftValue.type() == QVariant::Int ? leftValue.toInt() : KDirModel::ChildCountUnknown;

            QVariant rightValue = dirModel->data(right, KDirModel::ChildCountRole);
            int rightCount = rightValue.type() == QVariant::Int ? rightValue.toInt() : KDirModel::ChildCountUnknown;

            // In the case they two have the same child items, we sort them by
            // their names. So we have always everything ordered. We also check
            // if we are taking in count their cases.
            if (leftCount == rightCount) {
                return sortCaseSensitivity() ? (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                        (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
            }

            // If they had different number of items, we sort them depending
            // on how many items had each other.
            return leftCount < rightCount;
        }

        // If what we are measuring is two files and they have the same size,
        // sort them by their file names.
        if (leftFileItem.size() == rightFileItem.size()) {
            return sortCaseSensitivity() ? (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                    (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        // If their sizes are different, sort them by their sizes, as expected.
        return leftFileItem.size() < rightFileItem.size();
    }

    case KDirModel::ModifiedTime: {
        KDateTime leftTime = leftFileItem.time(KFileItem::ModificationTime);
        KDateTime rightTime = rightFileItem.time(KFileItem::ModificationTime);

        if (leftTime == rightTime) {
            return sortCaseSensitivity() ?
                   (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                   (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        return leftTime > rightTime;
    }

    case KDirModel::Permissions: {
        QFileInfo leftFileInfo(leftFileItem.url().pathOrUrl());
        QFileInfo rightFileInfo(rightFileItem.url().pathOrUrl());

        int leftPermissionsPoints = pointsForPermissions(leftFileInfo);
        int rightPermissionsPoints = pointsForPermissions(rightFileInfo);

        if (leftPermissionsPoints == rightPermissionsPoints) {
            return sortCaseSensitivity() ?
                   (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                   (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        return leftPermissionsPoints > rightPermissionsPoints;
    }

    case KDirModel::Owner: {
        if (leftFileItem.user() == rightFileItem.user()) {
            return sortCaseSensitivity() ?
                   (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                   (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        return naturalCompare(leftFileItem.user(), rightFileItem.user()) < 0;
    }

    case KDirModel::Group: {
        if (leftFileItem.group() == rightFileItem.group()) {
            return sortCaseSensitivity() ? (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                    (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        return naturalCompare(leftFileItem.group(),
                              rightFileItem.group()) < 0;
    }

    case KDirModel::Type: {
        if (leftFileItem.mimetype() == rightFileItem.mimetype()) {
            return sortCaseSensitivity() ?
                   (naturalCompare(leftFileItem.name(), rightFileItem.name()) < 0) :
                   (naturalCompare(leftFileItem.name().toLower(), rightFileItem.name().toLower()) < 0);
        }

        return naturalCompare(leftFileItem.mimeComment(),
                              rightFileItem.mimeComment()) < 0;
    }

    }

    // We have set a SortRole and trust the ProxyModel to do
    // the right thing for now.
    return KCategorizedSortFilterProxyModel::subsortLessThan(left, right);
}

int KDirSortFilterProxyModel::compareCategories(const QModelIndex &left,
                                                const QModelIndex &right) const
{
    KDirModel* dirModel = static_cast<KDirModel*>(sourceModel());

    const KFileItem leftFileItem  = dirModel->itemForIndex(left);
    const KFileItem rightFileItem = dirModel->itemForIndex(right);

    switch (left.column()) {
    case KDirModel::Name: {
        bool leftFileNameStartsByLetter = false;
        const QString str(leftFileItem.name().toUpper());
        const QChar* currA = str.unicode();
        while (!currA->isNull() && !leftFileNameStartsByLetter) {
            if (currA->isLetter())
                leftFileNameStartsByLetter = true;
            else if (currA->isDigit()) {
                break;
            } else
                ++currA;
        }

        bool rightFileNameStartsByLetter = false;
        const QString strb(rightFileItem.name().toUpper());
        const QChar *currB = strb.unicode();
        while (!currB->isNull() && !rightFileNameStartsByLetter) {
            if (currB->isLetter())
                rightFileNameStartsByLetter = true;
            else if (currB->isDigit()) {
                break;
            } else
                ++currB;
        }

        if (!rightFileNameStartsByLetter)
            return -1;

        if (!leftFileNameStartsByLetter && rightFileNameStartsByLetter)
            return 1;

        return naturalCompare(*currA, *currB);
    }

    case KDirModel::Size: {
        // If we are sorting by size, show folders first. We will sort them
        // correctly later.
        if (leftFileItem.isDir() && !rightFileItem.isDir()) {
            return -1;
        }

        if (!leftFileItem.isDir() && !rightFileItem.isDir()) {
            return rightFileItem.size() - leftFileItem.size();
        }

        return 1;
    }

    case KDirModel::ModifiedTime: {
        KDateTime leftTime = leftFileItem.time(KFileItem::ModificationTime);
        KDateTime rightTime = rightFileItem.time(KFileItem::ModificationTime);
	if(leftTime == rightTime) return 0;
	if(leftTime > rightTime) return 1;
	return -1;
    }

    case KDirModel::Permissions: {
        QFileInfo leftFileInfo(leftFileItem.url().pathOrUrl());
        QFileInfo rightFileInfo(rightFileItem.url().pathOrUrl());

        int leftPermissionsPoints = pointsForPermissions(leftFileInfo);
        int rightPermissionsPoints = pointsForPermissions(rightFileInfo);

        return leftPermissionsPoints - rightPermissionsPoints;
    }

    case KDirModel::Owner: {
        return naturalCompare(leftFileItem.user().toLower(),
                              rightFileItem.user().toLower());
    }

    case KDirModel::Group: {
        return naturalCompare(leftFileItem.group().toLower(),
                              rightFileItem.group().toLower());
    }

    case KDirModel::Type: {
        // If we are sorting by size, show folders first. We will sort them
        // correctly later.
        if (leftFileItem.isDir() && !rightFileItem.isDir()) {
            return -1;
        } else if (!leftFileItem.isDir() && rightFileItem.isDir()) {
            return 1;
        }

        return naturalCompare(leftFileItem.mimeComment().toLower(),
                              rightFileItem.mimeComment().toLower());
    }

    default:
        break;
    }
    return 0;
}

#include "kdirsortfilterproxymodel.moc"
