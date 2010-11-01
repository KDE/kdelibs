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

#ifndef _NEPOMUK_QUERY_SIMPLE_FACET_H_
#define _NEPOMUK_QUERY_SIMPLE_FACET_H_

#include "facet.h"
#include "nepomukutils_export.h"

namespace Nepomuk {
    namespace Utils {
        /**
         * \class SimpleFacet simplefacet.h Nepomuk/Utils/SimpleFacet
         *
         * \ingroup nepomuk_facets
         *
         * \brief A simple facet using a list of terms.
         *
         * The %SimpleFacet maintains a list of Query::Term objects combined
         * with a label (or a full KGuiItem if required). The usage is very
         * simple: Create a new instance of SimpleFacet, add terms via the addTerm()
         * methods, choose a selection mode via setSelectionMode(), and let
         * SimpleFacet do the rest.
         *
         * %SimpleFacet even implements the selectFromTerm() method which
         * provides enhanced user experience by converting queries into facets.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT SimpleFacet : public Facet
        {
            Q_OBJECT

        public:
            /**
             * Create a new empty facet.
             */
            SimpleFacet( QObject* parent = 0 );

            /**
             * Destructor
             */
            ~SimpleFacet();

            /**
             * \return The currently selected Term. Depending on the
             * selectionMode() this is either a single Term, a combination
             * through Query::AndTerm, or a combination through Query::OrTerm.
             */
            Query::Term queryTerm() const;

            /**
             * Set the selection mode to use in this facet. The default is MatchOne.
             */
            void setSelectionMode( SelectionMode mode );

            /**
             * The selection mode set via setSelectionMode()
             */
            SelectionMode selectionMode() const;

            /**
             * \return The number of terms added via addTerm().
             */
            int count() const;

            /**
             * \return The gui item set via addTerm() for the Term at \p index
             * or an empty KGuiItem if \p index is out of range.
             */
            KGuiItem guiItem( int index ) const;

            /**
             * \return The term set via addTerm() for \p index
             * or an invalid Query::Term if \p index is out of range.
             */
            Query::Term termAt( int index ) const;

            /**
             * \return \p true if the term at \p index has been selected via
             * setSelected().
             */
            bool isSelected( int index ) const;

        public Q_SLOTS:
            /**
             * Clear the list of terms.
             */
            void clear();

            /**
             * Add a new term. This is equivalent to calling
             * \code
             * addTerm(KGuiItem(text), term);
             * \endcode
             */
            void addTerm( const QString& text, const Nepomuk::Query::Term& queryTerm );

            /**
             * Add a new term.
             * \param item The gui item used to present the choice to the user.
             * \param term The query term which represents this choice.
             */
            void addTerm( const KGuiItem& item, const Nepomuk::Query::Term& queryTerm );

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

        private:
            class Private;
            Private* d;
        };
    }
}

#endif
