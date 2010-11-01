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

#ifndef _NEPOMUK_QUERY_FACET_H_
#define _NEPOMUK_QUERY_FACET_H_

#include <QtCore/QObject>

#include "nepomukutils_export.h"

class KGuiItem;

namespace Nepomuk {
    namespace Query {
        class Term;
        class Query;
    }

    namespace Utils {
        /**
         * \class Facet facet.h Nepomuk/Utils/Facet
         *
         * \ingroup nepomuk_facets
         *
         * \brief The base class for all facets used to create query filters.
         *
         * A Facet represents one aspect of a query like the resource type or
         * the modification date. By providing a list of facets to the user they
         * can interactively construct a query by restricting the result set step
         * by step.
         *
         * A Facet can be seen as a list of choices presented to the user. Depending
         * on the selection (which is typically triggered by user interaction) the
         * Facet produces a resulting term() which is then used in the final query.
         *
         * Each facet needs to implement a set of methods describing the choices:
         * count(), guiItem(), isSelected(), setSelected(), and clearSelection().
         * In addition the term() method constructs the term this facet represents
         * with its current selection.
         *
         * Methods selectFromTerm() and setClientQuery() are optional but allow for a
         * more advanced usage and a better user experience. See the method documentation
         * for details.
         *
         * Before implementing your own Facet subclass make sure that SimpleFacet or
         * DynamicResourceFacet do not suit your needs.
         *
         * Facet also provides a set of static factory methods like createTagFacet() or
         * createFileTypeFacet() to create standard facets that are useful in most
         * query UI situations.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT Facet : public QObject
        {
            Q_OBJECT

        public:
            /**
             * Constructor
             */
            Facet( QObject* parent = 0 );

            /**
             * Destructor
             */
            virtual ~Facet();

            /**
             * A Facet can have one of three selection modes which is provided
             * by selectionMode().
             */
            enum SelectionMode {
                /**
                 * All of the choices need to be matched. This typically
                 * means that all choices' terms are combined using an
                 * AndTerm.
                 */
                MatchAll,

                /**
                 * At least one of the choices needs to be matched. This typically
                 * means that all choices' terms are combined using an
                 * OrTerm.
                 */
                MatchAny,

                /**
                 * Exactly one of the choices needs to match. This means
                 * that the Facet is exclusive, ie. only one choice
                 * can be selected at all times.
                 */
                MatchOne
            };

            /**
             * The selection mode used by this facet. The GUI client
             * can make use of this value to adjust the GUI accordingly.
             * A typical example would be using radio buttons for
             * MatchOne facets.
             */
            virtual SelectionMode selectionMode() const = 0;

            /**
             * The term currently produced by this facet. This is dependant
             * on the subclass implementation and the selectionMode(). The SimpleFacet for example
             * uses an AndTerm or an OrTerm to combine all its terms in MatchAll or MatchAny
             * mode while it returns the one selected term in MatchOne mode.
             */
            virtual Query::Term queryTerm() const = 0;

            /**
             * The number of choices this facet provides.
             */
            virtual int count() const = 0;

            /**
             * The text to be displayed at \p index. The default implementation makes
             * use of guiItem(). Normally subclasses would implement guItem() instead
             * of this method.
             */
            virtual QString text( int index ) const;

            /**
             * The parameters used to render the choice at \p index.
             */
            virtual KGuiItem guiItem( int index ) const;

            /**
             * \return \p true if the choice at \p index is selected, \p false otherwise.
             */
            virtual bool isSelected( int index ) const = 0;

            /**
             * The client query set via setClientQuery(). See the documentation of setClientQuery()
             * for details.
             */
            Query::Query clientQuery() const;

            /**
             * Creates a new Facet that allows to filter on the file type. Normally
             * this facet would be used in combination with a FileQuery.
             * \sa createTypeFacet()
             */
            static Facet* createFileTypeFacet( QObject* parent = 0 );

            /**
             * Creates a new Facet that allows to filter on the resource type without
             * a restriction to files.
             * \sa createFileTypeFacet()
             */
            static Facet* createTypeFacet( QObject* parent = 0 );

            /**
             * Creates a new Facet that allows to filter on the date. This includes
             * the modification, creation, and access dates.
             * \sa Query::dateRangeQuery()
             */
            static Facet* createDateFacet( QObject* parent = 0 );

