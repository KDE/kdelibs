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

#include "kwidgetitemdelegate.h"
#include "kwidgetitemdelegate_p.h"

#define POOL_USAGE 0

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KWidgetItemDelegatePoolPrivate
{
public:
    KWidgetItemDelegatePoolPrivate(KWidgetItemDelegate *d)
        : delegate(d), fakeParent(new QWidget()) { }

    KWidgetItemDelegate *delegate;
    QWidget *fakeParent;

    QList<QList<QWidget*> > allocatedWidgets;
    QList<QList<QWidget*> > unusedWidgets;
    QHash<QPersistentModelIndex, QList<QWidget*> > usedWidgets;
};

KWidgetItemDelegatePool::KWidgetItemDelegatePool(KWidgetItemDelegate *delegate)
    : d(new KWidgetItemDelegatePoolPrivate(delegate))
{
}

KWidgetItemDelegatePool::~KWidgetItemDelegatePool()
{
    foreach (const QList<QWidget*> &list, d->allocatedWidgets) {
        qDeleteAll(list);
    }
    delete d->fakeParent;
    delete d;
}

QList<QWidget*> KWidgetItemDelegatePool::findWidgets(const QPersistentModelIndex &index,
                                                     const QStyleOptionViewItem &option)
{
    QList<QWidget*> result;

    if (!index.isValid()) {
        return result;
    }

    if (d->usedWidgets.contains(index)) {
        result = d->usedWidgets[index];
    } else {
        result = d->delegate->createItemWidgets();
        d->allocatedWidgets << result;
        d->usedWidgets[index] = result;

        foreach (QWidget *widget, result) {
            widget->setParent(d->delegate->d->itemView->viewport());
            widget->setVisible(true);
        }
    }

    d->delegate->updateItemWidgets(result, option, index);

    return result;
}
//@endcond
