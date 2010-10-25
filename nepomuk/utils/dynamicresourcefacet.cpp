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

#include "dynamicresourcefacet.h"

#include "andterm.h"
#include "orterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"
#include "comparisonterm.h"
#include "result.h"
#include "query.h"
#include "queryserviceclient.h"

#include "property.h"
#include "class.h"
#include "resource.h"
#include "resourcemanager.h"

#include "searchwidget.h"

#include "kguiitem.h"
#include "kdebug.h"

#include <QtCore/QSet>


class Nepomuk::Utils::DynamicResourceFacet::Private
{
public:
    Private()
        : m_selectionMode(Facet::MatchOne),
          m_maxRows( 5 ),
          m_haveMore( false ) {
    }

    Types::Class resourceType() const {
        if ( m_resourceType.isValid() )
            return m_resourceType;
        else
            return m_property.range();
    }

    void rebuild( bool clearSelection = true );
    void addResource( const Nepomuk::Resource& res );

    void _k_newEntries( const QList<Nepomuk::Query::Result>& );
    void _k_populateFinished();

    Query::QueryServiceClient m_queryClient;

    Facet::SelectionMode m_selectionMode;
    Types::Property m_property;
    Types::Class m_resourceType;
    int m_maxRows;
    bool m_haveMore;

    /// The query which the client is currently using and which we use
    /// to select a subset of resources that make sense
    Query::Query m_currentQuery;

    // list of resources built by rebuild()
    QList<Resource> m_resources;

    QSet<Resource> m_selectedResources;

    DynamicResourceFacet* q;
};



void Nepomuk::Utils::DynamicResourceFacet::Private::rebuild( bool clearSelection )
{
    kDebug() << clearSelection;
    m_haveMore = false;
    m_resources.clear();
    if( clearSelection )
        m_selectedResources.clear();
    q->setLayoutChanged();

    Query::Query query = q->resourceQuery( m_currentQuery );
    query.setLimit( m_maxRows+1 );
    m_queryClient.query( query );
}


void Nepomuk::Utils::DynamicResourceFacet::Private::addResource( const Nepomuk::Resource& res )
{
    if ( !m_resources.contains( res ) ) {
        m_resources.append( res );
        q->setLayoutChanged();
    }
}


void Nepomuk::Utils::DynamicResourceFacet::Private::_k_newEntries( const QList<Nepomuk::Query::Result>& entries )
{
    kDebug();
    Q_FOREACH( const Query::Result& result, entries ) {
        if( m_resources.count() == m_maxRows ) {
            // add the more... button
            m_haveMore = true;
        }
        else {
            m_resources.append( result.resource() );
        }
    }
    q->setLayoutChanged();
}


void Nepomuk::Utils::DynamicResourceFacet::Private::_k_populateFinished()
{
    kDebug() << m_resources.count();
    m_queryClient.close();

    // clean up selection in case rebuild was calling without clearing it
    QSet<Resource>::iterator it = m_selectedResources.begin();
    while( it != m_selectedResources.end() ) {
        if( m_resources.contains( *it ) )
            ++it;
        else
            it = m_selectedResources.erase(it);
    }
}


Nepomuk::Utils::DynamicResourceFacet::DynamicResourceFacet( QObject* parent )
    : Facet(parent),
      d(new Private())
{
    d->q = this;
    connect( &d->m_queryClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
             this, SLOT(_k_newEntries(QList<Nepomuk::Query::Result>)) );
    connect( &d->m_queryClient, SIGNAL(finishedListing()),
             this, SLOT(_k_populateFinished()) );
}


Nepomuk::Utils::DynamicResourceFacet::~DynamicResourceFacet()
{
    delete d;
}


void Nepomuk::Utils::DynamicResourceFacet::setSelectionMode( SelectionMode mode )
{
    d->m_selectionMode = mode;
    clearSelection();
}


Nepomuk::Utils::Facet::SelectionMode Nepomuk::Utils::DynamicResourceFacet::selectionMode() const
{
    return d->m_selectionMode;
}


Nepomuk::Query::Term Nepomuk::Utils::DynamicResourceFacet::term() const
{
    if( d->m_resources.isEmpty() ||
        d->m_selectedResources.isEmpty() ) {
        return Query::Term();
    }
    else {
        switch( d->m_selectionMode ) {
        case MatchAll: {
            Query::AndTerm andTerm;
            Q_FOREACH( const Resource& res, d->m_selectedResources ) {
                andTerm.addSubTerm( termForResource( res ) );
            }
            return andTerm;
        }
        case MatchAny: {
            Query::OrTerm orTerm;
            Q_FOREACH( const Resource& res, d->m_selectedResources ) {
                orTerm.addSubTerm( termForResource(res) );
            }
            return orTerm;
        }
        case MatchOne:
            return termForResource( *d->m_selectedResources.begin() );
        }
    }

    // make gcc shut up
    return Query::Term();
}


int Nepomuk::Utils::DynamicResourceFacet::count() const
{
    int c = d->m_resources.count();
    if ( d->m_haveMore )
        ++c;
    return c;
}


bool Nepomuk::Utils::DynamicResourceFacet::isSelected( int index ) const
{
    if( index < d->m_resources.count() )
        return d->m_selectedResources.contains( d->m_resources[index] );
    else
        return false;
}


