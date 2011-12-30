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

#include "facet.h"
#include "simplefacet.h"
#include "dynamicresourcefacet.h"
#include "datefacet.h"
#include "typefacet.h"

#include <Soprano/Vocabulary/NAO>

#include "standardqueries.h"
#include "andterm.h"
#include "orterm.h"
#include "comparisonterm.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "resourcetypeterm.h"
#include "query.h"

#include "tag.h"
#include "property.h"

#include "tmo.h"
#include "nmo.h"
#include "nco.h"
#include "nfo.h"
#include "nie.h"

#include "kguiitem.h"
#include "kdebug.h"

using namespace Nepomuk::Query;


class Nepomuk::Utils::Facet::FacetPrivate
{
public:
    Query::Query m_clientQuery;
};


Nepomuk::Utils::Facet::Facet( QObject* parent )
    : QObject( parent ),
      d(new FacetPrivate())
{
}


Nepomuk::Utils::Facet::~Facet()
{
    delete d;
}


QString Nepomuk::Utils::Facet::text( int index ) const
{
    return guiItem( index ).text();
}


KGuiItem Nepomuk::Utils::Facet::guiItem( int index ) const
{
    Q_UNUSED(index);
    return KGuiItem();
}


void Nepomuk::Utils::Facet::setQueryTermChanged()
{
    emit queryTermChanged( this, queryTerm() );
}


void Nepomuk::Utils::Facet::setLayoutChanged()
{
    emit layoutChanged( this );
}


void Nepomuk::Utils::Facet::setSelectionChanged()
{
    emit selectionChanged( this );
}


void Nepomuk::Utils::Facet::setClientQuery( const Nepomuk::Query::Query& query )
{
    kDebug() << query;
    if( query != d->m_clientQuery ) {
        d->m_clientQuery = query;
        handleClientQueryChange();
    }
}


Nepomuk::Query::Query Nepomuk::Utils::Facet::clientQuery() const
{
    return d->m_clientQuery;
}


void Nepomuk::Utils::Facet::handleClientQueryChange()
{
    // do nothing
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createFileTypeFacet( QObject* parent )
{
    SimpleFacet* facetTypes = new SimpleFacet( parent );
    facetTypes->setSelectionMode( Facet::MatchAny );
    facetTypes->addTerm( i18nc("@option:check A filter on file type", "Documents"),
                         ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Document()) );

    // need to check the mimetype as well since strigi is still not perfect
    facetTypes->addTerm( i18nc("@option:check A filter on file type - audio files", "Audio"),
                         ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), LiteralTerm(QLatin1String("audio"))) );
    facetTypes->addTerm( i18nc("@option:check A filter on file type - media video", "Video"),
                         ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), LiteralTerm(QLatin1String("video"))) );

    facetTypes->addTerm( i18nc("@option:check A filter on file type", "Images"),
                         ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Image()) );
    return facetTypes;
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createTypeFacet( QObject* parent )
{
    return new TypeFacet( parent );
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createDateFacet( QObject* parent )
{
    return new DateFacet( parent );
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createTagFacet( QObject* parent )
{
    DynamicResourceFacet* facetTags = new DynamicResourceFacet( parent );
    facetTags->setSelectionMode( Facet::MatchAll );
    facetTags->setRelation( Soprano::Vocabulary::NAO::hasTag() );
    facetTags->setResourceType( Soprano::Vocabulary::NAO::Tag() );
    return facetTags;
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createPriorityFacet( QObject* parent )
{
    SimpleFacet* priorityFacet = new SimpleFacet( parent );
    priorityFacet->setSelectionMode(Facet::MatchOne);
    priorityFacet->addTerm( i18nc("@option:radio A filter on prioritizing/sorting a selection of resources", "No priority"), Term() );
    priorityFacet->addTerm( i18nc("@option:radio A filter on prioritizing/sorting a selection of resources", "Last modified"), standardQuery( LastModifiedFilesQuery ).term() );
    priorityFacet->addTerm( i18nc("@option:radio A filter on prioritizing/sorting a selection of resources", "Most important"), standardQuery( MostImportantResourcesQuery ).term() );
    priorityFacet->addTerm( i18nc("@option:radio A filter on prioritizing/sorting a selection of resources", "Never opened"), standardQuery( NeverOpenedFilesQuery ).term() );
    return priorityFacet;
}


// static
Nepomuk::Utils::Facet* Nepomuk::Utils::Facet::createRatingFacet( QObject* parent )
{
    SimpleFacet* facet = new SimpleFacet(parent);
    facet->setSelectionMode(Facet::MatchOne);
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "Any Rating"), Term() );
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "1 or more"), Soprano::Vocabulary::NAO::numericRating() >= LiteralTerm( 1.5 ) );
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "2 or more"), Soprano::Vocabulary::NAO::numericRating() >= LiteralTerm( 3.5 ) );
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "3 or more"), Soprano::Vocabulary::NAO::numericRating() >= LiteralTerm( 5.5 ) );
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "4 or more"), Soprano::Vocabulary::NAO::numericRating() >= LiteralTerm( 7.5 ) );
    facet->addTerm( i18nc("@option:radio A filter on the rating of a resource", "Max Rating"), Soprano::Vocabulary::NAO::numericRating() >= LiteralTerm( 9.5 ) );
    return facet;
}

#include "facet.moc"
