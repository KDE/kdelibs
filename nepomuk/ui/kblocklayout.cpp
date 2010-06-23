/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
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

/*
  KBlockLayout is based on the FlowLayout example from QT4.
  Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
  Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
  All rights reserved.
  Contact: Nokia Corporation (qt-info@nokia.com)
*/

#include "kblocklayout.h"

#include <QtCore/QList>
#include <QtGui/QStyle>
#include <QtGui/QWidget>

class KBlockLayout::Private
{
public:
    Private()
        : alignment(Qt::AlignLeft|Qt::AlignTop) {
    }

    int smartSpacing(QStyle::PixelMetric pm) const
    {
        QObject *parent = q->parent();
        if (!parent) {
            return -1;
        } else if (parent->isWidgetType()) {
            QWidget *pw = static_cast<QWidget *>(parent);
            return pw->style()->pixelMetric(pm, 0, pw);
        } else {
            return static_cast<QLayout *>(parent)->spacing();
        }
    }

    QList<QLayoutItem*> itemList;

    int m_hSpace;
    int m_vSpace;

    Qt::Alignment alignment;

    KBlockLayout* q;
};


KBlockLayout::KBlockLayout( QWidget* parent, int margin, int hSpacing, int vSpacing )
    : QLayout(parent),
      d( new Private() )
{
    d->q = this;
    setMargin(margin);
    setSpacing(hSpacing, vSpacing);
}

KBlockLayout::KBlockLayout( int margin, int hSpacing, int vSpacing )
    : d( new Private() )
{
    d->q = this;
    setMargin(margin);
    setSpacing(hSpacing, vSpacing);
}

KBlockLayout::~KBlockLayout()
{
    QLayoutItem* item;
    while ((item = takeAt(0)))
        delete item;
    delete d;
}

void KBlockLayout::setAlignment( Qt::Alignment a )
{
    d->alignment = a;
}

Qt::Alignment KBlockLayout::alignment() const
{
    return d->alignment;
}

int KBlockLayout::horizontalSpacing() const
{
    if (d->m_hSpace >= 0) {
        return d->m_hSpace;
    } else {
        return d->smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int KBlockLayout::verticalSpacing() const
{
    if (d->m_vSpace >= 0) {
        return d->m_vSpace;
    } else {
        return d->smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

void KBlockLayout::setSpacing( int h, int v )
{
    d->m_hSpace = h;
    d->m_vSpace = v;
    QLayout::setSpacing( h );
}

void KBlockLayout::addItem( QLayoutItem* item )
{
    d->itemList.append(item);
}

int KBlockLayout::count() const
{
    return d->itemList.size();
}

QLayoutItem *KBlockLayout::itemAt( int index ) const
{
    return d->itemList.value(index);
}

QLayoutItem *KBlockLayout::takeAt( int index )
{
    if (index >= 0 && index < d->itemList.size())
        return d->itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations KBlockLayout::expandingDirections() const
{
    return 0;
}

bool KBlockLayout::hasHeightForWidth() const
{
    return true;
}

int KBlockLayout::heightForWidth( int width ) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void KBlockLayout::setGeometry( const QRect& rect )
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize KBlockLayout::sizeHint() const
{
    // TODO: try to get the items into a square
    QSize size;
    QLayoutItem *item;
    foreach (item, d->itemList) {
        const QSize itemSize = item->minimumSize();
        size.rwidth() += itemSize.width();
        if (itemSize.height() > size.height()) {
            size.setHeight(itemSize.height());
        }
    }

    size.rwidth() += horizontalSpacing() * d->itemList.count();
    size += QSize(2*margin(), 2*margin());
    return size;
}

QSize KBlockLayout::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, d->itemList) {
        size = size.expandedTo(item->minimumSize());
    }

    size += QSize(2*margin(), 2*margin());
    return size;
}

struct Row {
    Row( const QList<QLayoutItem*>& i, int h, int w )
        : items(i), height(h), width(w) {
    }

    QList<QLayoutItem*> items;
    int height;
    int width;
};

int KBlockLayout::doLayout( const QRect& rect, bool testOnly ) const
{
    int x = rect.x();
    int y = rect.y();
    int lineHeight = 0;

    // 1. calculate lines
    QList<Row> rows;
    QList<QLayoutItem*> rowItems;
    for( int i = 0; i < d->itemList.count(); ++i ) {
        QLayoutItem* item = d->itemList[i];
        int nextX = x + item->sizeHint().width() + horizontalSpacing();
        if (nextX - horizontalSpacing() > rect.right() && lineHeight > 0) {
            rows.append( Row( rowItems, lineHeight, x - horizontalSpacing() ) );
            rowItems.clear();

            x = rect.x();
            y = y + lineHeight + verticalSpacing();
            nextX = x + item->sizeHint().width() + horizontalSpacing();
            lineHeight = 0;
        }

        rowItems.append( item );

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    // append the last row
    rows.append( Row( rowItems, lineHeight, x-horizontalSpacing() ) );

    int finalHeight = y + lineHeight - rect.y();
    if( testOnly )
        return finalHeight;

    // 2. place the items
    y = rect.y();
    foreach( const Row &row, rows ) {
        x = rect.x();
        if( alignment() & Qt::AlignRight )
            x += (rect.width() - row.width);
        else if( alignment() & Qt::AlignHCenter )
            x += (rect.width() - row.width)/2;

        foreach( QLayoutItem* item, row.items ) {
            int yy = y;
            if( alignment() & Qt::AlignBottom )
                yy += (row.height - item->sizeHint().height());
            else if( alignment() & Qt::AlignVCenter )
                yy += (row.height - item->sizeHint().height())/2;
            item->setGeometry(QRect(QPoint(x, yy), item->sizeHint()));

            x += item->sizeHint().width() + horizontalSpacing();

            if( alignment() & Qt::AlignJustify )
                x += (rect.width() - row.width)/qMax(row.items.count()-1,1);
        }

        y = y + row.height + verticalSpacing();
    }

    return finalHeight;
}
