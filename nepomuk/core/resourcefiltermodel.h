/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
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

    class ResourceManager;

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
        Q_OBJECT

    public:
        ResourceFilterModel( ResourceManager* manager );
        ~ResourceFilterModel();

        /**
         * Adds a statement to the Model. Statements without a valid context will be
         * added to the mainContext.
         */
        Soprano::Error::ErrorCode addStatement( const Soprano::Statement& statement );

        /**
         * The ResourceFilterModel automatically creates graph metadata and tries to
         * group batch operations into one graph. This method returns the currently
         * "open" graph, i.e. that one which is used in all the convinience methods.
         */
        QUrl mainContext();

        /**
         * Updates the modification date of \p resource to \p date.
         */
        Soprano::Error::ErrorCode updateModificationDate( const QUrl& resource, const QDateTime& date = QDateTime::currentDateTime() );

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

        using FilterModel::addStatement;

    private:
        class Private;
        Private* const d;
    };
}

#endif
