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

#ifndef _NEPOMUK_QUERY_DYNAMIC_RESOURCE_FACET_H_
#define _NEPOMUK_QUERY_DYNAMIC_RESOURCE_FACET_H_

#include "facet.h"
#include "nepomukutils_export.h"

#include <QtCore/QList>

namespace Nepomuk {
    class Resource;
    namespace Types {
        class Property;
        class Class;
    }
    namespace Query {
        class Result;
    }

    namespace Utils {
        /**
         * \class DynamicResourceFacet dynamicresourcefacet.h Nepomuk/Utils/DynamicResourceFacet
         *
         * \ingroup nepomuk_facets
         *
         * \brief A facet that presents a dynamic lists of resources with the possibility
         * to search for more.
         *
         * The %DynamicResourceFacet provides a way for the user to choose one or more resources
         * which in combination with a property are used to construct Query::ComparisonTerm instances
         * that make up the facet's term.
         *
         * A typical use case would be the relation to a project or a person or a tag. The latter,
         * however, is already provided via Facet::createTagFacet().
         *
         * Usage is simple: set the property to use via setRelation() and optionally a resource type
         * via setResourceType().
         *
         * For customized query terms and resource listing DynamicResourceFacet might also be used
         * as a base class for a new facet by reimplementing methods like resourceTerm() or
         * getMoreResources().
         *
         * \warning DynamicResourceFacet will create a local event loop in order to show a dialog when
         * the user wants to search for more resource candidates.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class NEPOMUKUTILS_EXPORT DynamicResourceFacet : public Facet
        {
            Q_OBJECT

        public:
            /**
             * Create a new empty facet.
             */
            DynamicResourceFacet( QObject* parent = 0 );

            /**
             * Destructor
             */
            virtual ~DynamicResourceFacet();

            /**
             * \return The currently selected Term. Depending on the
             * selectionMode() this is either a single Term, a combination
             * through Query::AndTerm, or a combination through Query::OrTerm.
             */
            Query::Term queryTerm() const;

            /**
             * Set the selection mode to use in this facet. The default is MatchAny.
             */
            void setSelectionMode( SelectionMode mode );

            /**
             * The selection mode set via setSelectionMode()
             */
            SelectionMode selectionMode() const;

            /**
             * \return The number of choices in the facet.
             */
            int count() const;

            /**
             * The parameters used to render the choice at \p index.
             */
            KGuiItem guiItem( int index ) const;

            /**
             * \return \p true if the choice at \p index is selected, \p false otherwise.
             */
            bool isSelected( int index ) const;

            /**
             * Set the property to use in the facet.
             */
            void setRelation( const Types::Property& prop );

            /**
             * This is optional and defaults to the property's
             * range if not set.
             */
            void setResourceType( const Types::Class& type );

            /**
             * Set the maximum rows to show by default. Further resources
             * are accessible through a search dialog.
             * By default the number of max rows is 5.
             */
            void setMaxRows( int max );

            /**
             * \return The property set via setRelation()
             */
            Types::Property relation() const;

            /**
             * \return The resource type set via setResourceType() or the range
             * of relation() in case no specific resource type was set.
             */
            Types::Class resourceType() const;

            /**
             * \return The number of maximum rows set via setMaxRows().
             */
            int maxRows() const;

            /**
             * \return The resource that is represented by the choice at index
             * \p i or an invalid Resource in case \i is out of bounds or points
             * to the "more" action.
             */
            Nepomuk::Resource resourceAt( int i ) const;

            /**
             * \return The currently selected resources.
             */
            QList<Resource> selectedResources() const;

        public Q_SLOTS:
            /**
             * Clear the selection. If selectionMode() is MatchOne the first
             * choice will be selected.
             */
            void clearSelection();

            /**
             * Select or deselect the resource at \p index.
             */
            void setSelected( int index, bool selected = true );

            /**
             * Select or deselect \p res. If \p res is not of the correct type
             * nothing happens.
             */
            void setSelected( const Nepomuk::Resource& res, bool selected = true );

            /**
             * The \p term is analyzed depending on the selectionMode().
             *
             * \sa Facet::selectFromTerm()
             */
            bool selectFromTerm( const Nepomuk::Query::Term& queryTerm );

        protected:
            /**
             * Construct the query that will be used to determine the resources that
             * are presented as choices in the facet.
             *
             * The default implementation uses resourceType() in combination with \p clientQuery
             * to determine the resources that can actually change the final query results.
             *
             * \param clientQuery The query set via setClientQuery(). Reimplementations should
             * use this to restrict the query to resources that actually change the final
             * query results.
             *
             * Reimplementations might also call the default implementation to simply add
             * additional restrictions to the query like so:
             *
             * \code
             * Query::Query query = DynamicResourceFacet::resourceQuery( clientQuery );
             * query.setTerm( query.term() && myFancyQueryTerm );
             * \endcode
             *
             * \return The query that is used to fetch resources to be provided as choices
             * in this facet.
             */
            virtual Query::Query resourceQuery( const Query::Query& clientQuery ) const;

            /**
             * Used by term() to construct the final query term this facet provides.
             * Reimplementing this method allows to customize the way the term is built.
             *
             * The default implementation uses relation() in a Query::ComparisonTerm.
             *
             * When reimplementing this method resourceForTerm() also needs to be reimplemented.
             */
            virtual Nepomuk::Query::Term termForResource( const Resource& res ) const;

            /**
             * Used by selectFromTerm() to convert terms into resources.
             * This is the counterpart to termForResource(). The default implementation checks
             * if \p term is a Query::ComparisonTerm which uses relation().
             *
             * When reimplementing this method termForResource() also needs to be reimplemented.
             *
             * \return The resource that is represented by \p term if the latter has the form
             * as terms generated by termForResource(), an invalid Resource otherwise.
             */
            virtual Nepomuk::Resource resourceForTerm( const Nepomuk::Query::Term& queryTerm ) const;

            /**
             * Used to query additional resources if maxRows() is too small to fit all
             * candidates.
             *
             * The default implementation simply uses SearchWidget to let the user query
             * additional resources.
             *
             * \warning The default implementation opens a dialog which will start a local
             * event loop.
             *
             * Reimplementations may use clientQuery() to optimize the resource selection.
             *
             * \return The list of resources that should be selected in addition to the
             * already selected ones.
             */
            virtual QList<Resource> getMoreResources() const;

            /**
             * Reimplemented to properly update the resource list on client query change.
             */
            void handleClientQueryChange();

#ifdef BUILDING_NEPOMUK_TESTS
        protected:
#else
        private:
#endif
            class Private;
            Private* d;

            Q_PRIVATE_SLOT( d, void _k_newEntries( const QList<Nepomuk::Query::Result>& ) )
            Q_PRIVATE_SLOT( d, void _k_populateFinished() )
        };
    }
}

#endif
