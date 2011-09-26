/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SIMPLE_RESOURCE_MODEL_H_
#define _NEPOMUK_SIMPLE_RESOURCE_MODEL_H_

#include "resourcemodel.h"

#include "nepomukutils_export.h"

#include "resource.h"
#include "result.h"

#include <QtCore/QList>

namespace Nepomuk {
    namespace Utils {
        /**
         * \class SimpleResourceModel simpleresourcemodel.h Nepomuk/Utils/SimpleResourceModel
         *
         * A simple ResourceModel that handles a list of Resource instances which
         * can be managed via the setResources(), addResource(), addResources(), and
         * clear() methods.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT SimpleResourceModel : public ResourceModel
        {
            Q_OBJECT

        public:
            /**
             * Creates an empty resource model.
             */
            SimpleResourceModel( QObject* parent = 0 );

            /**
             * Destructor
             */
            ~SimpleResourceModel();

            /**
             * Get the Resource which corresponds to \p index.
             *
             * \return The Resource which corresponds to \p index or an invalid Resource
             * if \p index is invalid.
             */
            QModelIndex indexForResource( const Resource& res ) const;

            /**
             * Get the index for a resource.
             *
             * \return The index which corresponds to \p res of an invalid QModelIndex
             * if \p res is not part of this model.
             */
            Resource resourceForIndex( const QModelIndex& index ) const;

            /**
             * \return The number of resources added to the model for an invalid parent index.
             */
            int rowCount( const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Creates an index for the cell at \p row and \p column.
             */
            QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Removes those resources from the model.
             */
            bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

        public Q_SLOTS:
            /**
             * Set the resources to be provided by the model to \p resources.
             */
            void setResources( const QList<Nepomuk::Resource>& resources );

            /**
             * Add \p resources to the list of resources being provided by the
             * model.
             */
            void addResources( const QList<Nepomuk::Resource>& resources );

            /**
             * Add \p resource to the list of resources being provided by the
             * model.
             */
            void addResource( const Nepomuk::Resource& resource );

            /**
             * This method is similar to setResources(). It is provided for
             * allowing convenient connections from signals that provide
             * Query::Result objects.
             */
            void setResults( const QList<Nepomuk::Query::Result>& results );

            /**
             * This method is similar to addResources(). It is provided for
             * allowing convenient connections from signals that provide
             * Query::Result objects like Query::QueryServiceClient::newResults().
             */
            void addResults( const QList<Nepomuk::Query::Result>& results );

            /**
             * This method is similar to addResource(). It is provided for
             * allowing convenient connections from signals that provide
             * Query::Result objects.
             */
            void addResult( const Nepomuk::Query::Result result );

            /**
             * Clear the model by removing all resources added via setResources() and friends.
             */
            void clear();

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif
