/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
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

#ifndef KWIDGETITEMDELEGATEPOOL_P_H
#define KWIDGETITEMDELEGATEPOOL_P_H

#include <QtCore/QModelIndex>

class QWidget;
class QStyleOptionViewItem;
class KWidgetItemDelegate;
class KWidgetItemDelegatePoolPrivate;


/**
  * @brief Implements a widget pool. The pool will have a constant size, where
  *        widgets will be allocated. The substitution strategy used is Least
  *        Recently Used (LRU)
  * @note Since this class is only meant for implementation purposes, it is not
  *       exported on the library.
  * @internal
  */
class KWidgetItemDelegatePool
{
public:
    /**
      * Creates a new ItemDelegatePool.
      *
      * @param delegate the ItemDelegate for this pool.
      */

    KWidgetItemDelegatePool(KWidgetItemDelegate *delegate);

    /**
      * Destroys an ItemDelegatePool.
      */
    ~KWidgetItemDelegatePool();

    /**
      * @brief Returns the widget associated to @p index and @p widget
      * @param index The index to search into.
      * @param option a QStyleOptionViewItem.
      * @return A QList of the pointers to the widgets found.
      * @internal
      */
    QList<QWidget*> findWidgets(const QPersistentModelIndex &index, const QStyleOptionViewItem &option) const;

    QList<QWidget*> invalidIndexesWidgets() const;

private:
    KWidgetItemDelegatePoolPrivate *const d;
};

#endif