void Nepomuk::Utils::DynamicResourceFacet::setRelation( const Types::Property& prop )
{
    d->m_property = prop;
    d->rebuild();
}


void Nepomuk::Utils::DynamicResourceFacet::setResourceType( const Types::Class& type )
{
    d->m_resourceType = type;
    d->rebuild();
}


void Nepomuk::Utils::DynamicResourceFacet::setMaxRows( int max )
{
    d->m_maxRows = max;
    d->rebuild();
}


Nepomuk::Types::Property Nepomuk::Utils::DynamicResourceFacet::relation() const
{
    return d->m_property;
}


Nepomuk::Types::Class Nepomuk::Utils::DynamicResourceFacet::resourceType() const
{
    return d->resourceType();
}


QList<Nepomuk::Resource> Nepomuk::Utils::DynamicResourceFacet::selectedResources() const
{
    return d->m_selectedResources.toList();
}


int Nepomuk::Utils::DynamicResourceFacet::maxRows() const
{
    return d->m_maxRows;
}


KGuiItem Nepomuk::Utils::DynamicResourceFacet::guiItem( int index ) const
{
    KGuiItem item;

    if ( index < d->m_resources.count() ) {
        item.setText( d->m_resources[index].genericLabel() );
        return item;
    }
    else if ( d->m_haveMore && index == count()-1 ) {
        item.setText( i18nc( "@option:check An item in a list of resources that allows to query for more resources to put in the list", "More..." ) );
    }

    return item;
}


void Nepomuk::Utils::DynamicResourceFacet::setSelected( const Resource& res, bool selected )
{
    if ( d->m_resources.contains( res ) ) {
        setSelected( d->m_resources.indexOf( res ), selected );
    }
}


void Nepomuk::Utils::DynamicResourceFacet::setSelected( int index, bool selected )
{
    if ( d->m_haveMore && index == count()-1 && selected ) {
        const QList<Resource> rl = getMoreResources();
        Q_FOREACH( const Resource& res, rl ) {
            // FIXME: try to honor d->m_maxRows
            if ( !d->m_resources.contains( res ) ) {
                d->m_resources.append( res );
                setLayoutChanged();
            }

            // select the new item
            setSelected( d->m_resources.indexOf(res) );
        }

        // unselect the more item
        setSelected( d->m_resources.count(), false );
    }
    else if( index < d->m_resources.count() ) {
        Resource res = d->m_resources[index];
        if ( selectionMode() == MatchOne ) {
            if ( d->m_selectedResources.contains( res ) && !selected ) {
                clearSelection();
            }
            else if ( selected ) {
                d->m_selectedResources.clear();
                d->m_selectedResources.insert( res );
            }
        }
        else if ( selected ) {
            d->m_selectedResources.insert( res );
        }
        else {
            d->m_selectedResources.remove( res );
        }
    }
    setSelectionChanged();
    setTermChanged();
}


void Nepomuk::Utils::DynamicResourceFacet::clearSelection()
{
    d->m_selectedResources.clear();
    if( selectionMode() == MatchOne && !d->m_resources.isEmpty() )
        d->m_selectedResources.insert(d->m_resources.first());
    setSelectionChanged();
    setTermChanged();
}


bool Nepomuk::Utils::DynamicResourceFacet::selectFromTerm( const Nepomuk::Query::Term& term )
{
    if ( term.isComparisonTerm() &&
         term.toComparisonTerm().subTerm().isResourceTerm() &&
         term.toComparisonTerm().subTerm().toResourceTerm().resource().hasType( d->resourceType() ) ) {
        d->addResource( term.toComparisonTerm().subTerm().toResourceTerm().resource() );
        setSelected( term.toComparisonTerm().subTerm().toResourceTerm().resource() );
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk::Utils::DynamicResourceFacet::handleClientQueryChange()
{
    kDebug();
    d->rebuild( false );
}


Nepomuk::Query::Query Nepomuk::Utils::DynamicResourceFacet::resourceQuery( const Query::Query& clientQuery ) const
{
    // we only select resources that make sense with the current query, ie. those that would actually
    // change the current result set
    Query::ComparisonTerm clientQueryRestrictionTerm( d->m_property, clientQuery.term() );
    clientQueryRestrictionTerm.setInverted(true);

    // we sort the resources by usage
    Nepomuk::Query::ComparisonTerm term( d->m_property, Nepomuk::Query::Term() );
    term.setSortWeight( 1, Qt::DescendingOrder );
    term.setAggregateFunction( Nepomuk::Query::ComparisonTerm::Count );
    term.setInverted(true);

    return Query::Query( Query::ResourceTypeTerm( resourceType() ) && clientQueryRestrictionTerm && term );
}


Nepomuk::Query::Term Nepomuk::Utils::DynamicResourceFacet::termForResource( const Resource& res ) const
{
    return d->m_property == Query::ResourceTerm( res );
}


QList<Nepomuk::Resource> Nepomuk::Utils::DynamicResourceFacet::getMoreResources() const
{
    return SearchWidget::searchResources( 0, resourceQuery( d->m_currentQuery ), SearchWidget::NoConfigFlags );
}

#include "dynamicresourcefacet.moc"
