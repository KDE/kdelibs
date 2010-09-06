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

#include "facetcontainer_p.h"
#include "andterm.h"
#include "orterm.h"
#include "resourcemanager.h"

#include <Soprano/Util/AsyncQuery>
#include <Soprano/LiteralValue>
#include <Soprano/Node>

#include "kdebug.h"

Nepomuk::Query::FacetContainer::FacetContainer( const Facet& facet, QObject* parent )
    : QObject( parent ),
      m_facet(facet),
      m_countQuery(0)
{
    if( m_facet.exclusive() ) {
        m_selectedFacets << 0;
    }
    m_frequencies.fill(-1, m_facet.count());
}


Nepomuk::Query::FacetContainer::~FacetContainer()
{
    if( m_countQuery ) {
        m_countQuery->disconnect();
        m_countQuery->close();
    }
}


void Nepomuk::Query::FacetContainer::setSelected( int index, bool selected )
{
    if( m_facet.exclusive() ) {
        if( selected ) {
            if( m_selectedFacets.count() != 1 ||
                *m_selectedFacets.constBegin() != index ) {
                m_selectedFacets.clear();
                m_selectedFacets.insert(index);
                emit selectionChanged(this);
            }
        }
        else {
            if( m_selectedFacets.count() != 1 ||
                *m_selectedFacets.begin() != 0 ) {
                m_selectedFacets.clear();
                m_selectedFacets.insert(0);
                emit selectionChanged(this);
            }
        }
    }
    else if( selected ) {
        if( !m_selectedFacets.contains(index) ) {
            m_selectedFacets.insert(index);
            emit selectionChanged(this);
        }
    }
    else {
        if( m_selectedFacets.contains(index) ) {
            m_selectedFacets.remove(index);
            emit selectionChanged(this);
        }
    }
}


Nepomuk::Query::Term Nepomuk::Query::FacetContainer::selectedTerm() const
{
    if( m_facet.exclusive() ) {
        return m_facet.termAt( *m_selectedFacets.constBegin() );
    }
    else {
        AndTerm andTerm;
        Q_FOREACH( int i, m_selectedFacets ) {
            andTerm.addSubTerm( m_facet.termAt(i) );
        }
        return andTerm;
    }
}


bool Nepomuk::Query::FacetContainer::selectFromTerm( const Term& term )
{
    // 1. check if term is in our list of terms
    const int i = indexOf( term );
    if( i >= 0 ) {
        kDebug() << m_facet.title() << "Found term at" << i << term;
        setSelected( i, true );
        return true;
    }

    // 2. an OrTerm may be a set of terms in a non-exclusive facte
    else if( term.isOrTerm() &&
             !m_facet.exclusive() ) {
        QHash<int, int> selectionHash;
        Q_FOREACH( const Term& term, term.toOrTerm().subTerms() ) {
            int i = indexOf( term );
            if( i < 0 ) {
                kDebug() << m_facet.title() << "Term not found" << term;
                return false;
            }
            kDebug() << m_facet.title() << "Found term at" << i << term;
            setSelected( i, true );
        }
        return true;
    }

    else {
        kDebug() << m_facet.title() << "Term not found" << term;
        return false;
    }
}


void Nepomuk::Query::FacetContainer::reset()
{
    m_selectedFacets.clear();
    if( m_facet.exclusive() ) {
        m_selectedFacets.insert(0);
    }
    emit selectionChanged(this);
}


void Nepomuk::Query::FacetContainer::resetFrequencies()
{
    m_frequencies.fill(-1, m_facet.count());
}


void Nepomuk::Query::FacetContainer::updateFrequencies( const Nepomuk::Query::Query& baseQuery )
{
    kDebug() << baseQuery;

    // reset frequencies
    m_frequencies.fill(-1, m_facet.count());

    // re-query the frequencies
    m_frequencyBaseQuery = baseQuery;
    m_currentFrequencyIndex = -1;

    // we do not delete an old query since that would wait for
    // it to finish which might take long and, thus, would block
    // the main thread.
    if( m_countQuery ) {
        m_countQuery->disconnect();
        m_countQuery->close();
        m_countQuery = 0;
    }
    queryNextFrequency();
}


void Nepomuk::Query::FacetContainer::slotCountQueryDone( Soprano::Util::AsyncQuery* query )
{
    // this is a small hack since the above disconnect does not seem to work in all cases
    if( query != m_countQuery )
        return;

    m_countQuery = 0;
    const int frequency = query->binding(0).literal().toInt();
    query->close();
    kDebug() << frequency;
    m_frequencies[m_currentFrequencyIndex] = frequency;
    emit frequencyChanged( this, m_currentFrequencyIndex, frequency );
}


void Nepomuk::Query::FacetContainer::queryNextFrequency()
{
    // do not get the frequency of selected terms. They are of no interest
    do {
        ++m_currentFrequencyIndex;
    } while( m_selectedFacets.contains( m_currentFrequencyIndex ) &&
             m_currentFrequencyIndex < m_facet.count() );

    if( m_currentFrequencyIndex < m_facet.count() ) {
        Query query(m_frequencyBaseQuery);
        query.setTerm( query.term() && m_facet.termAt(m_currentFrequencyIndex));

        kDebug() << query.toSparqlQuery(Query::CreateCountQuery);
        m_countQuery = Soprano::Util::AsyncQuery::executeQuery( ResourceManager::instance()->mainModel(),
                                                                query.toSparqlQuery(Query::CreateCountQuery),
                                                                Soprano::Query::QueryLanguageSparql );
        connect( m_countQuery, SIGNAL(nextReady(Soprano::Util::AsyncQuery*)),
                 SLOT(slotCountQueryDone(Soprano::Util::AsyncQuery*)) );
        connect( m_countQuery, SIGNAL(finished(Soprano::Util::AsyncQuery*)),
                 SLOT(queryNextFrequency()) );
    }
}


int Nepomuk::Query::FacetContainer::indexOf( const Term& term ) const
{
    QList<Term> terms = m_facet.termList();
    for( int i = 0; i < terms.count(); ++i ) {
        if( term == terms[i] ) {
            return i;
        }
    }
    return -1;
}

#include "facetcontainer_p.moc"