            /**
             * Creates a new Facet that allows to filter on tags, i.e. search for
             * resources that are tagged with the selected tags. The list of tags
             * is dynamic. This means it has a default selection of the most often
             * used ones and provides a means to access all tags through a dialog.
             */
            static Facet* createTagFacet( QObject* parent = 0 );

            /**
             * Creates a new Facet that allows to sort the results by one of several
             * criteria like last modification date or a score calculated by the
             * %Nepomuk system.
             * Using this facet does only make sense when displaying sorted results
             * or when using a limit on the number of results.
             */
            static Facet* createPriorityFacet( QObject* parent = 0 );

            /**
             * Creates a new Facet that allows to filter on the rating of resources.
             */
            static Facet* createRatingFacet( QObject* parent = 0 );

        public Q_SLOTS:
            /**
             * Clear the selection. If selectionMode() is MatchOne the first
             * choice should be selected.
             */
            virtual void clearSelection() = 0;

            /**
             * Called by client code to change the selection.
             *
             * \param index The index of the choice for which the selection should
             * be changed.
             * \param selected If \p true the item should be selected, otherwise
             * it should be deselected.
             */
            virtual void setSelected( int index, bool selected = true ) = 0;

            /**
             * If a client application provides several ways to construct a query
             * (one could think of a query editor which allows to insert certain
             * constraints or simply another application providing a base query)
             * the user expects to be able to modify that query using the available
             * facets. Thus, a Facet should be able to extract parts from a query.
             *
             * Implement this method to create a selection based on the contents of
             * \p term. The selection of the facet must only change if all of \p term
             * can be used. This means that if \p term is for example an OrTerm and
             * the Facet can only handle one of the sub terms it needs to ignore the
             * whole term. FacetModel::setQuery() internally calls selectFromTerm()
             * on all its facets.
             *
             * A simple example is SimpleFacet which compares all its terms which have been
             * added via SimpleFacet::addTerm() to \p term. Depending on the selectionMode()
             * it also checks for AndTerm or OrTerm.
             *
             * \warning Implementations of this method should \em never reset the selection
             * before handling \p term. Instead the method should work similar to setSelected(),
             * ie. in MatchAll or MatchAny facets calling it multiple times should select
             * multiple choices.
             *
             * \return \p true if all of \p term could be used to select choices in
             * this term, \p false otherwise.
             */
            virtual bool selectFromTerm( const Nepomuk::Query::Term& queryTerm ) = 0;

            /**
             * The FacetModel will set this to the final query that has been constructed
             * from the facets and any other query part set by the client via FacetModel::setClientQuery().
             * The facet may use this information to filter the presented choices. A typical example
             * is to only show terms that would actually change the result set. The latter is what
             * DynamicResourceFacet does.
             */
            void setClientQuery( const Nepomuk::Query::Query& query );

        Q_SIGNALS:
            /**
             * Emitted when the term of the facet changed, ie. when the value returned by term()
             * has changed.
             *
             * Subclasses should call setTermChanged() instead of emitting this signal manually.
             *
             * Be aware that in most situations queryTermChanged() and selectionChanged() will
             * have to be emitted at the same time.
             */
            void queryTermChanged( Nepomuk::Utils::Facet* facet, const Nepomuk::Query::Term& queryTerm );

            /**
             * Emitted when the layout of the facet changed, ie. one of count(), guitItem(), or
             * text() return different values.
             *
             * Subclasses should call setLayoutChanged() instead of emitting this signal manually.
             */
            void layoutChanged( Nepomuk::Utils::Facet* facet );

            /**
             * Emitted when the selection changed - normally triggered by a call to setSelected().
             *
             * Subclasses should call setSelectionChanged() instead of emitting this signal manually.
             *
             * Be aware that in most situations queryTermChanged() and selectionChanged() will
             * have to be emitted at the same time.
             */
            void selectionChanged( Nepomuk::Utils::Facet* facet );

        protected Q_SLOTS:
            /**
             * Subclasses should call this method instead of emitting termChanged() manually.
             */
            void setQueryTermChanged();

            /**
             * Subclasses should call this method instead of emitting layoutChanged() manually.
             */
            void setLayoutChanged();

            /**
             * Subclasses should call this method instead of emitting selectionChanged() manually.
             */
            void setSelectionChanged();

        protected:
            /**
             * This method is called from setClientQuery() and can be reimplemented by subclasses.
             *
             * The default implementation does nothing.
             */
            virtual void handleClientQueryChange();

        private:
            class FacetPrivate;
            FacetPrivate* const d;
        };
    }
}

#endif
