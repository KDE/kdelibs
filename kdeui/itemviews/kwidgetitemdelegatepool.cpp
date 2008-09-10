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

#include "kwidgetitemdelegatepool_p.h"

#include <math.h>

#include <QtCore/qobjectdefs.h>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaMethod>
#include <QtCore/QMetaProperty>
#include <QtCore/QPair>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtGui/QWidget>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QInputEvent>
#include <QtGui/QAbstractProxyModel>

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"

#define POOL_USAGE 0

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class EventListener
    : public QObject
{
public:
    EventListener(KWidgetItemDelegatePoolPrivate *poolPrivate, QObject *parent = 0)
        : QObject(parent)
        , poolPrivate(poolPrivate)
    {
    }

    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    KWidgetItemDelegatePoolPrivate *poolPrivate;
};

class KWidgetItemDelegatePoolPrivate
{
public:
    KWidgetItemDelegatePoolPrivate(KWidgetItemDelegate *d)
        : delegate(d)
        , eventListener(new EventListener(this))
    {
    }

    KWidgetItemDelegate *delegate;
    EventListener *eventListener;

    QList<QList<QWidget*> > allocatedWidgets;
    QHash<QModelIndex, QList<QWidget*> > usedWidgets;
    QHash<QWidget*, QPersistentModelIndex> widgetInIndex;
};

KWidgetItemDelegatePool::KWidgetItemDelegatePool(KWidgetItemDelegate *delegate)
    : d(new KWidgetItemDelegatePoolPrivate(delegate))
{
}

KWidgetItemDelegatePool::~KWidgetItemDelegatePool()
{
    delete d->eventListener;
    delete d;
}

QList<QWidget*> KWidgetItemDelegatePool::findWidgets(const QPersistentModelIndex &idx,
                                                     const QStyleOptionViewItem &option) const
{
    QList<QWidget*> result;

    if (!idx.isValid()) {
        return result;
    }

    QModelIndex index;
    if (const QAbstractProxyModel *proxyModel = qobject_cast<const QAbstractProxyModel*>(idx.model())) {
        index = proxyModel->mapToSource(idx);
    } else {
        index = idx;
    }

    if (d->usedWidgets.contains(index)) {
        result = d->usedWidgets[index];
    } else {
        result = d->delegate->createItemWidgets();
        d->allocatedWidgets << result;
        d->usedWidgets[index] = result;
        foreach (QWidget *widget, result) {
            d->widgetInIndex[widget] = idx;
            widget->setParent(d->delegate->d->itemView->viewport());
            widget->installEventFilter(d->eventListener);
            widget->setVisible(true);
        }
    }

    foreach (QWidget *widget, result) {
        widget->setVisible(true);
    }

    d->delegate->updateItemWidgets(result, option, idx);

    foreach (QWidget *widget, result) {
        widget->move(widget->x() + option.rect.left(), widget->y() + option.rect.top());
    }

    return result;
}

QList<QWidget*> KWidgetItemDelegatePool::invalidIndexesWidgets() const
{
    QList<QWidget*> result;
    foreach (QWidget *widget, d->widgetInIndex.keys()) {
        if (!d->widgetInIndex[widget].isValid()) {
            result << widget;
        }
    }
    return result;
}

bool EventListener::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = static_cast<QWidget*>(watched);
    switch (event->type()) {
        case QEvent::Enter:
        case QEvent::FocusIn:
        case QEvent::MouseMove:
            poolPrivate->delegate->d->focusedIndex = poolPrivate->widgetInIndex[widget]; // fall through
        default:
            if (dynamic_cast<QInputEvent*>(event) && !poolPrivate->delegate->blockedEventTypes(widget).contains(event->type())) {
                QWidget *viewport = poolPrivate->delegate->d->itemView->viewport();
                if (QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
                    QMouseEvent evt(event->type(), viewport->mapFromGlobal(mouseEvent->globalPos()),
                                    mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
                    QApplication::sendEvent(viewport, &evt);
                } else if (QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent*>(event)) {
                    QWheelEvent evt(viewport->mapFromGlobal(wheelEvent->globalPos()),
                                    wheelEvent->delta(), wheelEvent->buttons(), wheelEvent->modifiers(), wheelEvent->orientation());
                    QApplication::sendEvent(viewport, &evt);
                } else if (QTabletEvent *tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
                    QTabletEvent evt(event->type(), viewport->mapFromGlobal(tabletEvent->globalPos()),
                                     tabletEvent->globalPos(), tabletEvent->hiResGlobalPos(), tabletEvent->device(),
                                     tabletEvent->pointerType(), tabletEvent->pressure(), tabletEvent->xTilt(),
                                     tabletEvent->yTilt(), tabletEvent->tangentialPressure(), tabletEvent->rotation(),
                                     tabletEvent->z(), tabletEvent->modifiers(), tabletEvent->uniqueId());
                    QApplication::sendEvent(viewport, &evt);
                } else {
                    QApplication::sendEvent(viewport, event);
                }
            }
            break;
    }

    return QObject::eventFilter(watched, event);
}
//@endcond
