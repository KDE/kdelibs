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

#ifndef _NEPOMUK_STANDARD_QUERIES_H_
#define _NEPOMUK_STANDARD_QUERIES_H_

#include "nepomukquery_export.h"
#include "term.h"

class QDate;

namespace Nepomuk {
    namespace Query {
        class Query;

        /**
         * A set of predefined queries that can be created via standardQuery().
         *
         * \since 4.6
         */
        enum StandardQuery {
            /**
             * Creates a query that returns all files sorted by descending modification date.
             *
             * The subterm parameter can be used to specify an application restricting the results
             * to files created/opened with that application.
             */
            LastModifiedFilesQuery,

            /**
             * Creates a query that returns all resources sorted by descending score (as calculated
             * by the DataMaintenanceService)
             *
             * The subterm parameter can be used to specify an application restricting the results
             * to files created/opened with that application.
             */
            MostImportantResourcesQuery,

            /**
             * Creates a query that returns all files with a usage count of 0
             * sorted by descending modification date.
             */
            NeverOpenedFilesQuery,

            /**
             * Get the resources related to a specific activity. Use a ResourceTerm referring to
             * the activity as parameter in standardQuery.
             */
            ResourcesForActivityQuery
        };


        /**
         * Modificators to influence the behaviour of dateRangeQuery().
         *
         * \since 4.6
         */
        enum DateRangeFlag {
            /**
             * Query for the modification date (nie:lastModified)
             */
            ModificationDate = 0x1,

            /**
             * Query for the content creation date (nie:contentCreated)
             */
            ContentDate = 0x2,

            /**
             * Query for usage events referring to the resource.
             */
            UsageDate = 0x4,

            /**
             * Query for all possible dates.
             */
            AllDates = ModificationDate|ContentDate|UsageDate
        };
        Q_DECLARE_FLAGS( DateRangeFlags, DateRangeFlag )

        /**
         * Create a standard query as defined by \p query.
         *
         * \param query The query to be generated. See StandardQuery.
         * \param subterm An optional subterm used for specific types of standard queries that need
         * a parameter like ResourcesForActivityQuery.
         *
         * To get a query that only returns files (this is already true for some of the predefined queries)
         * use something like the following:
         *
         * \code
         * Query::FileQuery query = Query::standardQuery( Query::LastModifiedFilesQuery );
         * \endcode
         *
         * Be aware that queries can be combined. One can for example get the most important files related
         * to an activity as follows:
         *
         * \code
         * Query query = Query::standardQuery( Query::ResourcesForActivityQuery, myActivity )
         *     && Query::standardQuery( Query::MostImportantResourcesQuery );
         * \endcode
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Query standardQuery( StandardQuery query, const Term& subterm = Term() );

        /**
         * Create a query that returns resources/files that have been modified/accessed in the range
         * from \p start to \p end (including both full days). The flags specified in \p dateFlags can be used to influence the
         * type of dates that are queried.
         *
         * \param start The start date of the range, if invalid no start is used, i.e. everything before \p end matches.
         * \param end The end date of the range, if invalid no end is used, i.e. everything after \p start matches.
         * \param dateFlags Optional flags to influence the final query.
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Query dateRangeQuery( const QDate& start, const QDate& end, DateRangeFlags dateFlags = AllDates );
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Query::DateRangeFlags )

#endif
