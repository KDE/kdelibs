/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2009 Rafael Fernández López <ereslibre@kde.org>
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

#ifndef KCATEGORIZEDVIEW_P_H
#define KCATEGORIZEDVIEW_P_H

class KCategorizedSortFilterProxyModel;
class KCategoryDrawer;

/**
  * @internal
  */
class KCategorizedView::Private
{
public:
    struct Block;
    struct Item;

    Private(KCategorizedView *q);
    ~Private();

    bool isCategorized() const;

    /**
      * Returns the first and last element that intersects with rect.
      *
      * @note see that here we cannot take out items between first and last (as we could
      *       do with the rubberband).
      *
      * Complexity: O(log(n)) where n is model()->rowCount().
      */
    QPair<QModelIndex, QModelIndex> intersectingIndexesWithRect(const QRect &rect) const;

    /**
      * Returns the position of the block of @p category.
      *
      * Complexity: O(n) where n is the number of different categories when the block has been
      *             marked as in quarantine. O(1) the rest of the times (the vast majority).
      */
    QPoint blockPosition(const QString &category);

    /**
      * Returns the height of the block determined by @p category.
      */
    int blockHeight(const QString &category);

    /**
      * Returns the actual viewport width.
      */
    int viewportWidth() const;

    /**
      * Marks all elements as in quarantine.
      *
      * Complexity: O(n) where n is model()->rowCount().
      *
      * @warning this is an expensive operation
      */
    void regenerateAllElements();

    /**
      * Update internal information, and keep sync with the real information that the model contains.
      */
    void rowsInserted(const QModelIndex &parent, int start, int end);

    /**
      * Returns @p rect in viewport terms, taking in count horizontal and vertical offsets.
      */
    QRect mapToViewport(const QRect &rect) const;

    /**
      * Returns @p rect in absolute terms, converted from viewport position.
      */
    QRect mapFromViewport(const QRect &rect) const;

    /**
      * Returns the height of the highest element in last row. This is only applicable if there is
      * no grid set and uniformItemSizes is false.
      *
      * @param block in which block are we searching. Necessary to stop the search if we hit the
      *              first item in this block.
      */
    int highestElementInLastRow(const Block &block) const;

    /**
      * Returns whether the view has a valid grid size.
      */
    bool hasGrid() const;

    /**
      * Returns the category for the given index.
      */
    QString categoryForIndex(const QModelIndex &index) const;

    /**
      * Updates the visual rect for item when flow is LeftToRight.
      */
    void leftToRightVisualRect(const QModelIndex &index, Item &item,
                               const Block &block, const QPoint &blockPos) const;

    /**
      * Updates the visual rect for item when flow is TopToBottom.
      * @note we only support viewMode == ListMode in this case.
      */
    void topToBottomVisualRect(const QModelIndex &index, Item &item,
                               const Block &block, const QPoint &blockPos) const;

    void _k_slotCollapseOrExpandClicked();

    KCategorizedView *q;
    KCategorizedSortFilterProxyModel *proxyModel;
    KCategoryDrawer *categoryDrawer;
    KCategoryDrawerV2 *categoryDrawerV2;
    int categorySpacing;
    bool alternatingBlockColors;
    bool collapsibleBlocks;

    QModelIndex hoveredIndex;

    QPoint pressedPosition;
    QRect rubberBandRect;

    QHash<QString, Block> blocks;
};

#endif // KCATEGORIZEDVIEW_P_H
