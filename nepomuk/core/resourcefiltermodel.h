/* 
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2008 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_RESOURCE_FILTER_MODEL_H_
#define _NEPOMUK_RESOURCE_FILTER_MODEL_H_

#include <Soprano/FilterModel>
#include <Soprano/Node>
#include <Soprano/Vocabulary/RDFS>

#include <QtCore/QList>
#include <QtCore/QDateTime>

namespace Nepomuk {
    /**
     * Filter model that provides a set of convenience methods
     * for maintaining resource properties.
     *
     * It does automatic NRL named graph handling, i.e. provedance
     * data is created and deleted automatically.
     *
     * \warning This model assumes that no property value is stored twice,
     * i.e. in two different named graphs.
     */
    class ResourceFilterModel : public Soprano::FilterModel
    {
    public:
        ResourceFilterModel( Soprano::Model* model = 0 );
        ~ResourceFilterModel();

        /**
         * Update a property. This means an existing property is replaced if it differs from
         * the provided value. Otherwise nothing is done.
         *
         * This method assumes that the cardinality or property is 1.
         */
        Soprano::Error::ErrorCode updateProperty( const QUrl& resource, const QUrl& property, const Soprano::Node& value );

        /**
         * Update a property with a cardinality > 1.
         * This method optmizes the add and remove actions necessary.
         */
        Soprano::Error::ErrorCode updateProperty( const QUrl& resource, const QUrl& property, const QList<Soprano::Node>& values );

        /**
         * Remove a property from a resource and make sure no dangling graphs are left
         */
        Soprano::Error::ErrorCode removeProperty( const QUrl& resource, const QUrl& property );

        /**
         * Ensures that resoruce exists with type.
         */
        Soprano::Error::ErrorCode ensureResource( const QUrl& resource, const QUrl& type = Soprano::Vocabulary::RDFS::Resource() );

        /**
         * Adds the statements into a new nrl:InstanceBase context.
         */
        Soprano::Error::ErrorCode addStatements( const QList<Soprano::Statement>& statements );

        /**
         * Reimplemented to remove metadata about graphs.
         */
        Soprano::Error::ErrorCode removeStatement( const Soprano::Statement &statement );

        /**
         * Reimplemented to remove metadata about graphs.
         */
        Soprano::Error::ErrorCode removeAllStatements( const Soprano::Statement &statement );

        /**
         * Perform a query on instance base graphs. The resource filter model will take care of all graph matching.
         * Thus, queries have a rather restricted form which do not include any "graph" keywords. Basically
         * queries look like:
         *
         * \code
         * select ?v1 ?v2 .... where { ?v1 foo ?v2 . ?v2 bar 'xxx' . ... }
         * \endcode
         *
         * \param start The results can be filtered on creation date. If the start date is set no results created
         * before that date are returned.
         * \param end If the end date is set no results created after that date are returned.
         *
         * \return An iterator iterating over the bindings requested in the original query filteres by nrl:InstanceBase
         * and optionally nao:created.
         *
         * \warning This is a temporary method used until Soprano provides a full query API which does not
         * rely on query strings.
         */
        Soprano::QueryResultIterator instanceQuery( const QString& query, const QDateTime& start = QDateTime(), const QDateTime& end = QDateTime() );

    private:
        Soprano::Error::ErrorCode removeGraphIfEmpty( const Soprano::Node& graph );

        class Private;
        Private* const d;
    };
}

namespace Soprano {
    uint qHash( const Node& node );
}


#endif
