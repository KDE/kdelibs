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

#include "facetmodel.h"
#include "simplefacet.h"
#include "dynamicresourcefacet.h"
#include "datefacet.h"
#include "facet.h"

#include <QtCore/QDate>

#include <Soprano/Vocabulary/NAO>

#include "andterm.h"
#include "orterm.h"
#include "comparisonterm.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"
#include "andterm.h"
#include "standardqueries.h"
#include "tag.h"

#include "tmo.h"
#include "nmo.h"
#include "nco.h"
#include "nfo.h"
#include "nie.h"

#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kcalendarsystem.h"
#include "kguiitem.h"

using namespace Nepomuk::Query;


class Nepomuk::Utils::FacetModel::Private
{
public:
    QList<Facet*> m_facets;

    bool m_blockQueryTermChangedSignal;

    void addFacet( Facet* facet );
    QModelIndex parentIndexForFacet( Facet* facet ) const;

    void handleFacetsChanged();

    void _k_queryTermChanged();
    void _k_facetSelectionChanged( Nepomuk::Utils::Facet* facet );
    void _k_facetLayoutChanged( Nepomuk::Utils::Facet* );

    FacetModel* q;
};


void Nepomuk::Utils::FacetModel::Private::addFacet( Facet* facet )
{
    q->connect(facet, SIGNAL(queryTermChanged(Nepomuk::Utils::Facet*,Nepomuk::Query::Term)),
               SLOT(_k_queryTermChanged()) );
    q->connect(facet, SIGNAL(selectionChanged(Nepomuk::Utils::Facet*)),
               SLOT(_k_facetSelectionChanged(Nepomuk::Utils::Facet*)) );
    q->connect(facet, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)),
               SLOT(_k_facetLayoutChanged(Nepomuk::Utils::Facet*)) );
    m_facets.append(facet);
}


QModelIndex Nepomuk::Utils::FacetModel::Private::parentIndexForFacet( Facet* facet ) const
{
    const int i = m_facets.indexOf( facet );
    return q->index( i, 0, QModelIndex() );
}


void Nepomuk::Utils::FacetModel::Private::handleFacetsChanged()
{
    if( !m_blockQueryTermChangedSignal )
        emit q->queryTermChanged( q->queryTerm() );
}


void Nepomuk::Utils::FacetModel::Private::_k_queryTermChanged()
{
    handleFacetsChanged();
}


void Nepomuk::Utils::FacetModel::Private::_k_facetSelectionChanged( Nepomuk::Utils::Facet* facet )
{
    kDebug();
    QModelIndex parent = parentIndexForFacet( facet );
    if( facet->count() )
        emit q->dataChanged( q->index( 0, 0, parent ), q->index( q->rowCount(parent)-1, 0, parent ) );
}


void Nepomuk::Utils::FacetModel::Private::_k_facetLayoutChanged( Nepomuk::Utils::Facet* )
{
    // sadly we do not know the differences
    q->reset();
}


Nepomuk::Utils::FacetModel::FacetModel( QObject* parent )
    : QAbstractItemModel( parent ),
      d(new Private() )
{
    d->q = this;
    d->m_blockQueryTermChangedSignal = false;
}


Nepomuk::Utils::FacetModel::~FacetModel()
{
    qDeleteAll( d->m_facets );
    delete d;
}


int Nepomuk::Utils::FacetModel::columnCount( const QModelIndex& ) const
{
    return 1;
}


QVariant Nepomuk::Utils::FacetModel::data( const QModelIndex& index, int role ) const
{
    if( index.isValid() ) {
        if( index.internalPointer() ) {
            Facet* facet = static_cast<Facet*>( index.internalPointer() );

            if ( role == FacetRole )
                return QVariant::fromValue( facet );

            switch( role ) {
            case Qt::DisplayRole: {
                QString title = facet->text(index.row());
                return title;
            }
            case Qt::CheckStateRole:
                return( facet->isSelected(index.row()) ? Qt::Checked : Qt::Unchecked );
            }
        }
        else {
            switch( role ) {
            case FacetRole:
                return QVariant::fromValue( d->m_facets[index.row()] );

//             case Qt::DisplayRole:
//                 return d->m_facets[index.row()]->facet().title();
            }
        }
    }

    return QVariant();
}


bool Nepomuk::Utils::FacetModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( role == Qt::CheckStateRole && index.internalPointer() ) {
        Facet* facet = static_cast<Facet*>( index.internalPointer() );
        facet->setSelected( index.row(), value.toBool() );
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Utils::FacetModel::hasChildren( const QModelIndex& parent ) const
{
    // facet terms don't have children
    if( parent.internalPointer() ) {
        return false;
    }
    else {
        return rowCount(parent) > 0;
    }
}


QModelIndex Nepomuk::Utils::FacetModel::parent( const QModelIndex& index ) const
{
    if( index.internalPointer() ) {
        return d->parentIndexForFacet( static_cast<Facet*>( index.internalPointer() ) );
    }
    else {
        return QModelIndex();
    }
}


int Nepomuk::Utils::FacetModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() ) {
        return d->m_facets.count();
    }
    else if( parent.row() < d->m_facets.count() ) {
        return d->m_facets[parent.row()]->count();
    }
    else {
        return 0;
    }
}


