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

#ifndef _NEPOMUK_QUERY_PROXY_FACET_H_
#define _NEPOMUK_QUERY_PROXY_FACET_H_

#include "facet.h"
#include "nepomukutils_export.h"

namespace Nepomuk {
    namespace Utils {
        /**
         * \class ProxyFacet proxyfacet.h Nepomuk/Utils/ProxyFacet
         *
         * \ingroup nepomuk_facets
         *
         * \brief A facet forwarding the filtered choices from another facet.
         *
         * The ProxyFacet allows to perform filter operations on another facet.
         * Most noticable one can set a general condition which needs to be fulfilled
         * in the client query in order for the facet to show any choices.
         *
         * \sa \ref nepomuk_facet_examples
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT ProxyFacet : public Facet
        {
            Q_OBJECT

        public:
            /**
             * Create a new proxy facet without a source.
             */
            ProxyFacet( QObject* parent = 0 );

            /**
             * Destructor.
             */
            virtual ~ProxyFacet();

            /**
             * Set the source facet. The proxy will forward all calls to
             * \p source.
             */
            void setSourceFacet( Facet* source );

            /**
             * The source facet which this proxy facet is forwarding.
             * \sa setSourceFacet()
             */
            Facet* sourceFacet() const;

            /**
             * The selection mode forwarded from the sourceFacet().
             */
            SelectionMode selectionMode() const;

            /**
             * The query term forwarded and optionally filtered from the sourceModel().
             */
            virtual Nepomuk::Query::Term queryTerm() const;

            /**
             * \return The number of terms forwarded from the sourceFacet().
             */
            int count() const;

            /**
             * The gui item forwarded from the sourceFacet().
             */
            KGuiItem guiItem( int index ) const;

            /**
             * \return \p true if the term at \p index has been selected via
             * setSelected().
             */
            bool isSelected( int index ) const;

            /**
             * The facet condition set via setFacetCondition()
             */
            Nepomuk::Query::Term facetCondition() const;

        public Q_SLOTS:
            /**
             * Allows to set a condition for the entire facet. Unless it is not met in the
             * client query set via setClientQuery() the facet does not provide any choices.
             *
             * \param term The condition term. If valid this facet will only show any choices
             * if the client query contains \p term in a non-optional way.
             *
             * A typical example is a facet which filters images by size. In that case the
             * condition would be
             * \code
             * Query::ResourceTypeTerm( Vocabulary::NFO::RasterImage() );
             * \endcode
             * since filtering by image size does only make sense if only images are selected.
             */
            void setFacetCondition( const Nepomuk::Query::Term& queryTerm );

            /**
             * Clear the selection. If selectionMode() is MatchOne the first
             * choice should be selected.
             */
            void clearSelection();

            /**
             * Selects or deselects the term at \p index. Depending on the selectionMode()
             * this will also affect the selection of other terms.
             */
            void setSelected( int index, bool selected = true );

            /**
             * The \p term is analyzed depending on the selectionMode().
             *
             * \sa Facet::selectFromTerm()
             */
            bool selectFromTerm( const Nepomuk::Query::Term& queryTerm );

        protected:
            /**
             * Used internally to determine if the facet condition is met. Can be
             * reimplemented for customized filtering.
             *
             * \return \p true if the facet condition is met and all choices from the
             * source facet should be forwarded.
             */
            virtual bool facetConditionMet() const;

            void handleClientQueryChange();

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif
