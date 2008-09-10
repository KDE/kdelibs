/**
  * This file is part of the KDE project
  * Copyright (C) 2007-2008 Rafael Fernández López <ereslibre@kde.org>
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

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"

#include <QIcon>
#include <QSize>
#include <QStyle>
#include <QEvent>
#include <QHoverEvent>
#include <QFocusEvent>
#include <QCursor>
#include <QTimer>
#include <QBitmap>
#include <QLayout>
#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QStyleOption>
#include <QPaintEngine>
#include <QCoreApplication>
#include <QAbstractItemView>

#include "kwidgetitemdelegatepool_p.h"

Q_DECLARE_METATYPE(QList<QEvent::Type>)

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
KWidgetItemDelegatePrivate::KWidgetItemDelegatePrivate(KWidgetItemDelegate *q, QObject *parent)
    : QObject(parent)
    , itemView(0)
    , widgetPool(new KWidgetItemDelegatePool(q))
    , model(0)
    , focusedIndex(QPersistentModelIndex())
    , q(q)
{
}

KWidgetItemDelegatePrivate::~KWidgetItemDelegatePrivate()
{
    delete widgetPool;
}

void KWidgetItemDelegatePrivate::_k_slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    updateRowRange(parent, start, end, false);
}

void KWidgetItemDelegatePrivate::_k_slotRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    updateRowRange(parent, start, end, true);
}

void KWidgetItemDelegatePrivate::_k_slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        for (int j = topLeft.column(); j <= bottomRight.column(); ++j) {
            const QModelIndex index = model->index(i, j, topLeft.parent());
            QStyleOptionViewItemV4 optionView;
            optionView.initFrom(itemView->viewport());
            optionView.rect = itemView->visualRect(index);
            widgetPool->findWidgets(index, optionView);
        }
    }
}

void KWidgetItemDelegatePrivate::_k_slotLayoutChanged()
{
    foreach (QWidget *widget, widgetPool->invalidIndexesWidgets()) {
        widget->setVisible(false);
    }
    initializeModel();
}

void KWidgetItemDelegatePrivate::updateRowRange(const QModelIndex &parent, int start, int end, bool isRemoving)
{
    int i = start;
    while (i <= end) {
        for (int j = 0; j < model->columnCount(parent); ++j) {
            const QModelIndex index = model->index(i, j, parent);
            QStyleOptionViewItemV4 optionView;
            optionView.initFrom(itemView->viewport());
            optionView.rect = itemView->visualRect(index);

            QList<QWidget*> widgetList = widgetPool->findWidgets(index, optionView, isRemoving ? KWidgetItemDelegatePool::NotUpdateWidgets
                                                                                               : KWidgetItemDelegatePool::UpdateWidgets);
            if (isRemoving) {
                widgetPool->d->allocatedWidgets.removeAll(widgetList);
                foreach (QWidget *widget, widgetList) {
                    widgetPool->d->widgetInIndex.remove(widget);
                    widget->hide(); // why deleting it crashes ?
                }
            }
        }
        i++;
    }
}

void KWidgetItemDelegatePrivate::initializeModel(const QModelIndex &parent)
{
    for (int i = 0; i < model->rowCount(parent); ++i) {
        for (int j = 0; j < model->columnCount(parent); ++j) {
            const QModelIndex index = model->index(i, j, parent);
            if (index.isValid()) {
                QStyleOptionViewItemV4 optionView;
                optionView.initFrom(itemView->viewport());
                optionView.rect = itemView->visualRect(index);
                widgetPool->findWidgets(index, optionView);
            }
        }
        // Check if we need to go recursively through the childs of parent (if any) to initialize
        // all possible indexes that are shown.
        const QModelIndex index = model->index(i, 0, parent);
        if (index.isValid() && model->hasChildren(index)) {
            initializeModel(index);
        }
    }
}
//@endcond

KWidgetItemDelegate::KWidgetItemDelegate(QAbstractItemView *itemView, QObject *parent)
    : QAbstractItemDelegate(parent)
    , d(new KWidgetItemDelegatePrivate(this))
{
    Q_ASSERT(itemView);

    itemView->setMouseTracking(true);
    itemView->viewport()->setAttribute(Qt::WA_Hover);

    d->itemView = itemView;

    itemView->viewport()->installEventFilter(d); // mouse events
    itemView->installEventFilter(d);             // keyboard events
}

KWidgetItemDelegate::~KWidgetItemDelegate()
{
    delete d;
}

QAbstractItemView *KWidgetItemDelegate::itemView() const
{
    return d->itemView;
}

QPersistentModelIndex KWidgetItemDelegate::focusedIndex() const
{
    return d->focusedIndex;
}

void KWidgetItemDelegate::paintWidgets(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QPersistentModelIndex &index) const
{
}

//@cond PRIVATE
bool KWidgetItemDelegatePrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Destroy) {
        return false;
    }

    Q_UNUSED(watched);
    Q_ASSERT(itemView);

    if (model != itemView->model()) {
        if (model) {
            disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(_k_slotRowsInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsAboutToBeRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(_k_slotDataChanged(QModelIndex,QModelIndex)));
            disconnect(model, SIGNAL(layoutChanged()), q, SLOT(_k_slotLayoutChanged()));
        }
        model = itemView->model();
        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(_k_slotRowsInserted(QModelIndex,int,int)));
        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(_k_slotRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(_k_slotDataChanged(QModelIndex,QModelIndex)));
        connect(model, SIGNAL(layoutChanged()), q, SLOT(_k_slotLayoutChanged()));
        initializeModel();
    }

    switch (event->type()) {
        case QEvent::Polish:
        case QEvent::Resize:
            QTimer::singleShot(0, this, SLOT(initializeModel()));
            itemView->viewport()->update();
            break;
        case QEvent::MouseMove: {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                focusedIndex = itemView->indexAt(mouseEvent->pos());
            }
            break;
        default:
            break;
    }

    return QObject::eventFilter(watched, event);
}
//@endcond

void KWidgetItemDelegate::setBlockedEventTypes(QWidget *widget, QList<QEvent::Type> types) const
{
    widget->setProperty("goya:blockedEventTypes", qVariantFromValue(types));
}

QList<QEvent::Type> KWidgetItemDelegate::blockedEventTypes(QWidget *widget) const
{
    return widget->property("goya:blockedEventTypes").value<QList<QEvent::Type> >();
}

#include "kwidgetitemdelegate.moc"
#include "kwidgetitemdelegate_p.moc"
