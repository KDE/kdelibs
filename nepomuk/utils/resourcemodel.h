/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_RESOUCE_MODEL_H_
#define _NEPOMUK_RESOUCE_MODEL_H_

#include <QtCore/QAbstractItemModel>

#include "nepomukutils_export.h"

namespace Nepomuk {

    class Resource;

    namespace Utils {
        /**
         * \class ResourceModel resourcemodel.h Nepomuk/Utils/ResourceModel
         *
         * \brief Base class for all models providing a plain list of resources.
         *
         * The %ResourceModel is a base class for models that handle a set of
         * %Nepomuk Resource instances. This can be a simple list as in SimpleResourceModel
         * or a set of query results as in ResultModel. It could also be a dynamic list
         * which is updated while the user scrolls it.
         *
         * %ResourceModel cannot be instanciated by itself. Use one of the subclasses
         * or derive your own subclass from it.
         *
         * At least the following methods need to be implemented in a subclass:
         * \li resourceForIndex()
         * \li indexForResource(),
         * \li QAbstractItemModel::rowCount()
         * \li QAbstractItemModel::index()
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT ResourceModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:
            /**
             * Constructor.
             */
            ResourceModel( QObject* parent = 0 );

            /**
             * Destructor
             */
            virtual ~ResourceModel();

            /**
             * Custom roles that can be accessed for example in delegates.
             */
            enum ResourceRoles {
                /**
                 * The resource itself.
                 */
                ResourceRole = 7766897,

                /**
                 * The creation date of the resource.
                 */
                ResourceCreationDate = 7766898
            };

            /**
             * Get the Resource which corresponds to \p index.
             *
             * \return The Resource which corresponds to \p index or an invalid Resource
             * if \p index is invalid.
             */
            virtual Resource resourceForIndex( const QModelIndex& index ) const = 0;

            /**
             * Get the index for a resource.
             *
             * \return The index which corresponds to \p res of an invalid QModelIndex
             * if \p res is not part of this model.
             */
            virtual QModelIndex indexForResource( const Resource& res ) const = 0;

            /**
             * The default implementation returns an invalid QModelIndex, thus providing
             * a plain list.
             */
            virtual QModelIndex parent( const QModelIndex& child ) const;

            /**
             * The default implementation returns 1 as the default implementation of data()
             * does only return values for the first column.
             */
            virtual int columnCount( const QModelIndex& parent ) const;

            /**
             * Handles most roles typically used in applications like Qt::DisplayRole, Qt::ToolTipRole,
             * and Qt::DecorationRole. Additionally KCategorizedSortFilterProxyModel roles are supported
             * categorizing by resource types.
             *
             * Only the first column is supported.
             */
            virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

            /**
             * Reimplemented to support dragging of resources out of the model.
             */
            Qt::ItemFlags flags( const QModelIndex& index ) const;

            /**
             * Stores the resource URIs via KUrl::List::populateMimeData() and as a specific
             * "application/x-nepomuk-resource-uri" mime type to indicate that these are URIs
             * corresponding to actual %Nepomuk resources.
             */
            QMimeData* mimeData( const QModelIndexList& indexes ) const;

            /**
             * \return The KUrl mime types and "application/x-nepomuk-resource-uri".
             */
            QStringList mimeTypes() const;

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif
