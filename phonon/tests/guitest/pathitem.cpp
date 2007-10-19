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

#include "pathitem.h"
#include <QtGui/QLinearGradient>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>

PathItem::PathItem(WidgetRectItem *start, WidgetRectItem *end, const Path &_path)
    : m_path(_path)
{
    m_startPos.setX(start->sceneBoundingRect().right());
    m_startPos.setY(start->sceneBoundingRect().center().y());
    m_endPos.setX(end->sceneBoundingRect().left());
    m_endPos.setY(end->sceneBoundingRect().center().y());
    updatePainterPath();

    setZValue(-1.0);
    connect(start->qObject(), SIGNAL(itemMoved(const QRectF &)), SLOT(startMoved(const QRectF &)));
    connect(end->qObject(), SIGNAL(itemMoved(const QRectF &)), SLOT(endMoved(const QRectF &)));
}

void PathItem::startMoved(const QRectF &pos)
{
    m_startPos.setX(pos.right());
    m_startPos.setY(pos.center().y());
    updatePainterPath();
}

void PathItem::endMoved(const QRectF &pos)
{
    m_endPos.setX(pos.left());
    m_endPos.setY(pos.center().y());
    updatePainterPath();
}

void PathItem::updatePainterPath()
{
    QPainterPath path;
    path.moveTo(m_startPos);
    path.cubicTo(m_startPos + QPointF(150.0, 0.0),
            m_endPos - QPointF(150.0, 0.0),
            m_endPos);
    setPath(path);

    QLinearGradient gradient(m_startPos, m_endPos);
    gradient.setColorAt(0, QColor(64, 0, 0, 200));
    gradient.setColorAt(1, QColor(0, 64, 0, 200));
    setPen(QPen(gradient, 3.0));

    updateChildrenPositions();
}

void PathItem::updateChildrenPositions()
{
    const qreal divisor = QGraphicsItem::children().count() + 1;
    int positionOnLine = 0;
    foreach (QGraphicsItem *item, QGraphicsItem::children()) {
        item->setPos(QGraphicsPathItem::path().pointAtPercent(++positionOnLine / divisor) - item->boundingRect().center());
    }
}
