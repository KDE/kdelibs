/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_SEARCH_QUERY_PARSER_H_
#define _NEPOMUK_SEARCH_QUERY_PARSER_H_

#include "query.h"

#include <QtCore/QString>

#include "nepomukquery_export.h"


namespace Nepomuk {
    namespace Query {
        /**
         * \class QueryParser queryparser.h Nepomuk/Query/QueryParser
         *
         * \brief Parser for desktop user queries.
         *
         * The QueryParser can be used to parse queries and convert them into
         * Query instances.
         *
         * The syntax is fairly simple: plain strings match to LiteralTerm terms,
         * URIs need to be N3-encoded, when using white space parenthesis need to
         * be put around properties and values, terms can be excluded via \p '-'.
         *
         * \section queryparser_examples Examples
         *
         * %Query everything that contains the term "nepomuk":
         * \code
         * nepomuk
         * \endcode
         *
         * %Query everything that contains both the terms "Hello" and "World":
         * \code
         * Hello World
         * \endcode
         *
         * %Query everything that contains the term "Hello World":
         * \code
         * "Hello World"
         * \endcode
         *
         * %Query everything that has a tag whose label matches "nepomuk" (actually
         * this is where Query::resolveProperties would match "hastag" to nao:hasTag):
         * \code
         * hastag:nepomuk
         * \endcode
         *
         * %Query everything that has a tag labeled "nepomuk" or a tag labeled "scribo:
         * \code
         * hastag:nepomuk OR hastag:scribo
         * \endcode
         *
         * %Query everything that has a tag labeled "nepomuk" but not a tag labeled "scribo":
         * \code
         * +hastag:nepomuk AND -hastag:scribo
         * \endcode
         *
         * The parser can handle one special case of nesting (TODO: implement a "real" parser)
         * which matches all resources that are related to a resource which in turn has a certain
         * property:
         * \code
         * related:(hastag:nepomuk)
         * \endcode
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT QueryParser
        {
        public:
            /**
             * Create a new query parser.
             */
            QueryParser();

            /**
             * Destructor
             */
            ~QueryParser();

            /**
             * Parse a user query.
             *
             * \return The parsed query or an invalid Query object
             * in case the parsing failed.
             */
            Query parse( const QString& query ) const;

            /**
             * Try to match a field name as used in a query string to actual
             * properties.
             *
             * The matching is cached inside the Query instance for fast
             * subsequent lookups.
             *
             * Example:
             * \code
             * hastag:nepomuk
             * \endcode
             *
             * \return A list of properties that match \p fieldName or an empty
             * list in case nothing was matched.
             *
             * This method is used by parse() to match properties used in user queries.
             */
            QList<Types::Property> matchProperty( const QString& fieldName ) const;

            /**
             * Convenience method to quickly parse a query without creating an object.
             *
             * \return The parsed query or an invalid Query object
             * in case the parsing failed.
             */
            static Query parseQuery( const QString& query );

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif
