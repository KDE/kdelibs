/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _NEPMUK_BACKBONE_RDF_REPOSITORY_PUBLISHER_H_
#define _NEPMUK_BACKBONE_RDF_REPOSITORY_PUBLISHER_H_

#include "servicepublisher.h"
#include "queryresulttable.h"
#include "nepomuk_export.h"

#include <soprano/statement.h>

namespace Nepomuk {
    namespace RDF {
	class QueryResultTable;
    }

    namespace Services {
	/**
	 * \brief ServicePublisher base class for service implementations publishing
	 * the %http://nepomuk.semanticdesktop.org/services/RDFRepository service type.
	 *
	 * \section rdfrepository The Nepomuk RDF Repository Service
	 *
	 * This is the interface to the Nepomuk desktop RDF repository. In this
	 * repository, users can store their private RDF data. The repository supports
	 * multiple repository-ids inside, each repository is a named-graph aware
	 * quadstore. The most important repository-ids are "main" for the main data
	 * repository and "config" for Nepomuk configuration data.
	 *
	 * \subsection querytimeout Non-blocking queries and timeouts
	 *
	 * The result of a query can be large. If you are accessing the RDF repository
	 * using a SOAP connection or another remote protocol, you can retrieve the
	 * results in sets, to start using the first results while waiting for more to
	 * come. The methods <code>query****</code> support this mode. You pass in a
	 * query and a timeout and the server will keep the query-results in a buffer,
	 * for you to retrieve.
	 *
	 * The \a timeout is passed in milliseconds, the server will keep the query
	 * open for this amount of milliseconds. If you call a <code>fetch***</code>
	 * method, the server will start this timeout again. Passing \a 0 as
	 * a timeout instructs the server to keep the query "as long as possible", which
	 * can be an hour or 10 minutes, that is up to the implementation.
	 *
	 * All queries can be closed prematurely using closeQuery.
	 * 
	 * \subsection querylang Query Languages
	 *
	 * The store can support one or more query languages. The following strings are
	 * used to identify them:
	 * 
	 * \li sparql - as defined by <a
	 * href="http://www.w3.org/TR/rdf-sparql-query/">W3C SPARQL</a>
	 * \li serql - <a
	 * href="http://www.openrdf.org/doc/sesame/users/ch06.html">Sesame RDF Query
	 * Language</a>
	 * \li rdql - <a href="http://www.w3.org/Submission/RDQL/">RDF Data Query
	 * Language</a>
	 * 
	 * \subsection repositoryId Repository-Ids
	 *
	 * The RDF service hosts multiple RDF databases, called "repositories". In each
	 * repository, there are separate features available (how to store the data,
	 * inference, text search support). We support two pre-configured repositories
	 * out of the box:
	 * 
	 * \li main - the ontology store containing NRL, DEO, DAO, PIMO, etc. most
	 * annotations go here. crawled resources gathered by the DataWrapper are stored
	 * here
	 * \li config - configuration data needed by services
	 * 
	 * Additional repositories can be created  and removed.
	 * 
	 * \subsection rdfmime RDF Mime/Type
	 *
	 * The format of the serialisation, possible values are
	 * 
	 * \li application/rdf+xml - rdf/xml
	 * \li application/x-turtle - Turtle
	 * \li text/rdf+n3 - N3
	 * \li application/trix - TRIX
	 * 
	 * \subsection valueser Value Serialization
	 *
	 * RDF Values are serialized according to a special magic that we defined. This
	 * is needed for passing back select results. Our serialization works like this:
	 * 
	 * \li an uri, string contains the uri. "http://www.example.com"
	 * \li a bnode, string contains the bnode-id in brackets. "(node123123)"
	 * \li a literal, string contains the literal in hyphens: ""a literal""
	 *
	 * If you wonder why we didn't use structured types: the reason is because the
	 * serialization is already taking much space using SOAP, so keeping RDF values
	 * in strings is a fallback.
	 */
	class NEPOMUK_MIDDLEWARE_EXPORT RDFRepositoryPublisher : public Middleware::ServicePublisher
	    {
		Q_OBJECT

