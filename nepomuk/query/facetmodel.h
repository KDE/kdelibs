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

#include "nepomukquery_export.h"

#include "query.h"
#include "term.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>

namespace Nepomuk {
    namespace Query {
        class Facet;
        class FacetContainer;

        /**
         * A FacetModel contains a list of facets that are provided in a tree
         * structure.
         */
        class NEPOMUKQUERY_EXPORT FacetModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:
            FacetModel( QObject* parent = 0 );
            ~FacetModel();

            int columnCount( const QModelIndex& parent = QModelIndex() ) const;
            QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
            bool setData( const QModelIndex& index, const QVariant& value, int role );
            bool hasChildren(const QModelIndex &parent) const;
            QModelIndex parent( const QModelIndex& index ) const;
            int rowCount( const QModelIndex& parent = QModelIndex() ) const;
            QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
            Qt::ItemFlags flags( const QModelIndex& index ) const;

            bool frequenciesEnabled() const;

            Query baseQuery() const;

            QList<Facet> facets() const;

            QList<Term> selectedTerms() const;

            /**
             * Construct a query from the selected facets in this model and \p baseQuery.
             *
             * \return A new query which combines the facets in this model with \p baseQuery.
             */
            Query constructQuery() const;

            enum DefaultFacetFlag {
                NoDefaultFacetFlags = 0x0,
                CreateFileFacets = 0x1
            };
            Q_DECLARE_FLAGS( DefaultFacetFlags, DefaultFacetFlag )

            /**
             * Builds the list of default facets for the given type
             *
             * \return the list of default facets
             */
            static QList<Facet> defaultFacets( DefaultFacetFlags flags = NoDefaultFacetFlags );

        public Q_SLOTS:
            /**
             * Extract as many facets from a query as possible. This method is not able to handle all
             * kinds of queries but works well on queries created via constructQuery().
             *
             * Facets supported by this model will be extracted from the \p query and configured
             * accordingly in the model.
             *
             * \return The rest query after facets have been extracted. This is also saved as baseQuery().
             */
            Nepomuk::Query::Query setQuery( const Query& query );

            void setBaseQuery( const Query& query );
            void setFrequenciesEnabled( bool enabled );

            void addFacet( const Facet& facet );
            void setFacets( const QList<Facet>& facets );
            void clear();

        Q_SIGNALS:
            void facetsChanged();

        private:
            class Private;
            Private* const d;

            Q_PRIVATE_SLOT( d, void _k_facetSelectionChanged( Nepomuk::Query::FacetContainer* ) )
            Q_PRIVATE_SLOT( d, void _k_facetFrequencyChanged( Nepomuk::Query::FacetContainer*, int, int ) )
        };
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Query::FacetModel::DefaultFacetFlags )

#endif
