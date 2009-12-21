/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_QUERY_H_
#define _NEPOMUK_QUERY_QUERY_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QDebug>

#include <kurl.h>

#include "property.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {

        class Term;
        class QueryPrivate;

        /**
         * \brief Convinience definition for request property mappings as used in
         * QueryServiceClient::sparqlQuery() and QueryServiceClient::blockingSparqlQuery().
         */
        typedef QHash<QString, Nepomuk::Types::Property> RequestPropertyMap;

        /**
         * \class Query query.h Nepomuk/Query/Query
         *
         * \brief A Nepomuk desktop query.
         *
         * A Query consists or a combination of Terms that can be optionally
         * restricted via a limit (setLimit()) or folder filters (setIncludeFolders() and
         * setExcludeFolders()).
         *
         * Additionally setRequestProperties() allows to retrieve additional information
         * about the results.
         *
         * Queries can be easily converted to SPARQL via the toSparqlQuery() method or to
         * search URLs ready for KIO::DirLister using toSearchUrl().
         *
         * The typical usage of a Query instance would be QueryServiceClient::query().
         * However one can also use toSparqlQuery() to convert it into a SPARQL query
         * string and use that string in QueryServiceClient::sparqlQuery() or even
         * directly in Soprano::Model::executeQuery() via \p ResourceManager::instance()->mainModel()
         * or in a custom Model.
         *
         * \sa QueryParser
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT Query
        {
        public:
            /**
             * Create an empty invalid query object.
             */
            Query();

            /**
             * Create a query of type PlainQuery based on
             * \a term.
             */
            explicit Query( const Term& term );

            /**
             * Copy constructor.
             */
            Query( const Query& );

            /**
             * Destructor
             */
            ~Query();

            /**
             * Assignment operator
             */
            Query& operator=( const Query& );

            /**
             * \return \p true if the query is valid, i.e.
             * it has a valid term().
             */
            bool isValid() const;

            /**
             * The root term of the query.
             * This can be any type of term.
             *
             * \sa setTerm
             */
            Term term() const;

            /**
             * The maximum number of results that this query should yield.
             *
             * \sa setLimit
             */
            int limit() const;

            /**
             * Set the root term of the query.
             *
             * \sa term
             */
            void setTerm( const Term& );

            /**
             * Set the maximum number of results this query
             * should yield.
             *
             * \sa limit
             */
            void setLimit( int );

            /**
             * \class RequestProperty query.h Nepomuk/Query/Query
             *
             * \brief A request property can be added to a Query to retrieve
             * additional information about the results.
             *
             * Normally a query would simply yield a list of resources,
             * ie. URIs. Using RequestProperty one can request additional
             * fields such as the modification time or the label or whatever
             * is of interest in the current context.
             *
             * \sa addRequestProperty, setRequestProperties
             */
            class NEPOMUKQUERY_EXPORT RequestProperty {
            public:
                /**
                 * Create a new request property.
                 * \param property The requested %property.
                 * \param optional if \p true the requested property will
                 * be %optional, ie. marked as OPTIONAL in SPARQL queries.
                 */
                RequestProperty( const Nepomuk::Types::Property& property,
                                 bool optional = true );

                /**
                 * Copy constructor.
                 */
                RequestProperty( const RequestProperty& );

                /**
                 * Destructor.
                 */
                ~RequestProperty();

                /**
                 * Copy operator.
                 */
                RequestProperty& operator=( const RequestProperty& );

                /**
                 * Comparison operator. Two RequestProperty instances
                 * are equal if their property() and value of optional()
                 * are equal.
                 */
                bool operator==( const RequestProperty& other ) const;

                /**
                 * \return The property set in the constructor.
                 */
                Nepomuk::Types::Property property() const;

                /**
                 * \return The optional value set in the constructor.
                 */
                bool optional() const;

            private:
                class Private;
                QSharedDataPointer<Private> d;
            };

            /**
             * Add a property that should be reported with each search result.
             *
             * \param property The requested property.
             *
             * \sa setRequestProperties, requestProperties
             */
            void addRequestProperty( const RequestProperty& property );

            /**
             * Set the properties that should be reported with each search result.
             *
             * \param properties The requested properties.
             *
             * \sa addRequestProperty, requestProperties
             */
            void setRequestProperties( const QList<RequestProperty>& properties );

            /**
             * \return The list of RequestProperty instances set via addRequestProperty
             * and setRequestProperties.
             */
            QList<RequestProperty> requestProperties() const;

            /**
             * \brief Aditional flags modifying the behaviour of toSparqlQuery() and toSearchUrl().
             */
            enum SparqlFlag {
                /**
                 * No flags, i.e. create a default query.
                 */
                NoFlags = 0x0,

                /**
                 * Create a SPARQL count query which will return the number
                 * of results rather than the results themselves. Be aware that
                 * with a count query any requestProperties() will result in a grouping,
                 * i.e. multiple count values will be returned with respect to the request
                 * properties.
                 */
                CreateCountQuery = 0x1,

                /**
                 * Automatically handle inverse properties, consider for example nie:isPartOf
                 * and nie:hasPart at the same time and even if only one of both is
                 * defined.
                 */
                HandleInverseProperties = 0x2
            };
            Q_DECLARE_FLAGS( SparqlFlags, SparqlFlag )

            /**
             * Convert the query into a SPARQL query which can be used with the
             * Nepomuk query service or directly in Soprano::Model::executeQuery.
             *
             * The resulting query will bind the results to variable \p 'r'. Request
             * properties will be bound to variables \p 'reqProp1' through \p 'reqPropN'.
             *
             * \warning The SPARQL queries created by this method contain SPARQL extensions
             * from Virtuoso and will not work with other RDF storage solutions!
             *
             * \param flags Optional flags to change the query.
             *
             * \return The SPARQL representation of this query or an empty string
             * if the query could not be converted (invalid query.)
             *
             * \sa toSearchUrl(), SparqlFlag
             */
            QString toSparqlQuery( SparqlFlags flags = NoFlags ) const;

            /**
             * Convert the query into a URL which can be listed using KIO::DirLister.
             * The URL will use the \p nepomuksearch:/ KIO protocol to handle the listing
             * of search results.
             *
             * This is the perfect method for listing results in file managers or file
             * dialogs.
             *
             * \param flags Optional flags to change the query.  Query::CreateCountQuery is not
             * supported and will silently be dropped from \p flags.
             *
             * \return A URL which will list a virtual folder containing all search results
             * from this query or an invalid URL in case this query is invalid.
             *
             * \sa toSparqlQuery(), SparqlFlag
             */
            KUrl toSearchUrl( SparqlFlags flags = NoFlags ) const;

            /**
             * Build a request property map as used in QueryServiceClient::sparqlQuery()
             * from the request properties set via addRequestProperty() and setRequestProperties().
             *
             * Be aware that in most situations it is much simpler to use QueryServiceClient::query().
             */
            RequestPropertyMap requestPropertyMap() const;

            /**
             * Comparison operator.
             *
             * \return \p true if this query is equal to \p query.
             */
            bool operator==( const Query& query ) const;

        protected:
            /** \cond protected_error_members */
            QSharedDataPointer<QueryPrivate> d;

            friend class QueryParser;
            /** \endcond */
        };

        NEPOMUKQUERY_EXPORT uint qHash( const Nepomuk::Query::Query& );
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Query::Query::SparqlFlags )

NEPOMUKQUERY_EXPORT QDebug operator<<( QDebug, const Nepomuk::Query::Query& );

#endif
