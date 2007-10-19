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
#include "effectitem.h"

#include <QtGui/QMenu>

#include <Phonon/BackendCapabilities>

#include <kdebug.h>

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
    kDebug() << mouseEvent->button() << mouseEvent->scenePos();
    if (mouseEvent->button() == Qt::LeftButton) {
        QGraphicsItem *itemUnderMouse = itemAt(mouseEvent->scenePos());
        PathItem *pathItem = qgraphicsitem_cast<PathItem *>(itemUnderMouse);
        if (!pathItem && !itemUnderMouse) {
            const QPointF offset(12.0, 12.0);
            QList<QGraphicsItem *> startItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
            if (startItems.isEmpty()) {
                const QPointF offset(25.0, 25.0);
                startItems = items(QRectF(mouseEvent->scenePos() - offset, mouseEvent->scenePos() + offset));
            }
            if (startItems.size() == 1) {
                m_startItem = qgraphicsitem_cast<MediaObjectItem *>(startItems.first());
                if (!m_startItem) {
                    m_startItem = qgraphicsitem_cast<EffectItem *>(startItems.first());
                }
                if (m_startItem) {
                    m_lineItem = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(), mouseEvent->scenePos()));
                    addItem(m_lineItem);
                    return;
                }
                pathItem = qgraphicsitem_cast<PathItem *>(startItems.first());
            }
            if (!pathItem) {
                addItem(new RedRectItem(mouseEvent->scenePos()));
                return;
            }
        }
        if (pathItem) {
            // allow to disconnect or add an effect
            QMenu popupMenu;
            QAction *disconnectAction = popupMenu.addAction("disconnect");
            popupMenu.addSeparator();

            //QMenu *effectMenu = popupMenu.addMenu("add Effect");
            QList<EffectDescription> effectList = Phonon::BackendCapabilities::availableAudioEffects();
            QHash<QAction *, EffectDescription> actionHash;
            foreach (const EffectDescription &d, effectList) {
                QAction *subAction = popupMenu.addAction("add " + d.name());
                actionHash.insert(subAction, d);
            }

            QAction *triggeredAction = popupMenu.exec(mouseEvent->screenPos());
            if (triggeredAction) {
                kDebug() << "popupMenu exec triggered action" << triggeredAction->metaObject()->className();
                if (actionHash.contains(triggeredAction)) {
                    EffectDescription d = actionHash[triggeredAction];
                    Q_ASSERT(m_view);
                    new EffectItem(d, pathItem, m_view);
                } else {
                    // disconnect
                    Q_ASSERT(triggeredAction == disconnectAction);
                    pathItem->path().disconnect();
                    delete pathItem;
                }
                return;
            } else {
                kDebug() << "popupMenu no action";
            }
        }
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
        if (endItems.size() == 1 && endItems.first() != m_startItem) {
            QGraphicsItem *endItem = endItems.first();
            MediaNode *sourceNode = 0;
            MediaNode *sinkNode = 0;
            WidgetRectItem *sourceItem = 0;
            WidgetRectItem *sinkItem = 0;

            MediaObjectItem *source = qgraphicsitem_cast<MediaObjectItem *>(m_startItem);
            if (source) {
                sourceNode = source->mediaNode();
                sourceItem = source;
            } else {
                EffectItem *source= qgraphicsitem_cast<EffectItem *>(m_startItem);
                if (source) {
                    sourceNode = source->mediaNode();
                    sourceItem = source;
                }
            }
            if (sourceItem && sourceNode) {
                SinkItem *sink = qgraphicsitem_cast<SinkItem *>(endItem);
                if (sink) {
                    sinkNode = sink->mediaNode();
                    sinkItem = sink;
                } else {
                    EffectItem *sink = qgraphicsitem_cast<EffectItem *>(endItem);
                    if (sink) {
                        sinkNode = sink->mediaNode();
                        sinkItem = sink;
                    }
                }
                if (sinkItem && sinkNode) {
                    Path p = Phonon::createPath(sourceNode, sinkNode);
                    if (p.isValid()) {
                        addItem(new PathItem(sourceItem, sinkItem, p));
                        m_startItem = 0;
                        return;
                    }
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