	    public:
                /**
                 * Creates a new RDFRepositoryPublisher instance.
                 * \param name The name of the service, for informational purposes only.
                 * \param url The URL of the service, uniquely identifying it.
                 */
		RDFRepositoryPublisher( const QString& name, const QString& url );

                /**
                 * Destructor
                 */
		virtual ~RDFRepositoryPublisher();

		/**
		 * Error codes as returned by the addStatement and removeStatement methods
		 * FIXME: define useful ones in the Nepomuk standard, not only the KDE version.
		 */
		enum ErrorCodes {
		    ERROR_SUCCESS = 0,
		    ERROR_FAILURE = -1
		};

	    public Q_SLOTS:
		/**
		 * Create a new repository.
		 *
		 * \param repositoryId The id/name of the new repository.
		 *
		 * \return An error code as defined by ErrorCodes
		 */
		virtual void createRepository( const QString& repositoryId ) = 0;

		/**
		 * List all the repositories in the system.
		 * This information can also be retrieved through the default "system" repositoryId
		 * via listStatements.
		 *
		 * \return A list of all repositoryIds in the storage.
		 */
		virtual QStringList listRepositoryIds() = 0;

		/**
		 * Remove the repository identified by the \a repositoryId.
		 *
		 * Note that you cannot remove the preconfigured repositories.
		 *
		 * \param repositoryId the reference to the repository to remove.
		 *
		 * \return An error code as defined by ErrorCodes
		 */
		virtual void removeRepository( const QString& repositoryId ) = 0;

		/**
		 * Get the size of a repository, i.e. its statement count.
		 *
		 * \param repositoryId The id/name of the repository.
		 *
		 * \return The total number of statements in the repository referenced by \a repositoryId.
		 */
		virtual int getRepositorySize( const QString& repositoryId ) = 0;

		/**
		 * Check if the %RDF store contains \a statement. If the statement contains null
		 * nodes, the rest will be matched and the null nodes are wildcards.
		 *
		 * \param The name of the %RDF repository to search
		 * \param statement The statement to match.
		 *
		 * \return 1 if the repository referenced by \a repositoryId contains \a statement, 0 if 
		 *         \a statement could not be found.
		 */
		virtual int contains( const QString& repositoryId, const Soprano::Statement& statement ) = 0;

		/**
		 * Add a statement to a repository.
		 * 
		 * \param repositoryId The name of the %RDF repository to add the statement to.
		 * \param statement The new statement to add.
		 */
		virtual void addStatement( const QString& repositoryId, const Soprano::Statement& statement ) = 0;

		/**
		 * Add a list of statements to a repository.
		 * 
		 * \param repositoryId The name of the %RDF repository to add the statements to.
		 * \param statements The new statements to add.
		 */
		virtual void addStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements ) = 0;

		/**
		 * Remove a context from a repository.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param context The context to remove.
		 */
		virtual void removeContext( const QString& repositoryId, const Soprano::Node& context ) = 0;

		/**
		 * Remove a statement from a particular repository using a direct matching, i.e.
		 * null nodes will not be used as wildcards.
		 * 
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param statement The statement to remove.
		 * 
		 * \return the number of removed statements.
		 */
		virtual int removeStatement( const QString& repositoryId, const Soprano::Statement& statement ) = 0;

		/**
		 * Remove a list of statements from a particular repository using a direct matching, i.e.
		 * null nodes will not be used as wildcards.
		 * 
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param statements the statements to remove
		 * 
		 * \return the number of removed statements.
		 */
		virtual int removeStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements ) = 0;

		/**
		 * Remove all statements matching \a statement. Null nodes
		 * will be used as wildcards.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param statement the statement to remove
		 * 
		 * \return the number of removed statements.
		 */
		virtual int removeAllStatements( const QString& repositoryId, const Soprano::Statement& statement ) = 0;

