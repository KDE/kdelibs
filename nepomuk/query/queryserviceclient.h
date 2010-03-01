/*
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_SERVICE_CLIENT_H_
#define _NEPOMUK_QUERY_SERVICE_CLIENT_H_

#include <QtCore/QObject>

#include "property.h"
#include "query.h"

#include "nepomukquery_export.h"

class QUrl;

namespace Nepomuk {
    namespace Query {

        class Result;

        /**
         * \class QueryServiceClient queryserviceclient.h Nepomuk/Query/QueryServiceClient
         *
         * \brief Convenience frontend to the %Nepomuk Query DBus Service
         *
         * The QueryServiceClient provides an easy way to access the %Nepomuk Query Service
         * without having to deal with any communication details. By default it monitors
         * queries for changes.
         *
         * Usage is simple: Create an instance of the client for each search you want to
         * track. One instance may also be reused for subsequent queries if further updates
         * of the persistent query are not necessary.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT QueryServiceClient : public QObject
        {
            Q_OBJECT

        public:
            /**
             * Create a new QueryServiceClient instance.
             */
            QueryServiceClient( QObject* parent = 0 );

            /**
             * Desctructor. Closes the query.
             */
            ~QueryServiceClient();

            /**
             * \brief Check if the Nepomuk query service is running.
             * \return \p true if the Nepomuk query service is running and could
             * be contacted via DBus, \p false otherwise
             */
            static bool serviceAvailable();

        public Q_SLOTS:
            /**
             * Start a query using the Nepomuk query service.
             *
             * Results will be reported via newEntries. All results
             * have been reported once finishedListing has been emitted.
             *
             * \param query the query to perform.
             *
             * \return \p true if the query service was found and the query
             * was started. \p false otherwise.
             */
            bool query( const Query& query );

            /**
             * \overload
             *
             * \param query a SPARQL query which binds results to variable \p 'r'.
             * \param requestPropertyMap An optional mapping of variable binding names in \p query
             * to their corresponding properties. These bindings will then be reported via
             * Result::requestProperties(). This map will be constructed automatically when using
             * query(const Query&).
             *
             * \section sparql_query_examples Examples:
             *
             * Select a simple request property as can also be done via Query::Query:
             *
             * \code
             * QString query = "select ?r ?mtime where { "
             *                 "?r a nfo:FileDataObject . "
             *                 "?r nie:lastModified ?mtime . "
             *                 "}";
             *
             * QueryServiceClient::RequestPropertyMap requestPropertyMap;
             * requestPropertyMap.insert( "mtime", Vocabulary::NIE::lastModified() );
             *
             * sparqlQuery( query, requestPropertyMap );
             * \endcode
             *
             * This will report the resources themselves and their modification time in the result's request
             * properties:
             *
             * \code
             * void handleResult( const Query::Result& results ) {
             *     QDateTime lastModified = result.requestProperty( Vocabulary::NIE::lastModified() ).toDateTime();
             *     [...]
             * \endcode
             *
             * While using Query::Query restricts to request properties to diret properties of the
             * results using a custom SPARQL query allows to use any binding as request property.
             * The used property URI in the mapping does not even need to match anything in the query:
             *
             * \code
             * QString query = "select ?r ?phone where { "
             *                 "?r a nco:PersonContact . "
             *                 "?r nco:hasPhoneNumber ?n . "
             *                 "?n nco:phoneNumber ?phone . "
             *                 "}";
             *
             * QueryServiceClient::RequestPropertyMap requestPropertyMap;
             * requestPropertyMap.insert( "phone", Vocabulary::NCO::hasPhoneNumber() );
             * \endcode
             *
             * \sa Query::requestPropertyMap()
             */
            bool sparqlQuery( const QString& query, const Nepomuk::Query::RequestPropertyMap& requestPropertyMap = Nepomuk::Query::RequestPropertyMap() );

            /**
             * \overload
             *
             * \param query a desktop query string which can be parsed by QueryParser.
             */
            bool desktopQuery( const QString& query );

            /**
             * Start a query using the Nepomuk query service.
             *
             * Results will be reported as with query(const QString&)
             * but a local event loop will be started to block the method
             * call until all results have been listed.
             *
             * The client will be closed after the initial listing. Thus,
             * changes to results will not be reported as it is the case
             * with the non-blocking methods.
             *
             * \param query the query to perform.
             *
             * \return \p true if the query service was found and the query
             * was started. \p false otherwise.
             *
             * \sa query(const Query&), close()
             */
            bool blockingQuery( const Query& query );

            /**
             * \overload
             *
             * \param query a SPARQL query which binds results to variable \p 'r'.
             * \param requestPropertyMap An optional mapping of variable binding names in \p query
             * to their corresponding properties. For details see sparqlQuery.
             *
             * \sa sparqlQuery(const Query&)
             */
            bool blockingSparqlQuery( const QString& query, const Nepomuk::Query::RequestPropertyMap& requestPropertyMap = Nepomuk::Query::RequestPropertyMap() );

            /**
             * \overload
             *
             * \param query a desktop query string which can be parsed by QueryParser.
             *
             * \sa desktopQuery(const QString&), close()
             */
            bool blockingDesktopQuery( const QString& query );

            /**
             * Close the client, thus stop to monitor the query
             * for changes. Without closing the client it will continue
             * signalling changes to the results.
             *
             * This will also make any blockingQuery return immediately.
             */
            void close();

        Q_SIGNALS:
            /**
             * Emitted for new search results. This signal is emitted both
             * for the initial listing and for changes to the search.
             */
            void newEntries( const QList<Nepomuk::Query::Result>& entries );

            /**
             * Emitted if the search results changed when monitoring a query.
             * \param entries A list of resource URIs identifying the resources
             * that dropped out of the query results.
             */
            void entriesRemoved( const QList<QUrl>& entries );

            /**
             * Emitted when the initial listing has been finished, ie. if all
             * results have been reported via newEntries. If no further updates
             * are necessary the client should be closed now.
             */
            void finishedListing();

        private:
            class Private;
            Private* const d;

            Q_PRIVATE_SLOT( d, void _k_entriesRemoved( const QStringList& ) )
            Q_PRIVATE_SLOT( d, void _k_finishedListing() )
        };
    }
}

#endif
