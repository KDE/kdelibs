/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "facetdelegate.h"
#include "facet.h"
#include "facetmodel.h"

#include <QtGui/QAbstractProxyModel>
#include <QtGui/QApplication>
#include <QtGui/QWidget>

Nepomuk::Utils::FacetDelegate::FacetDelegate( QObject* parent )
    : QItemDelegate( parent ),
      m_isExclusive( false )
{
}

Nepomuk::Utils::FacetDelegate::~FacetDelegate()
{
}

void Nepomuk::Utils::FacetDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    // hacky way to not paint anything on title items which we use as spacers for now
    if( index.data(Qt::DisplayRole).toString().isEmpty() ) {
        return;
    }

    const QAbstractProxyModel* proxyModel = qobject_cast<const QAbstractProxyModel*>(index.model());
    const Facet* facet = proxyModel->data( index, FacetModel::FacetRole ).value<Facet*>();
    if( facet ) {
        m_isExclusive = ( facet->selectionMode() == Facet::MatchOne );
    }

    QItemDelegate::paint( painter, option, index );
}

void Nepomuk::Utils::FacetDelegate::drawCheck( QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, Qt::CheckState state ) const
{
    if( m_isExclusive ) {
        const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>( &option );
        const QWidget* widget = v3 ? v3->widget : 0;

        QStyleOptionViewItem opt(option);
        opt.rect = rect;
        opt.state = opt.state & ~QStyle::State_HasFocus;

        switch( state ) {
        case Qt::Unchecked:
            opt.state |= QStyle::State_Off;
            break;
        case Qt::PartiallyChecked:
            opt.state |= QStyle::State_NoChange;
            break;
        case Qt::Checked:
            opt.state |= QStyle::State_On;
            break;
        }

        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawPrimitive( QStyle::PE_IndicatorRadioButton, &opt, painter, widget );
    }
    else {
        QItemDelegate::drawCheck( painter, option, rect, state );
    }
}
