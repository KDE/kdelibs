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

/**
  * This class is necessary to be installed because of the templated method.
  * It is private in the sense of having clean the public header.
  * Do not forget that this file _has_ to be installed.
  */

#ifndef KWIDGETITEMDELEGATE_P_H
#define KWIDGETITEMDELEGATE_P_H

#include <QtGui/QItemSelectionModel>

class KWidgetItemDelegate;

class KWidgetItemDelegatePrivate : public QObject
{
    Q_OBJECT

public:
    explicit KWidgetItemDelegatePrivate(KWidgetItemDelegate *q, QObject *parent = 0);
    ~KWidgetItemDelegatePrivate();

    void analyzeInternalMouseEvents(const QStyleOptionViewItem &option, QMouseEvent *mouseEvent);

    QPoint mappedPointForWidget(QWidget *widget, const QPersistentModelIndex &index, const QPoint &pos) const;
    QRect widgetRect(QWidget *widget, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const;

public Q_SLOTS:
    void slotCurrentChanged(const QModelIndex &currentIndex, const QModelIndex &previousIndex);
    void slotSelectionModelDestroyed();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

public:
    QAbstractItemView *itemView;
    QPersistentModelIndex hoveredIndex;
    QPersistentModelIndex lastHoveredIndex;
    QPersistentModelIndex focusedIndex;

    // If we do a press event, and we later move the mouse, those events will be forwarded to
    // the widget that has been pressed (while still not released).
    QPersistentModelIndex buttonPressedIndex;

    QPersistentModelIndex currentIndex;
    QItemSelectionModel *selectionModel;

    KWidgetItemDelegatePool *widgetPool;

    KWidgetItemDelegate *q;
};

#endif
