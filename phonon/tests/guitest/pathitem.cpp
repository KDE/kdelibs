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

PathItem::PathItem(WidgetRectItem *start, WidgetRectItem *end, const Path &path)
    : m_path(path)
{
    setLine(QLineF(start->sceneBoundingRect().center(), end->sceneBoundingRect().center()));
    setZValue(-1.0);
    connect(start->qObject(), SIGNAL(itemMoved(const QPointF &)), SLOT(startMoved(const QPointF &)));
    connect(end->qObject(), SIGNAL(itemMoved(const QPointF &)), SLOT(endMoved(const QPointF &)));
}

void PathItem::startMoved(const QPointF &pos)
{
    const QLineF old = line();
    setLine(QLineF(pos, old.p2()));
}

void PathItem::endMoved(const QPointF &pos)
{
    const QLineF old = line();
    setLine(QLineF(old.p1(), pos));
}

