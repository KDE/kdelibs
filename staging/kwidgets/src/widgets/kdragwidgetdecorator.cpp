/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdragwidgetdecorator.h"

#include <QApplication>
#include <QDrag>
#include <QMouseEvent>
#include <QWidget>

class KDragWidgetDecoratorPrivate
{
public:
    KDragWidgetDecoratorPrivate()
        : dragEnabled(true),
          decoratedWidget(0)
    {
    }

    bool dragEnabled;
    QWidget *decoratedWidget;
    QPoint startPos;
};

KDragWidgetDecorator::KDragWidgetDecorator(QWidget *parent)
    : QObject(parent),
      d(new KDragWidgetDecoratorPrivate)
{
    parent->installEventFilter(this);
    d->decoratedWidget = parent;
}

KDragWidgetDecorator::~KDragWidgetDecorator()
{
    delete d;
}

bool KDragWidgetDecorator::isDragEnabled() const
{
    return d->dragEnabled;
}

void KDragWidgetDecorator::setDragEnabled(bool enable)
{
    d->dragEnabled = enable;
}

bool KDragWidgetDecorator::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched == d->decoratedWidget);

    if (!d->dragEnabled) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        d->startPos = e->pos();

    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if ((e->buttons() & Qt::LeftButton) &&
            (e->pos() - d->startPos).manhattanLength() >
            QApplication::startDragDistance()) {
            startDrag();
            d->decoratedWidget->setProperty("down", false);
            return true;
        }
    }

    return false;
}

QWidget *KDragWidgetDecorator::decoratedWidget() const
{
    return d->decoratedWidget;
}

QDrag *KDragWidgetDecorator::dragObject()
{
    return 0;
}

void KDragWidgetDecorator::startDrag()
{
    QDrag *drag = dragObject();
    if (drag) {
        drag->exec(Qt::CopyAction);
    }
}