		/**
		 * Retrieve a list of statements.  The passed statement
		 * serves as a query for statements that should be
		 * returned.
		 *
		 * Caution: If you expect the result to contain many
		 * triples, use queryListStatements instead.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param statement Defines the query by setting one or more of the Nodes to type NodeNull.
		 *                  The result will then contain all the statements that match the defined 
		 *                  Nodes.
		 *
		 * \return a list of statements that match the passed query.
		 */
		virtual QList<Soprano::Statement> listStatements( const QString& repositoryId, const Soprano::Statement& statement ) = 0;

		/**
		 * Execute a construct SPARQL query on a specific repository.
		 *
		 * Caution: If you expect the result to contain many
		 * triples, use queryConstructSparql instead.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param query A SPARQL construct query string.
		 *
		 * \return a list of statements matching the query.
		 */
		virtual QList<Soprano::Statement> constructSparql( const QString& repositoryId, const QString& query ) = 0;

		/**
		 * Execute a select SPARQL query on a specific repository.
		 *
		 * Caution: If you expect the result to contain many
		 * triples, use querySelectSparql instead.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param query A SPARQL select query string.
		 *
		 * \return a list of result bindings packed into a QueryResultTable.
		 */
		virtual RDF::QueryResultTable selectSparql( const QString& repositoryId, const QString& query ) = 0;
	  
		/**
		 * Run a <a href="http://www.w3.org/TR/rdf-sparql-query/#describe">sparql
		 * describe query</a>. Result is a graph, as defined in the spec.
		 * 
		 * Caution: If you expect the result to contain many
		 * triples, use queryDescribeSparql instead.
		 *
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            sparql DESCRIBE query
		 * \return the query result
		 *
		 * \sa queryDescribeSparql
		 */
		virtual QList<Soprano::Statement> describeSparql( const QString& repositoryId, const QString& query ) = 0;

		/**
		 * Execute a construct query on a specific repository.
		 *
		 * Caution: If you expect the result to contain many
		 * triples, use queryConstruct instead.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param query A construct query string.
		 * \param querylanguage The language \a query is written in. See \ref querylang.
		 *
		 * \return a list of statements matching the query.
		 */
		virtual QList<Soprano::Statement> construct( const QString& repositoryId, const QString& query,
                                                             const QString& querylanguage ) = 0;
	  
		/**
		 * Execute a select query on a specific repository.
		 *
		 * Caution: If you expect the result to contain many
		 * triples, use querySelect instead.
		 *
		 * \param repositoryId The name of the %RDF repository to search.
		 * \param query A select query string.
		 * \param querylanguage The language \a query is written in. See \ref querylang.
		 *
		 * \return a list of result bindings packed into a QueryResultTable.
		 */
		virtual RDF::QueryResultTable select( const QString& repositoryId, const QString& query,
						      const QString& querylangauge ) = 0;

