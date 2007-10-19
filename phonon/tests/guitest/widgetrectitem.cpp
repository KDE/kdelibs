/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "widgetrectitem.h"
#include <QtCore/QEvent>
#include "pathitem.h"

WidgetRectItem::WidgetRectItem(QGraphicsItem *parent, QGraphicsView *widget)
    : QObject(widget), QGraphicsRectItem(parent), m_view(widget)
{
    m_frame = new QWidget(m_view);
    m_frame->setAutoFillBackground(true);
    m_frame->show();
    m_frame->installEventFilter(this);
}

WidgetRectItem::WidgetRectItem(const QPoint &pos, QGraphicsView *widget)
    : QObject(widget), m_view(widget)
{
    setFlag(QGraphicsItem::ItemIsMovable);

    m_frame = new QWidget(m_view);
    m_frame->setAutoFillBackground(true);
    m_frame->show();

    m_frame->installEventFilter(this);
    setPos(m_view->mapToScene(pos));
}

WidgetRectItem::~WidgetRectItem()
{
    delete m_frame;
}

QVariant WidgetRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit itemMoved(sceneBoundingRect());
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void WidgetRectItem::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void WidgetRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QSize size(m_frame->sizeHint());
    setRect(0.0, 0.0, size.width() + 31.0, size.height() + 30.0);
    QGraphicsRectItem::paint(painter, option, widget);
    painter->drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, m_title);

    const QPoint mapped = m_view->mapFromScene(pos());
    const QRect frameGeometry(mapped.x() + 18, mapped.y() + 17, size.width(), size.height());
    if (m_frame->geometry() != frameGeometry) {
        m_frame->setGeometry(frameGeometry);
        emit itemMoved(sceneBoundingRect());
        PathItem *pathItem = qgraphicsitem_cast<PathItem *>(parentItem());
        if (pathItem) {
            pathItem->updateChildrenPositions();
        }
    }
}

bool WidgetRectItem::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_frame && e->type() == QEvent::LayoutRequest) {
        update();
    }
    return QObject::eventFilter(obj, e);
}

