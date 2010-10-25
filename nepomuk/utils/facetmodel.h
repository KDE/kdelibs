/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_FACET_MODEL_H_
#define _NEPOMUK_QUERY_FACET_MODEL_H_

#include "query.h"
#include "term.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>

namespace Nepomuk {
    namespace Utils {
        class Facet;

        /**
         * \class FacetModel facetmodel.h Nepomuk/Utils/FacetModel
         *
         * \ingroup nepomuk_facets
         *
         * \brief A FacetModel contains a list of facets that are provided in a tree structure.
         *
         * The FacetModel manages a list of Facet instances that are layed out
         * in a tree structure in which the leafs are checkable. This allows to
         * easily configure the facets by clicking the tree.
         *
         * Facets are added via setFacets() and addFacet().
         *
         * Typically one would use FacetWidget instead of creating ones own FacetModel.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class FacetModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:
            /**
             * Creates an empty facet model
             */
            FacetModel( QObject* parent = 0 );

            /**
             * Destructor
             */
            ~FacetModel();

            /**
             * Special roles FacetModel provides through data() in
             * addition to the standard Qt roles.
             *
             * \sa Qt::ItemDataRole
             */
            enum CustomRoles {
                /**
                 * Provides a pointer to the Facet instance itself.
                 * Used internally.
                 */
                FacetRole = 235265643
            };

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            int columnCount( const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            bool setData( const QModelIndex& index, const QVariant& value, int role );

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            bool hasChildren(const QModelIndex &parent) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            QModelIndex parent( const QModelIndex& index ) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            int rowCount( const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;

            /**
             * Reimplemented to provide facet data to Qt's model/view framework.
             */
            Qt::ItemFlags flags( const QModelIndex& index ) const;

            /**
             * \return All Facet instances added via addFacet() and setFacets().
             */
            QList<Facet*> facets() const;

            /**
             * Construct a query term from the selected facets in this model.
             *
             * \return A new query which combines the facets in this model.
             */
            Query::Term queryTerm() const;

        public Q_SLOTS:
            /**
             * Extract as many facets from a query as possible. This method is not able to handle all
             * kinds of queries but works well on queries created via queryTerm().
             *
             * Facets supported by this model will be extracted from \p term and configured
             * accordingly in the model.
             *
             * \return The rest term after facets have been extracted.
             */
            Nepomuk::Query::Term extractFacetsFromTerm( const Nepomuk::Query::Term& term );

            /**
             * Can be used to set the full query the client is using (this includes facets
             * created through this model). It allows the facet system to disable certain
             * choices that would not change the result set or do not make sense otherwise.
             *
             * \sa Facet::setClientQuery()
             */
            void setClientQuery( const Nepomuk::Query::Query& query );

            /**
             * Add \p facet to the model. Used to populate the model with facets.
             * Ownership of \p facet is transferred to the model.
             */
            void addFacet( Nepomuk::Utils::Facet* facet );

            /**
             * Set \p facets as the list of facets used in this model. Used to populate the model with facets.
             * Ownership of the facets is transferred to the model.
             */
            void setFacets( const QList<Nepomuk::Utils::Facet*>& facets );

            /**
             * Remove all facets from the model.
             */
            void clear();

        Q_SIGNALS:
            /**
             * Emitted whenever the facets change, i.e. when the user changes the selection
             * or it is changed programmatically via extractFacetsFromQuery()
             */
            void facetsChanged();

        private:
            class Private;
            Private* const d;

            Q_PRIVATE_SLOT( d, void _k_facetSelectionChanged( Nepomuk::Utils::Facet* ) )
            Q_PRIVATE_SLOT( d, void _k_facetLayoutChanged( Nepomuk::Utils::Facet* ) )
        };
    }
}

Q_DECLARE_METATYPE( Nepomuk::Utils::Facet* )

#endif
