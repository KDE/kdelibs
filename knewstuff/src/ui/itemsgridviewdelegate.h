/*
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_ITEMSGRIDVIEWDELEGATE_H
#define KNEWSTUFF3_UI_ITEMSGRIDVIEWDELEGATE_H

#include "itemsviewbasedelegate.h"
class QToolButton;
namespace KNS3
{
    static const int ItemGridHeight = 202;
    static const int ItemGridWidth = 158;
    
    static const int FrameThickness = 5;
    static const int ItemMargin = 2;
class ItemsGridViewDelegate: public ItemsViewBaseDelegate
{
    Q_OBJECT
public:
    explicit ItemsGridViewDelegate(QAbstractItemView* itemView, Engine* engine, QObject* parent = 0);
    ~ItemsGridViewDelegate();


    // paint the item at index with all its attributes shown
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    // get the list of widgets
    virtual QList<QWidget*> createItemWidgets() const;

    // update the widgets
    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    
    private:
        void createOperationBar();
        void displayOperationBar(const QRect &rect,const QModelIndex & index);
        
    private:
        QWidget *m_operationBar;
        QToolButton *m_detailsButton;
        QToolButton *m_installButton;
        
        QModelIndex m_oldIndex;
        mutable int m_elementYPos;
};
}

#endif
