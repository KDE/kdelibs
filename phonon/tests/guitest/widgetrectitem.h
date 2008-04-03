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

#ifndef WIDGETRECTITEM_H
#define WIDGETRECTITEM_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QPoint>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsView>

class WidgetRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    protected:
        WidgetRectItem(QGraphicsItem *parent, QGraphicsView *widget);
        WidgetRectItem(const QPoint &pos, QGraphicsView *widget);

    public:
        ~WidgetRectItem();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QObject *qObject() { return this; }
        const QObject *qObject() const { return this; }

        bool eventFilter(QObject *obj, QEvent *e);

    signals:
        void itemMoved(const QRectF &newSceneCenterPos);

    protected:
        void setTitle(const QString &title);

        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        QWidget *m_frame;

    private:
        QGraphicsView *m_view;
        QString m_title;
};

#endif // WIDGETRECTITEM_H
