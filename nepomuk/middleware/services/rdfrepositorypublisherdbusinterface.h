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

#ifndef RDFREPOSITORYPUBLISHERDBUSINTERFACE_H
#define RDFREPOSITORYPUBLISHERDBUSINTERFACE_H

#include "servicepublisherdbusinterface.h"
#include "queryresulttable.h"
#include "nepomuk_export.h"

#include <QtCore/QList>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <soprano/statement.h>


namespace Nepomuk {
    namespace Services {
	class RDFRepositoryPublisher;

	namespace DBus {
	    class NEPOMUK_MIDDLEWARE_EXPORT RDFRepositoryPublisherInterface : public Middleware::DBus::ServicePublisherInterface
            {
                Q_OBJECT
                Q_CLASSINFO("D-Bus Interface", "org.semanticdesktop.nepomuk.services.RDFRepository")

            public:
                RDFRepositoryPublisherInterface( Middleware::ServicePublisher* );

            public Q_SLOTS:
                void createRepository( const QString& repositoryId, const QDBusMessage& );

                QStringList listRepositoryIds( const QDBusMessage& );

                void removeRepository( const QString& repositoryId, const QDBusMessage& );

                int getRepositorySize( const QString& repositoryId, const QDBusMessage& );

                int contains( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& );

                void addStatement( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& );

                void addStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements, const QDBusMessage& );

                void removeContext( const QString& repositoryId, const Soprano::Node& context, const QDBusMessage& );

                int removeStatement( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& );

                int removeStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements, const QDBusMessage& );

                int removeAllStatements( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& );

                QList<Soprano::Statement> listStatements( const QString& repositoryId, const Soprano::Statement& statement,
                                                          const QDBusMessage& );

                QList<Soprano::Statement> constructSparql( const QString& repositoryId, const QString& query, const QDBusMessage& );

                Nepomuk::RDF::QueryResultTable selectSparql( const QString& repositoryId, const QString& query, const QDBusMessage& );

                QList<Soprano::Statement> describeSparql( const QString& repositoryId, const QString& query, const QDBusMessage& );

                QList<Soprano::Statement> construct( const QString& repositoryId, const QString& query,
                                                     const QString& querylanguage, const QDBusMessage& );

                Nepomuk::RDF::QueryResultTable select( const QString& repositoryId, const QString& query,
                                                       const QString& querylangauge, const QDBusMessage& );

                int queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
                                         int timeoutMSec, const QDBusMessage& );

                int queryConstruct( const QString& repositoryId, const QString& query,
                                    const QString& querylanguage, int timeoutMSec, const QDBusMessage& );

                int querySelect( const QString& repositoryId, const QString& query,
                                 const QString& querylanguage, int timeoutMSec, const QDBusMessage& );

                int queryConstructSparql( const QString& repositoryId, const QString& query,
                                          int timeoutMSec, const QDBusMessage& );

                int querySelectSparql( const QString& repositoryId, const QString& query,
                                       int timeoutMSec, const QDBusMessage& );

                int queryDescribeSparql( const QString& repositoryId, const QString& query,
                                         int timeoutMSec, const QDBusMessage& );

                int askSparql( const QString& repositoryId, const QString& query, const QDBusMessage& );

                QList<Soprano::Statement> fetchListStatementsResults( int queryId, int max, const QDBusMessage& );

                QList<Soprano::Statement> fetchConstructResults( int queryId, int max, const QDBusMessage& );

                QList<Soprano::Statement> fetchDescribeResults( int queryId, int max, const QDBusMessage& );

                Nepomuk::RDF::QueryResultTable fetchSelectResults( int queryId, int size, const QDBusMessage& );

                void closeQuery( int queryId, const QDBusMessage& );

                QStringList supportedQueryLanguages( const QDBusMessage& );

                int supportsQueryLanguage( const QString& lang, const QDBusMessage& );

                QStringList supportedSerializations( const QDBusMessage& );

                int supportsSerialization( const QString& serializationMimeType, const QDBusMessage& );

                void addGraph( const QString& repositoryId, const QString& graph,
                               const QString& formatMimetype, const Soprano::Node& context, const QDBusMessage& );


                void removeGraph( const QString& repositoryId, const QString& graph,
                                  const QString& formatMimetype, const Soprano::Node& context, const QDBusMessage& );
            };
	}
    }
}

#endif // RDFREPOSITORYPUBLISHERDBUSINTERFACE_H
