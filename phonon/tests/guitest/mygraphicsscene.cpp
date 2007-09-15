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

#include "mygraphicsscene.h"

#include "mediaobjectitem.h"
#include "redrectitem.h"
#include "sinkitem.h"
#include "pathitem.h"

using Phonon::Path;

void MyGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (m_lineItem) {
        QLineF line = m_lineItem->line();
        m_lineItem->setLine(QLineF(line.p1(), mouseEvent->scenePos()));
    }
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton && 0 == itemAt(mouseEvent->scenePos())) {
        const QPointF offset(12.0, 12.0);
        QList<QGraphicsItem *> startItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
        if (startItems.isEmpty()) {
            const QPointF offset(25.0, 25.0);
            startItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
        }
        if (startItems.size() == 1) {
            m_startItem = qgraphicsitem_cast<MediaObjectItem *>(startItems.first());
            if (!m_startItem) {
                m_startItem = qgraphicsitem_cast<SinkItem *>(startItems.first());
            }
            if (m_startItem) {
                m_lineItem = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()));
                addItem(m_lineItem);
                return;
            }
        }
        addItem(new RedRectItem(mouseEvent->scenePos()));
        return;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void MyGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (m_lineItem && mouseEvent->button() == Qt::LeftButton) {
        delete m_lineItem;
        m_lineItem = 0;

        QList<QGraphicsItem *> endItems = items(mouseEvent->scenePos());
        if (endItems.isEmpty()) {
            const QPointF offset(12.0, 12.0);
            endItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
            if (endItems.isEmpty()) {
                const QPointF offset(25.0, 25.0);
                endItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
            }
        }
        if (endItems.size() == 1) {
            QGraphicsItem *endItem = endItems.first();
            MediaObjectItem *source = qgraphicsitem_cast<MediaObjectItem *>(m_startItem);
            SinkItem *sink = 0;
            if (!source) {
                source = qgraphicsitem_cast<MediaObjectItem *>(endItem);
                sink = qgraphicsitem_cast<SinkItem *>(m_startItem);
            } else {
                sink = qgraphicsitem_cast<SinkItem *>(endItem);
            }
            if (source && sink && endItem != m_startItem) {
                Path p = Phonon::createPath(source->mediaNode(), sink->mediaNode());
                if (p.isValid()) {
                    addItem(new PathItem(source, sink, p));
                    m_startItem = 0;
                    return;
                }
            }
        }
        m_startItem = 0;
        addItem(new RedRectItem(mouseEvent->scenePos()));
        return;
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

#include "moc_mygraphicsscene.cpp"
#include "moc_redrectitem.cpp"
