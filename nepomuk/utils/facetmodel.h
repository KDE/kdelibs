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
#include "nepomukutils_export.h"

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
         * The FacetModel can be used to create and augment queries using the queryTerm()
         * method and connecting to the queryTermChanged() signal. In addition %FacetModel
         * provides the extractFacetsFromTerm() method which \em converts a query into
         * facet selections. This is very convinient if the query comes from another source
         * like a query bookmark or another application.
         *
         * An improved user experience can be created by setting the final query used to
         * list the results via setClientQuery(). This allows the %FacetModel to filter
         * the available choices, hiding those that do not make sense with the current
         * result set or even showing facets that did not make sense before (compare the
         * ProxyFacet example in \ref nepomuk_facet_examples).
         *
         * Typically one would use FacetWidget instead of creating ones own FacetModel.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT FacetModel : public QAbstractItemModel
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
             * Facets supported by this model will be extracted from \p query and configured
             * accordingly in the model.
             *
             * Be aware that this method is not related to setClientQuery(). It is intended to
             * split up a query in order to represent it graphically. It will, however change the
             * client query in order to achieve the best possible term extraction. Thus, one would typically
             * call setClientQuery after calling this method if \p query does not already contain
             * the full client query.
             *
             * Typically a client would call this method and then try to handle the returned
             * rest query in another way like converting it into a desktop user query string
             * that can be shown in a search line edit. Another idea would be to use custom
             * filters or a simple warning for the user that additional conditions are in place
             * that could not be "translated" into facets.
             *
             * \return The rest query after facets have been extracted.
             */
            Nepomuk::Query::Query extractFacetsFromQuery( const Nepomuk::Query::Query& query );

            /**
             * Can be used to set the full query the client is using (this includes facets
             * created through this model). It allows the facet system to disable certain
             * choices that would not change the result set or do not make sense otherwise.
             *
             * Be aware that this method is not related to extractFacetsFromTerm(). It is merely
             * intended to improve the overall user experienceby filtering the facet choices
             * depending on the current query.
             *
             * Typically a client would call both extractFacetsFromTerm() and setClientQuery()
             * seperately. However, they will often be called with the same query/term.
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
             * Convenience method that clears the selection on all facets.
             * \sa Facet::clearSelection()
             */
            void clearSelection();

            /**
             * Remove all facets from the model.
             */
            void clear();

        Q_SIGNALS:
            /**
             * Emitted whenever the facets change, i.e. when the user changes the selection
             * or it is changed programmatically via extractFacetsFromQuery()
             */
            void queryTermChanged( const Nepomuk::Query::Term& term );

        private:
            class Private;
            Private* const d;

            Q_PRIVATE_SLOT( d, void _k_queryTermChanged() )
            Q_PRIVATE_SLOT( d, void _k_facetSelectionChanged( Nepomuk::Utils::Facet* ) )
            Q_PRIVATE_SLOT( d, void _k_facetLayoutChanged( Nepomuk::Utils::Facet* ) )
        };
    }
}

Q_DECLARE_METATYPE( Nepomuk::Utils::Facet* )

#endif
