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

#include "proxyfacet.h"
#include "andterm.h"
#include "query.h"

#include "kguiitem.h"
#include "kdebug.h"

class Nepomuk::Utils::ProxyFacet::Private
{
public:
    Private()
        : m_sourceFacet(0),
          m_facetConditionMet(true) {
    }

    void updateConditionStatus();

    Facet* m_sourceFacet;

    Query::Term m_facetCondition;
    bool m_facetConditionMet;

    ProxyFacet* q;
};


namespace {
    /**
     * Checks if a query contains a certain term in a non-optional manner. Basically this means
     * that either the query's term is the term in question or the query term is an AndTerm which
     * contains the requested term. All other situations result in an optional usage of \p term
     * or are too complex to handle here.
     */
    bool containsTerm( const Nepomuk::Query::Query& query, const Nepomuk::Query::Term& term ) {
        Nepomuk::Query::Term queryTerm = query.term().optimized();
        if( queryTerm == term ) {
            return true;
        }
        else if( queryTerm.isAndTerm() ) {
            Q_FOREACH( const Nepomuk::Query::Term& subTerm, queryTerm.toAndTerm().subTerms() ) {
                if( subTerm == term ) {
                    return true;
                }
            }
        }

        // fallback
        return false;
    }
}

void Nepomuk::Utils::ProxyFacet::Private::updateConditionStatus()
{
    bool newFacetConditionMet = true;
    if( m_facetCondition.isValid() ) {
        newFacetConditionMet = containsTerm( q->clientQuery(), m_facetCondition );
        kDebug() << m_facetConditionMet << newFacetConditionMet;
    }

    if( newFacetConditionMet != m_facetConditionMet ) {
        m_facetConditionMet = newFacetConditionMet;
        q->setLayoutChanged();
        q->setQueryTermChanged();
    }

    if( !m_facetConditionMet ) {
        q->clearSelection();
    }
}


Nepomuk::Utils::ProxyFacet::ProxyFacet( QObject* parent )
    : Facet(parent),
      d(new Private())
{
    d->q = this;
}


Nepomuk::Utils::ProxyFacet::~ProxyFacet()
{
    delete d;
}


void Nepomuk::Utils::ProxyFacet::setSourceFacet( Facet* source )
{
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->disconnect(this);
    }

    d->m_sourceFacet = source;
    if( d->m_sourceFacet ) {
        connect(d->m_sourceFacet, SIGNAL(queryTermChanged(Nepomuk::Utils::Facet*,Nepomuk::Query::Term)),
                this, SIGNAL(queryTermChanged(Nepomuk::Utils::Facet*,Nepomuk::Query::Term)));
        connect(d->m_sourceFacet, SIGNAL(selectionChanged(Nepomuk::Utils::Facet*)),
                this, SIGNAL(selectionChanged(Nepomuk::Utils::Facet*)));
        connect(d->m_sourceFacet, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)),
                this, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
    }

    setLayoutChanged();
    setQueryTermChanged();
    setSelectionChanged();
}


Nepomuk::Utils::Facet* Nepomuk::Utils::ProxyFacet::sourceFacet() const
{
    return d->m_sourceFacet;
}


Nepomuk::Utils::Facet::SelectionMode Nepomuk::Utils::ProxyFacet::selectionMode() const
{
    return d->m_sourceFacet ? d->m_sourceFacet->selectionMode() : MatchOne;
}


Nepomuk::Query::Term Nepomuk::Utils::ProxyFacet::queryTerm() const
{
    return facetConditionMet() && d->m_sourceFacet ? d->m_sourceFacet->queryTerm() : Query::Term();
}


int Nepomuk::Utils::ProxyFacet::count() const
{
    return d->m_sourceFacet && facetConditionMet() ? d->m_sourceFacet->count() : 0;
}


bool Nepomuk::Utils::ProxyFacet::isSelected( int index ) const
{
    return d->m_sourceFacet ? d->m_sourceFacet->isSelected(index) : false;
}


KGuiItem Nepomuk::Utils::ProxyFacet::guiItem( int index ) const
{
    return d->m_sourceFacet ? d->m_sourceFacet->guiItem(index) : KGuiItem();
}


void Nepomuk::Utils::ProxyFacet::setSelected( int index, bool selected )
{
    if( d->m_sourceFacet && facetConditionMet() ) {
        d->m_sourceFacet->setSelected( index, selected );
    }
}


void Nepomuk::Utils::ProxyFacet::clearSelection()
{
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->clearSelection();
    }
}


bool Nepomuk::Utils::ProxyFacet::selectFromTerm( const Nepomuk::Query::Term& term )
{
    if( d->m_sourceFacet && facetConditionMet() ) {
        return d->m_sourceFacet->selectFromTerm( term );
    }
    else {
        return false;
    }
}


void Nepomuk::Utils::ProxyFacet::handleClientQueryChange()
{
    d->updateConditionStatus();
    if( d->m_sourceFacet ) {
        d->m_sourceFacet->setClientQuery( clientQuery() );
    }
}


void Nepomuk::Utils::ProxyFacet::setFacetCondition( const Nepomuk::Query::Term& term )
{
    d->m_facetCondition = term;
    d->updateConditionStatus();
}


bool Nepomuk::Utils::ProxyFacet::facetConditionMet() const
{
    return d->m_facetConditionMet;
}

#include "proxyfacet.moc"