		/**
		 * Retrieve a list of statements. The passed statement serves as a query for
		 * statements that should be returned. The results of this command can then
		 * be retrieved using the fetchListStatementsResults command, using the returned
		 * queryId. 
		 *
		 * \param repositoryId
		 *            id of the repository
		 * \param statement
		 *            Defines the query by setting one or more of the Nodes to type
		 *            NodeNull. The result will then contain all the statements
		 *            that match the defined Nodes.
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 *
		 * \sa fetchListStatementsResults, queryClose
		 */
		virtual int queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
						 int timeoutMSec ) = 0;

		/**
		 * Executes the given query in the given repository and returns a queryId
		 * object providing access to all found entities. Use fetchConstructResults
		 * to retrieve the results.
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            The construct query string.
		 * \param querylanguage See \ref querylang.
		 *            
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 *
		 * \sa fetchConstructResults, queryClose
		 */
		virtual int queryConstruct( const QString& repositoryId, const QString& query,
					    const QString& querylanguage, int timeoutMSec ) = 0;

		/**
		 * Executes the given query in the given repository and returns a queryId
		 * object providing access to all found entities. Use
		 * fetchSelectResults to retrieve the results.
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            The select query string.
		 * \param querylanguage See \ref querylang.
		 * 
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 *
		 * \sa fetchSelectResults, queryClose
		 */
		virtual int querySelect( const QString& repositoryId, const QString& query,
					 const QString& querylanguage, int timeoutMSec ) = 0;

		/**
		 * Executes the given query in the given repository and returns a queryId
		 * object providing access to all found entities. Use
		 * fetchConstructResults to retrieve the results. 
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            The SPARQL construct query string.
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 */
		virtual int queryConstructSparql( const QString& repositoryId, const QString& query,
						  int timeoutMSec ) = 0;

		/**
		 * Executes the given query in the given repository and returns a queryId
		 * object providing access to all found entities. Use
		 * fetchSelectResults to retrieve the results.
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            The SPARQL select query string.
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 *
		 * \sa fetchSelectResults, closeQuery
		 */
		virtual int querySelectSparql( const QString& repositoryId, const QString& query,
					       int timeoutMSec ) = 0;

		/**
		 * Run a <a href="http://www.w3.org/TR/rdf-sparql-query/#describe">sparql
		 * describe query</a>. Result is a queryId, use fetchDescribeResults to
		 * get the results
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            sparql DESCRIBE query
		 * \param timeoutMSec
		 *            timeout in milliseconds. See \ref querytimeout.
		 * \return a queryId identifying the query.
		 *
		 * \sa describeSparql, closeQuery
		 */
		virtual int queryDescribeSparql( const QString& repositoryId, const QString& query,
						 int timeoutMSec ) = 0;

		/**
		 * Run a <a href="http://www.w3.org/TR/rdf-sparql-query/#ask">sparql ASK
		 * query</a>. Result is either 1 (=true) or 0 (=false)
		 * 
		 * \param repositoryId
		 *            id of the repository
		 * \param query
		 *            sparql ASK query
		 * \return the query result 1 (=true) or 0 (=false)
		 */
		virtual int askSparql( const QString& repositoryId, const QString& query ) = 0;

		/**
		 * Close a query.
		 *
		 * If further results are not needed a client may use this method to close the query and let the
		 * repository free resources related to this query id.
		 *
		 * \param queryId the id of the query to be closed as retrieved from a call to querySelect,
		 * queryConstruct, queryListStatements, or their SPARQL counterparts.
		 */
		virtual void closeQuery( int queryId ) = 0;

		/**
		 * Return a list of statements that are the result of a listStatements query
		 * identified by the passed queryId. The size indicates how many statements
		 * should be returned.
		 * 
		 * \param queryId
		 *            the queryId to fetch.
		 * \param max the maximum size of the returned List. a value between 100 and 1000 is
		 *            recommended. Pass 0 to return all remaining statements, this can block I/O.
		 *
		 * \return a list of result statements. If the size of the returned List is smaller than
		 * the \a max, all statements have been listed and the query id will be invalidated.
		 *
		 * \sa queryListStatements, closeQuery
		 */
		virtual QList<Soprano::Statement> fetchListStatementsResults( int queryId, int max ) = 0;

		/**
		 * Return a list of statements that are the result of a CONSTRUCT query
		 * identified by the passed \a queryId.
		 * 
		 * \param queryId
		 *            the queryId to fetch.
		 * \param max
		 *            the maximum size of the returned List. a value between 100 and 1000 is
		 *            recommended, if you are unsure, use the {\link #DEFAULTSIZE}
		 *
		 * \return a list of result statements. If the size of the returned List is smaller than
		 * the \a max, all statements have been listed and the query id will be invalidated.
		 *
		 * \sa queryConstruct, queryConstructSparql, closeQuery
		 */
		virtual QList<Soprano::Statement> fetchConstructResults( int queryId, int max ) = 0;

		/**
		 * Return a list of statements that are the result a DESCRIBE query,
		 * identified by the passed queryId.
		 * 
		 * \param queryId
		 *            the queryId to fetch.
		 * \param max
		 *            the maximum size of the returned List. a value between 100 and 1000 is
		 *            recommended, if you are unsure, use the {\link #DEFAULTSIZE}
		 *
		 * \return a list of result statements. If the size of the returned List is smaller than
		 * the \a max, all statements have been listed and the query id will be invalidated.
		 *
		 * \sa queryDescribeSparql, closeQuery
		 */
		virtual QList<Soprano::Statement> fetchDescribeResults( int queryId, int max ) = 0;

		/**
		 * Return a query-result-table containing result bindings of the SELECT
		 * query identified by the passed queryId.
		 *
		 * \param queryId
		 *            the queryId to fetch.
		 * \param max the maximum size of the returned List. a value between 100 and 1000 is
		 *            recommended. Pass 0 to return all remaining statements, this can block I/O.
		 *
		 * \return a list of result bindings packed into a QueryResultTable. If the size of the
		 * returned List is smaller than the \a max, all bindings have been listed and the query id 
		 * will be invalidated.
		 *
		 * \sa querySelect, querySelectSparql, closeQuery
		 */
		virtual RDF::QueryResultTable fetchSelectResults( int queryId, int size ) = 0;

		/**
		 * Implementations of the query service may supprot multiple query
		 * languages.
		 *
		 * Each implementation should at least support \a SPARQL.
		 *
		 * \return A list of supported query lanaguages.
		 *
		 * \sa \ref querylang.
		 */
		virtual QStringList supportedQueryLanguages() = 0;

		/**
		 * Implementations of the query service may supprot multiple query
		 * languages.
		 *
		 * Each implementation should at least support \a SPARQL.
		 *
		 * \return 1 if query language \a lang is supported, 0 otherwise.
		 *
		 * The default implementation uses supportedQueryLanguages to test for \a lang.
		 *
		 * \sa \ref querylang.
		 */
		virtual int supportsQueryLanguage( const QString& lang );

		/**
		 * Implementations of the repository service may support multiple RDF
		 * serializations. Each implementation should at least support
		 * application/rdf+xml.
		 * 
		 * \return A list of supported serialization MIME-types.
		 */
		virtual QStringList supportedSerializations() = 0;

		/**
		 * Implementations of the repository service may supprot multiple RDF
		 * serializations. Each implementation should at least support
		 * application/rdf+xml. The default implementation uses
		 * supportedSerialization to test for serializationMimeType.
		 * 
		 * \param serializationMimeType
		 *            the serialization in question
		 * \return 1 if serialization is supported, 0 if not.
		 */
		virtual int supportsSerialization( const QString& serializationMimeType ) = 0;

		/**
		 * Add an RDF graph to a repository.
		 * 
		 * \param repositoryId -
		 *            The \ref repositoryId to add to
		 * \param formatMimetype
		 *            the \ref rdfmime serialization format
		 *            of the string.
		 * \param graph The RDF graph to add, serialised as a string
		 * \param context -
		 *            The context to add to. If the mimetype is a context-aware
		 *            serialization, this can be null. If you set the contextUri,
		 *            and use a context-aware serialization, the context of the
		 *            serialized triples will be lost.
		 */
		virtual void addGraph( const QString& repositoryId, const QString& graph,
				       const QString& formatMimetype, const Soprano::Node& context ) = 0;

		/**
		 * Remove an RDF graph from a repository.
		 * 
		 * \param repositoryId -
		 *            The \ref repositoryId to add to
		 * \param graph -
		 *            The RDF graph to remove, serialised as string
		 * \param formatMimetype
		 *            the \ref rdfmime serialization format
		 *            of the string.
		 * \param context -
		 *            The context to add to. If the mimetype is a context-aware
		 *            serialization, this can be null. If you set the contextUri,
		 *            and use a context-aware serialization, the context of the
		 *            serialized triples will be lost.
		 * \throws Exception
		 *             if the formatMimetype is not supported, if the graph
		 *             serialisation is broken, if the repositoryid is unknown or if
		 *             the contexturi is not a URI
		 */
		virtual void removeGraph( const QString& repositoryId, const QString& graph,
					  const QString& formatMimetype, const Soprano::Node& context ) = 0;
	    };
    }
}

#endif
