/*
   Copyright (c) 2010 Oszkar Ambrus <aoszkar@gmail.com>
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

#ifndef FACETSWIDGET_H
#define FACETSWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QList>

#include "facet.h"
#include "nepomukutils_export.h"

namespace Nepomuk {
    namespace Query {
        class Query;
        class Term;
    }

    namespace Utils {
        /**
         * \class FacetWidget facetwidget.h Nepomuk/Utils/FacetWidget
         *
         * \ingroup nepomuk_facets
         *
         * \brief A widget providing a list of facets to add filters to a query.
         *
         * \author Oszkar Ambrus <aoszkar@gmail.com>, Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT FacetWidget : public QWidget
        {
            Q_OBJECT

        public:
            /**
             * Create a new empty FacetWidget.
             */
            FacetWidget( QWidget *parent = 0 );

            /**
             * Destructor
             */
            ~FacetWidget();

            /**
             * \return All Facet instances added via addFacet() and setFacets().
             */
            QList<Facet*> facets() const;

            /**
             * Extract as many facets from a query as possible. This method is not able to handle all
             * kinds of queries but works well on queries created via queryTerm().
             *
             * Facets supported by this widget will be extracted from \p term and configured
             * accordingly.
             *
             * \return The rest term after facets have been extracted.
             */
            Nepomuk::Query::Term extractFacetsFromTerm( const Nepomuk::Query::Term& term );

            /**
             * @returns the query term composed by the facets
             */
            Nepomuk::Query::Term queryTerm() const;

            /**
             * Can be used to set the full query the client is using (this includes facets
             * created through this widget). It allows the facet system to disable certain
             * choices that would not change the result set or do not make sense otherwise.
             */
            void setClientQuery( const Nepomuk::Query::Query& query );

        public Q_SLOTS:
            /**
             * Add \p facet to the widget. Used to populate the widget with facets.
             * Ownership of \p facet is transferred to the widget.
             */
            void addFacet( Nepomuk::Utils::Facet* facet );

            /**
             * Set \p facets as the list of facets used in this widget. Used to populate the widget with facets.
             * Ownership of the facets is transferred to the widget.
             */
            void setFacets( const QList<Nepomuk::Utils::Facet*>& facets );

            /**
             * Remove all facets from the widget.
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
            Private * const d;
        };
    }
}

#endif // FACETSWIDGET_H
