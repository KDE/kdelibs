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

WidgetRectItem::WidgetRectItem(const QPoint &pos, QGraphicsView *widget)
    : QObject(widget), m_view(widget)
{
    setFlag(QGraphicsItem::ItemIsMovable);

    m_frame = new QWidget(m_view);
    m_frame->setAutoFillBackground(true);
    //QPalette palette = m_frame->palette();
    //QColor background = QApplication::palette().color(QPalette::Window);
    //kDebug() << background;
    //background.setAlpha(128);
    //palette.setColor(QPalette::Window, background);
    //m_frame->setPalette(palette);
    m_frame->show();

    setPos(m_view->mapToScene(pos));
}

QVariant WidgetRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit itemMoved(sceneBoundingRect().center());
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void WidgetRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QSize size(m_frame->sizeHint());
    setRect(0.0, 0.0, size.width() + 1.0, size.height() + 30.0);
    QGraphicsRectItem::paint(painter, option, widget);

    const QPoint mapped = m_view->mapFromScene(pos());
    m_frame->setGeometry(mapped.x() + 3, mapped.y() + 32, size.width(), size.height());
}

