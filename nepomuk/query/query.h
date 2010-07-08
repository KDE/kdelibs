/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

class QTextStream;

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
         * \sa QueryParser, FileQuery
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
             * Create a query with root term \a term.
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
             * Assignment operator. Assigns \p term to
             * this query without changing any other
             * properties.
             *
             * \since 4.5
             */
            Query& operator=( const Term& term );

            /**
             * \return \p true if the query is valid, i.e.
             * it has a valid term().
             */
            bool isValid() const;

            /**
             * \return \p true if this is a file query that will
             * only return files and folders.
             *
             * \sa FileQuery
             *
             * \since 4.5
             */
            bool isFileQuery() const;

            /**
             * The root term of the query.
             * This can be any type of term.
             *
             * \sa setTerm()
             */
            Term term() const;

            /**
             * The maximum number of results that this query should yield.
             *
             * \sa setLimit()
             */
            int limit() const;

            /**
             * The first result that should be retrieved.
             *
             * \sa setOffset()
             *
             * \since 4.5
             */
            int offset() const;

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
             * The first result that should be retrieved. This can be combined
             * with setLimit() to do paged results.
             *
             * \since 4.5
             */
            void setOffset( int offset );

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
             * ComparisonTerm::setVariableName() provides a more flexible (but also
             * slightly more complicated) way to select additional results.
             *
             * \sa setRequestProperties, requestProperties, ComparisonTerm::setVariableName()
             */
            void addRequestProperty( const RequestProperty& property );

            /**
             * Set the properties that should be reported with each search result.
             *
             * \param properties The requested properties.
             *
             * ComparisonTerm::setVariableName() provides a more flexible (but also
             * slightly more complicated) way to select additional results.
             *
             * \sa addRequestProperty, requestProperties, ComparisonTerm::setVariableName()
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
                 * of results rather than the results themselves. The resuling query will have
                 * a single binding called \p 'cnt'.
                 */
                CreateCountQuery = 0x1,

                /**
                 * Automatically handle inverse properties, consider for example nie:isPartOf
                 * and nie:hasPart at the same time and even if only one of both is
                 * defined.
                 */
                HandleInverseProperties = 0x2,

                /**
                 * Disable the return of scores for full text matching. This flag is added
                 * automatically with CreateCountQuery.
                 *
                 * \since 4.6
                 */
                WithoutScoring = 0x4

            };
            Q_DECLARE_FLAGS( SparqlFlags, SparqlFlag )

            /**
             * Convert the query into a SPARQL query which can be used with the
             * Nepomuk query service or directly in Soprano::Model::executeQuery.
             *
             * The resulting query will bind the results to variable \p 'r'. Request
             * properties will be bound to variables \p 'reqProp1' through \p 'reqPropN'
             * (the only exception is a count query created via the CreateCountQuery flag).
             *
             * If you are looking for a serialization of a Query which can be parsed again
             * use toString() instead.
             *
             * \warning The SPARQL queries created by this method contain SPARQL extensions
             * from Virtuoso and will not work with other RDF storage solutions!
             *
             * \param flags Optional flags to change the query.
             *
             * \return The SPARQL representation of this query or an empty string
             * if the query could not be converted (invalid query.)
             *
             * \sa toString(), toSearchUrl(), SparqlFlag
             */
            QString toSparqlQuery( SparqlFlags flags = NoFlags ) const;

            /**
             * Convert the query into a URL which can be listed using KIO::DirLister.
             * The URL will use the \p nepomuksearch:/ KIO protocol to handle the listing
             * of search results.
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
             * Convert the query into a URL which can be listed using KIO::DirLister.
             * The URL will use the \p nepomuksearch:/ KIO protocol to handle the listing
             * of search results.
             *
             * This is the perfect method for listing results in file managers or file
             * dialogs.
             *
             * \param customTitle An optional custom title that will be used for the listing
             * of the results. This is achieved by setting the KIO::UDSEntry::UDS_DISPLAY_NAME to
             * the customTitle value.
             * \param flags Optional flags to change the query.  Query::CreateCountQuery is not
             * supported and will silently be dropped from \p flags.
             *
             * \return A URL which will list a virtual folder containing all search results
             * from this query or an invalid URL in case this query is invalid.
             *
             * \sa toSparqlQuery(), SparqlFlag
             *
             * \since 4.5
             */
            KUrl toSearchUrl( const QString& customTitle, SparqlFlags flags = NoFlags ) const;

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

            /**
             * Encode the Query in a string. Be aware that this does NOT create a SPARQL
             * query. The returned string can be used to serialize queries that can later
             * be read via fromString().
             *
             * \sa fromString(), toSparqlQuery()
             *
             * \since 4.5
             */
            QString toString() const;

            /**
             * Parse a Query that has been encoded as a string via toString().
             *
             * \warning This method can NOT parse SPARQL syntax.
             *
             * \sa toString()
             *
             * \since 4.5
             */
            static Query fromString( const QString& queryString );

            /**
             * Extract a query from a nepomuksearch:/ query URL.
             *
             * \return The query that was encoded in \p url or an invalid query if
             * either \p url is not a nepomuksearch:/ URL or if it contains a pure SPARQL
             * query. In the latter case sparqlFromQueryUrl() can be used to extract that
             * query.
             *
             * \sa sparqlFromQueryUrl()
             *
             * \since 4.5
             */
            static Query fromQueryUrl( const KUrl& url );

            /**
             * Extract the SPARQL query from a nepomuksearch:/ query URL. All kinds of
             * nepomuksearch:/ URLs are supported.
             *
             * \return The SPARQL query string representing the query encoded
             * in \p url.
             *
             * \sa fromQueryUrl()
             *
             * \since 4.5
             */
            static QString sparqlFromQueryUrl( const KUrl& url );

            /**
             * Extact the title from a nepomuksearch:/ query URL. The title
             * is either a custom title which has been specified in
             * toSearchUrl(const QString&, SparqlFlags) or the user query string
             * in case of nepomuksearch:/myquery URLs.
             *
             * \return A title for the query \p url or an empty string
             * in case \p url is not a nepomuksearch:/ URL or a useful title
             * cannot be extracted.
             *
             * \since 4.5
             */
            static QString titleFromQueryUrl( const KUrl& url );

        protected:
            /** \cond protected_query_members */
            QSharedDataPointer<QueryPrivate> d;

            friend class QueryParser;
            /** \endcond */
        };

        /**
         * Logical and operator which combines \p term into the term
         * of \p query to match both.
         *
         * \sa AndTerm
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Query operator&&( const Query& query, const Term& term );

        /**
         * Logical or operator which combines \p term into the term
         * of \p query to match either one.
         *
         * \sa OrTerm
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Query operator||( const Query& query, const Term& term );

        /**
         * Logical negation operator which negates the meaning of
         * a query.
         *
         * \sa NegationTerm::negateTerm()
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Query operator!( const Query& query );

        NEPOMUKQUERY_EXPORT uint qHash( const Nepomuk::Query::Query& );
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Query::Query::SparqlFlags )

NEPOMUKQUERY_EXPORT QDebug operator<<( QDebug, const Nepomuk::Query::Query& );

#endif
