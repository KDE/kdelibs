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

#include "simplefacet.h"
#include "andterm.h"
#include "orterm.h"
#include "query.h"

#include "kguiitem.h"
#include "kdebug.h"

#include <QtCore/QSet>

using namespace Nepomuk::Query;


class Nepomuk::Utils::SimpleFacet::Private
{
public:
    Private()
        : m_selectionMode(Facet::MatchOne) {
    }

    int indexOf( const Term& term ) const;

    Facet::SelectionMode m_selectionMode;

    QList<Nepomuk::Query::Term> m_terms;
    QList<KGuiItem> m_titles;

    QSet<int> m_selectedFacets;

    SimpleFacet* q;
};


int Nepomuk::Utils::SimpleFacet::Private::indexOf( const Term& term ) const
{
    return m_terms.indexOf( term );
}


Nepomuk::Utils::SimpleFacet::SimpleFacet( QObject* parent )
    : Facet(parent),
      d(new Private())
{
    d->q = this;
}


Nepomuk::Utils::SimpleFacet::~SimpleFacet()
{
    delete d;
}


void Nepomuk::Utils::SimpleFacet::setSelectionMode( SelectionMode mode )
{
    d->m_selectionMode = mode;
    clearSelection();
}


Nepomuk::Utils::Facet::SelectionMode Nepomuk::Utils::SimpleFacet::selectionMode() const
{
    return d->m_selectionMode;
}


Nepomuk::Query::Term Nepomuk::Utils::SimpleFacet::term() const
{
    if( d->m_terms.isEmpty() ||
        d->m_selectedFacets.isEmpty() ) {
        return Term();
    }
    else {
        switch( d->m_selectionMode ) {
        case MatchAll: {
            AndTerm andTerm;
            Q_FOREACH( int i, d->m_selectedFacets ) {
                andTerm.addSubTerm( termAt(i) );
            }
            return andTerm;
        }
        case MatchAny: {
            OrTerm orTerm;
            Q_FOREACH( int i, d->m_selectedFacets ) {
                orTerm.addSubTerm( termAt(i) );
            }
            return orTerm;
        }
        case MatchOne:
            return termAt( *d->m_selectedFacets.constBegin() );
        }
    }

    // make gcc shut up
    return Term();
}


int Nepomuk::Utils::SimpleFacet::count() const
{
    return d->m_terms.count();
}


Nepomuk::Query::Term Nepomuk::Utils::SimpleFacet::termAt( int index ) const
{
    return d->m_terms[index];
}


bool Nepomuk::Utils::SimpleFacet::isSelected( int index ) const
{
    return d->m_selectedFacets.contains( index );
}


KGuiItem Nepomuk::Utils::SimpleFacet::guiItem( int index ) const
{
    return d->m_titles[index];
}


void Nepomuk::Utils::SimpleFacet::clear()
{
    d->m_terms.clear();
    d->m_titles.clear();
    d->m_selectedFacets.clear();
    setTermChanged();
}


void Nepomuk::Utils::SimpleFacet::addTerm( const QString& text, const Nepomuk::Query::Term& term )
{
    addTerm( KGuiItem(text), term );
}


void Nepomuk::Utils::SimpleFacet::addTerm( const KGuiItem& title, const Nepomuk::Query::Term& term )
{
    d->m_titles.append( title );
    d->m_terms.append( term );
    clearSelection();
    setLayoutChanged();
}


void Nepomuk::Utils::SimpleFacet::setSelected( int index, bool selected )
{
    if( selectionMode() == MatchOne ) {
        if( d->m_selectedFacets.contains(index) && !selected ) {
            clearSelection();
        }
        else if( selected ) {
            d->m_selectedFacets.clear();
            d->m_selectedFacets.insert(index);
        }
    }
    else if( selected ) {
        d->m_selectedFacets.insert(index);
    }
    else {
        d->m_selectedFacets.remove(index);
    }
    setSelectionChanged();
    setTermChanged();
}


void Nepomuk::Utils::SimpleFacet::clearSelection()
{
    d->m_selectedFacets.clear();
    if( selectionMode() == MatchOne )
        d->m_selectedFacets.insert(0);
    setSelectionChanged();
    setTermChanged();
}


bool Nepomuk::Utils::SimpleFacet::selectFromTerm( const Nepomuk::Query::Term& term )
{
    // 1. check if term is in our list of terms
    const int i = d->indexOf( term );
    if( i >= 0 ) {
        kDebug() << "Found term at" << i << term;
        setSelected( i, true );
        return true;
    }

    // 2. an OrTerm may be a set of terms in a MatchOne facet
    // 3. an AndTerm may be a set of terms in a MatchAll facet
    if( ( term.isOrTerm() &&
          selectionMode() == MatchOne ) ||
        ( term.isAndTerm() &&
          selectionMode() == MatchAll ) ) {

        QSet<int> selectedTerms;

        // since single terms could consist of OrTerms or AndTerms as well
        // and thus, could be "merged" into the surrounding term, we check
        // every sensible combination of terms instead of only single ones

        QList<Term> subTerms;
        if( term.isAndTerm() )
            subTerms = term.toAndTerm().subTerms();
        else
            subTerms = term.toOrTerm().subTerms();

        while( !subTerms.isEmpty() ) {
            bool found = false;
            for( int len = 1; len <= subTerms.count(); ++len ) {
                const QList<Term> excerpt = subTerms.mid( 0, len );
                Term subTerm;
                if( term.isAndTerm() )
                    subTerm = AndTerm(excerpt).optimized();
                else
                    subTerm = OrTerm(excerpt).optimized();
                const int i = d->indexOf( subTerm );
                if( i >= 0 ) {
                    kDebug() << "Found term at" << i << subTerm;
                    selectedTerms << i;
                    subTerms = subTerms.mid(len);
                    found = true;
                    break;
                }
            }
            if( !found ) {
                kDebug() << "Term not found (not even as subterm):" << subTerms;
                return false;
            }
        }

        Q_FOREACH( int i, selectedTerms )
            setSelected( i );

        return true;
    }

    else {
        kDebug() << "Term not found" << term;
        return false;
    }
}

#include "simplefacet.moc"
