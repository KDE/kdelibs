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

#include "standardqueries.h"
#include "filequery.h"
#include "comparisonterm.h"
#include "literalterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"

#include "nie.h"
#include "nfo.h"
#include "nie.h"
#include "nuao.h"

#include <Soprano/Vocabulary/NAO>

#include <QtCore/QDate>


Nepomuk::Query::Query Nepomuk::Query::standardQuery( StandardQuery query, const Term& /*subterm*/ )
{
    switch( query ) {
    case LastModifiedFilesQuery: {
        ComparisonTerm lastModifiedTerm( Nepomuk::Vocabulary::NIE::lastModified(), Term() );
        lastModifiedTerm.setSortWeight( 1, Qt::DescendingOrder );
        FileQuery lastModifiedQuery( lastModifiedTerm );
        return lastModifiedQuery;
    }

    case MostImportantResourcesQuery: {
        ComparisonTerm fancyTerm( Soprano::Vocabulary::NAO::score(), Term() );
        fancyTerm.setSortWeight( 1, Qt::DescendingOrder );
        Query fancyQuery( fancyTerm );
        return fancyQuery;
    }

    case NeverOpenedFilesQuery: {
        // there are two ways a usage count of 0 can be expressed:
        // 1. property with value 0
        // 2. no property at all
        OrTerm usageCntTerm(
            ComparisonTerm(
                Nepomuk::Vocabulary::NUAO::usageCount(),
                LiteralTerm( 0 ),
                ComparisonTerm::Equal ),
            NegationTerm::negateTerm(
                ComparisonTerm(
                    Nepomuk::Vocabulary::NUAO::usageCount(),
                    Term() ) ) );

        // Before we had the data management service there was no usage count
        // tracking. Thus, in order not to return all files we should filter
        // out all files that were created before we started tracking usage.
        // However, by default we only show the top 10 results. Thus, in the
        // worst case this query will return the same as lastModifiedFilesQuery().
        ComparisonTerm modDateTerm(
            Nepomuk::Vocabulary::NIE::lastModified(),
            Term() );
        modDateTerm.setSortWeight( 1, Qt::DescendingOrder );

        FileQuery query( usageCntTerm && modDateTerm );
        return query;
    }

    case ResourcesForActivityQuery: {
        // FIXME
        // get all resources that have some prop defined in a graph which was created in the requested activity?
        // select distinct ?r where { graph ?g { ?r ?p ?o . } . ?g <activity> <A> . }
        // it would be something like: MetaDataTerm( <activity>, <A> )
    }
    }

    return Query();
}


Nepomuk::Query::Query Nepomuk::Query::dateRangeQuery( const QDate& start, const QDate& end, DateRangeFlags dateFlags )
{
    // create our range
    const LiteralTerm dateFrom( QDateTime( start, QTime( 0,0,0 ) ) );
    const LiteralTerm dateTo( QDateTime( end, QTime( 23, 59, 59, 999 ) ) );

    Query query;

    if( dateFlags & ModificationDate ) {
        // include files modified in our date range
        ComparisonTerm lastModifiedStart = Nepomuk::Vocabulary::NIE::lastModified() > dateFrom;
        ComparisonTerm lastModifiedEnd = Nepomuk::Vocabulary::NIE::lastModified() < dateTo;
        if( start.isValid() && end.isValid() )
            query = query || ( lastModifiedStart && lastModifiedEnd );
        else if( start.isValid() )
            query = query || lastModifiedStart;
        else if( end.isValid() )
            query = query || lastModifiedEnd;
    }

    if( dateFlags & ContentDate ) {
        // include files created (as in photos taken) in our data range
        ComparisonTerm contentCreatedStart = Nepomuk::Vocabulary::NIE::contentCreated() > dateFrom;
        ComparisonTerm contentCreatedEnd = Nepomuk::Vocabulary::NIE::contentCreated() < dateTo;
        if( start.isValid() && end.isValid() )
            query = query || ( contentCreatedStart && contentCreatedEnd );
        else if( start.isValid() )
            query = query || contentCreatedStart;
        else if( end.isValid() )
            query = query || contentCreatedEnd;
    }

    if( dateFlags & UsageDate ) {
        // include files opened (and optionally modified) in our date range
        // TODO: also take the end of the event into account
        ComparisonTerm accessEventStart = Nepomuk::Vocabulary::NUAO::start() > dateFrom;
        ComparisonTerm accessEventEnd = Nepomuk::Vocabulary::NUAO::start() < dateTo;
        ComparisonTerm accessEventCondition( Nepomuk::Vocabulary::NUAO::involves(), Term() );
        if( start.isValid() && end.isValid() )
            accessEventCondition.setSubTerm( accessEventStart && accessEventEnd );
        else if( start.isValid() )
            accessEventCondition.setSubTerm( accessEventStart );
        else if( end.isValid() )
            accessEventCondition.setSubTerm( accessEventEnd );
        if( accessEventCondition.subTerm().isValid() )
            query = query || accessEventCondition.inverted();
    }

    return query;
}