QModelIndex Nepomuk::Utils::FacetModel::index( int row, int column, const QModelIndex& parent ) const
{
    if(row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent)) {
        return QModelIndex();
    }
    if( parent.isValid() ) {
        return createIndex( row, column, d->m_facets[parent.row()] );
    }
    else {
        return createIndex( row, column );
    }
}


Qt::ItemFlags Nepomuk::Utils::FacetModel::flags( const QModelIndex& index ) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }
    // we do not even allow selection since that does not make much sense for our use case
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if( index.internalPointer() ) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}


void Nepomuk::Utils::FacetModel::addFacet( Facet* facet )
{
    d->addFacet( facet );
    reset();
    d->handleFacetsChanged();
    // TODO: do not use reset but the fancy begin/end methods
}


void Nepomuk::Utils::FacetModel::setFacets( const QList<Facet*>& facets )
{
    clear();
    Q_FOREACH( Facet* facet, facets ) {
        d->addFacet( facet );
    }
    reset();
    d->handleFacetsChanged();
}


void Nepomuk::Utils::FacetModel::clearSelection()
{
    d->m_blockQueryTermChangedSignal = true;
    Q_FOREACH( Facet* facet, d->m_facets ) {
        facet->clearSelection();
    }
    d->m_blockQueryTermChangedSignal = false;
    d->handleFacetsChanged();
}


void Nepomuk::Utils::FacetModel::clear()
{
    qDeleteAll(d->m_facets);
    d->m_facets.clear();
    reset();
}


QList<Nepomuk::Utils::Facet*> Nepomuk::Utils::FacetModel::facets() const
{
    return d->m_facets;
}


Nepomuk::Query::Term Nepomuk::Utils::FacetModel::queryTerm() const
{
    AndTerm term;
    Q_FOREACH( Facet* facet, d->m_facets ) {
        term.addSubTerm( facet->queryTerm() );
    }
    return term.optimized();
}


Nepomuk::Query::Term Nepomuk::Utils::FacetModel::extractFacetsFromTerm( const Term& term )
{
    // safety net to revent endless loops
    // ===============================
    if ( term == queryTerm() )
        return Term();

    // we do not want to emit any queryTermChanged() signal during this method
    // as it would confuse client code
    // ================================
    d->m_blockQueryTermChangedSignal = true;

    // reset all facets in the model
    // ================================
    Q_FOREACH( Facet* f, d->m_facets ) {
        f->clearSelection();
    }

    // we extract all facets we can find and leave the rest in the query
    // ================================
    Term restTerm = term.optimized();

    // first we check if the main term is already a facet term
    // (this way we can also handle facets that use AndTerms)
    // ================================

    // if any of the facets contains the term, set it to selected
    Q_FOREACH( Facet* f, d->m_facets ) {
        if( f->selectFromTerm( restTerm ) ) {
            restTerm = Term();
            break;
        }
    }

    // now go into an AndTerm and check each sub term for facet
    // ================================
    if( restTerm.isAndTerm() ) {
        // we need to cache the facets since we cannot use MatchOne facets more than once
        QList<Facet*> facets = d->m_facets;

        AndTerm restAndTerm;
        foreach( const Term& term, restTerm.toAndTerm().subTerms() ) {
            bool termFound = false;

            Q_FOREACH( Facet* f, facets ) {
                if( f->selectFromTerm( term ) ) {
                    termFound = true;
                    // we can only use MatchAll facets more than once
                    // Example: we have an exclusive facet that can handle both term A and B
                    //          we now call selectFromTerm(A) and selectFromTerm(B)
                    //          both return true but only the latter is the one that sticks.
                    //          thus, we implicitely drop A from the query!
                    // None-exclusive facets are not a problem as they simply add to their
                    // existing selection.
                    if( f->selectionMode() != Facet::MatchAll )
                        facets.removeAll(f);
                    break;
                }
            }

            // we did not find a matching facet
            if( !termFound )
                restAndTerm.addSubTerm( term );
        }

        restTerm = restAndTerm;
    }

    // done with the facet selection
    // ================================
    d->m_blockQueryTermChangedSignal = false;

    d->handleFacetsChanged();

    return restTerm.optimized();
}


void Nepomuk::Utils::FacetModel::setClientQuery( const Nepomuk::Query::Query& query )
{
    Q_FOREACH( Facet* facet, d->m_facets ) {
        facet->setClientQuery( query );
    }
}

#include "facetmodel.moc"
