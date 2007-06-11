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


#ifndef _NEPMUK_BACKBONE_RDF_REPOSITORY_WRAPPER_H_
#define _NEPMUK_BACKBONE_RDF_REPOSITORY_WRAPPER_H_

#include "service.h"
#include "servicewrapper.h"

#include "nepomuk_export.h"

namespace Soprano {
    class Node;
    class Statement;
}

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace RDF {
	class QueryResultTable;
    }

    namespace Services {

	/**
	 * \brief Convenience wrapper class for an easy usage of
	 * a service of type
	 * %http://nepomuk.semanticdesktop.org/services/storage/rdf/Triple.
	 *
	 * \b Usage:
	 *
	 * \code
	 * Service* s = registry->discoverServiceByType( "http://nepomuk.semanticdesktop.org/services/RDFRepository" );
	 * RDFRepository qs( s );
	 * qs.addStatement( ... );
	 * \endcode
	 */
	class NEPOMUK_EXPORT RDFRepository : public Middleware::ServiceWrapper
	    {
	    public:
		RDFRepository( Middleware::Service* );

	    public:
		/**
		 * \sa RDFRepositoryPublisher::createRepository
		 */
		void createRepository( const QString& repositoryId );

		/**
		 * \sa RDFRepositoryPublisher::listRepositoriyIds
		 */
		QStringList listRepositoryIds();

		/**
		 * \sa RDFRepositoryPublisher::removeRepository
		 */
		void removeRepository( const QString& repositoryId );

		/**
		 * \sa RDFRepositoryPublisher::getRepositorySize
		 */
		int getRepositorySize( const QString& repositoryId );

		/**
		 * \sa RDFRepositoryPublisher::contains
		 */
		int contains( const QString& repositoryId, const Soprano::Statement& statement );

		/**
		 * \sa RDFRepositoryPublisher::addStatement
		 */
		void addStatement( const QString& repositoryId, const Soprano::Statement& statement );

		/**
		 * \sa RDFRepositoryPublisher::addStatements
		 */
		void addStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements );

		/**
		 * \sa RDFRepositoryPublisher::removeContext
		 */
		void removeContext( const QString& repositoryId, const Soprano::Node& context );

		/**
		 * \sa RDFRepositoryPublisher::removeStatement
		 */
		int removeStatement( const QString& repositoryId, const Soprano::Statement& statement );

		/**
		 * \sa RDFRepositoryPublisher::removeStatements
		 */
		int removeStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements );

		/**
		 * \sa RDFRepositoryPublisher::removeAllStatements
		 */
		int removeAllStatements( const QString& repositoryId, const Soprano::Statement& statement );

		/**
		 * \sa RDFRepositoryPublisher::listStatements
		 */
		QList<Soprano::Statement> listStatements( const QString& repositoryId, const Soprano::Statement& statement );

		/**
		 * \sa RDFRepositoryPublisher::constructSparql
		 */
		QList<Soprano::Statement> constructSparql( const QString& repositoryId, const QString& query );

		/**
		 * \sa RDFRepositoryPublisher::selectSparql
		 */
		Nepomuk::RDF::QueryResultTable selectSparql( const QString& repositoryId, const QString& query );
	  
		/**
		 * \sa RDFRepositoryPublisher::describeSparql
		 */
		QList<Soprano::Statement> describeSparql( const QString& repositoryId, const QString& query );

		/**
		 * \sa RDFRepositoryPublisher::construct
		 */
		QList<Soprano::Statement> construct( const QString& repositoryId, const QString& query,
						 const QString& querylanguage );
		/**
		 * \sa RDFRepositoryPublisher::select
		 */
		Nepomuk::RDF::QueryResultTable select( const QString& repositoryId, const QString& query,
						       const QString& querylangauge );

		/**
		 * \sa RDFRepositoryPublisher::queryListStatements
		 */
		int queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
					 int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::queryConstruct
		 */
		int queryConstruct( const QString& repositoryId, const QString& query,
				    const QString& querylanguage, int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::querySelect
		 */
		int querySelect( const QString& repositoryId, const QString& query,
				 const QString& querylanguage, int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::queryConstructSparql
		 */
		int queryConstructSparql( const QString& repositoryId, const QString& query,
					  int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::querySelectSparql
		 */
		int querySelectSparql( const QString& repositoryId, const QString& query,
				       int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::queryDescribeSparql
		 */
		int queryDescribeSparql( const QString& repositoryId, const QString& query,
					 int timeoutMSec );

		/**
		 * \sa RDFRepositoryPublisher::askSparql
		 */
		int askSparql( const QString& repositoryId, const QString& query );

		/**
		 * \sa RDFRepositoryPublisher::fetchListStatementResults
		 */
		QList<Soprano::Statement> fetchListStatementsResults( int queryId, int max );

		/**
		 * \sa RDFRepositoryPublisher::fetchConstructResults
		 */
		QList<Soprano::Statement> fetchConstructResults( int queryId, int max );

		/**
		 * \sa RDFRepositoryPublisher::fetchDescribeResults
		 */
		QList<Soprano::Statement> fetchDescribeResults( int queryId, int max );

		/**
		 * \sa RDFRepositoryPublisher::fetchSelectResults
		 */
		Nepomuk::RDF::QueryResultTable fetchSelectResults( int queryId, int size );

		/**
		 * \sa RDFRepositoryPublisher::closeQuery
		 */
		void closeQuery( int queryId );

		/**
		 * \sa RDFRepositoryPublisher::supportedQueryLanguages
		 */
		QStringList supportedQueryLanguages();

		/**
		 * \sa RDFRepositoryPublisher::supportsQueryLanguage
		 */
		int supportsQueryLanguage( const QString& lang );

		/**
		 * \sa RDFRepositoryPublisher::supportedSerializations
		 */
		QStringList supportedSerializations();

		/**
		 * \sa RDFRepositoryPublisher::supportsSerialization
		 */
		int supportsSerialization( const QString& serializationMimeType );

		/**
		 * \sa RDFRepositoryPublisher::addGraph
		 */
		void addGraph( const QString& repositoryId, const QString& graph,
			       const QString& formatMimetype, const Soprano::Node& context );
	  

		/**
		 * \sa RDFRepositoryPublisher::removeGraph
		 */
		void removeGraph( const QString& repositoryId, const QString& graph,
				  const QString& formatMimetype, const Soprano::Node& context );
	    };
    }
}

#endif
