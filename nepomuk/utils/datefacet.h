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

#ifndef _NEPOMUK_QUERY_DATE_FACET_H_
#define _NEPOMUK_QUERY_DATE_FACET_H_

#include "facet.h"

class DateRange;

namespace Nepomuk {
    namespace Utils {
        /**
         * \class DateFacet datefacet.h Nepomuk/Utils/DateFacet
         *
         * \ingroup nepomuk_facets
         *
         * \brief A Facet that provides a set of date ranges.
         *
         * Filtering search results on a date is a very typical problem. This could be
         * handled by adding fixed Terms to a SimpleFacet. The big problem with that
         * solution, however, is that these fixed terms would become inaccurate over time.
         * Say, one wants to filter files that have been modified today. This is a very
         * simple filter involving the nie:lastModified property and QDate::currentDate().
         * But as soon as the application runs while the current date is changing this
         * fixed term becomes invalid since it is now referencing yesterday instead of today.
         *
         * The DateFacet works around this problem by using the enumeration DateRangeCandidate
         * instead of fixed terms and creating the terms on the fly based on the enum value.
         *
         * By default it creates date terms via Nepomuk::Utils::dateRangeQuery() which
         * includes the file modification time but also access times and content creation times
         * (the latter includes the date a photo was taken). This can, however, be modified by
         * reimplementing createDateRangeTerm() and its counterpart extractDateRange().
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.6
         */
        class DateFacet : public Facet
        {
            Q_OBJECT

        public:
            /**
             * Creates a new date facet which contains the DefaultRanges
             * date range candidates as choices.
             */
            DateFacet( QObject* parent = 0 );

            /**
             * Destructor
             */
            virtual ~DateFacet();

            /**
             * Creates a query term for the currently selected date range.
             * This method makes use of createDateRangeTerm().
             */
            Query::Term queryTerm() const;

            /**
             * The selection mode in the date facet is fixed to MatchOne, ie.
             * it is an exclusive facet where a single date range must be
             * selected at all time.
             */
            SelectionMode selectionMode() const;

            /**
             * \return The number of date range candidates set via setDateRangeCandidates().
             */
            int count() const;

            KGuiItem guiItem( int index ) const;
            bool isSelected( int index ) const;

            /**
             * Using fixed QDate values would result in
             * invalid queries as soon as an application runs
             * over the span of two days. Thus, DateFacet uses
             * DateRangeCandidates to specify the list of
             * choices and generates the QDate instances on the
             * fly.
             */
            enum DateRangeCandidate {
                /// no date range
                NoDateRange = 0x0,

                /// no restriction on the date
                Anytime = 0x1,

                /// a date range which uses QDate::today() for start and end
                Today = 0x2,

                /// a date range which uses QDate::today().addDays(-1) for start and end
                Yesterday = 0x4,

                /// a date range that includes all days from the current week up to today
                ThisWeek = 0x8,

                /// a date range that includes all days from the last week, ie. that before the current one
                LastWeek = 0x10,

                /// a date range that includes all days from the current month up to today
                ThisMonth = 0x20,

                /// a date range that includes all days from the last month, ie. that before the current one
                LastMonth = 0x40,

                /// a date range that includes all days from the current year up to today
                ThisYear = 0x80,

                /// a date range that includes all days from the last year, ie. that before the current one
                LastYear = 0x100,

                /// a custom range - this will enable a special item that if selection will open a dialog
                /// allowing the user to choose a custom date range
                CustomDateRange = 0x10000,

                /// the defaults this facet starts with
                DefaultRanges = Anytime|Today|ThisWeek|ThisMonth|ThisYear|CustomDateRange
            };
            Q_DECLARE_FLAGS( DateRangeCandidates, DateRangeCandidate )

            /**
             * Set the date ranges this facet should provide. By default all
             * date ranges from DefaultRanges are enabled.
             */
            void setDateRangeCandidates( DateRangeCandidates ranges );

            /**
             * \return The date ranges configured via setDateRangeCandidates()
             * \sa DateRangeCandidate
             */
            DateRangeCandidates dateRangeCandidates() const;

        public Q_SLOTS:
            /**
             * Resets the current range to the default value which is the first in the
             * list of candidates. If Anytime is part of the configured candidates it will
             * be selected.
             */
            void clearSelection();

            /**
             * Change the currently selected date range. This method is typically triggered
             * by user interaction.
             */
            void setSelected( int index, bool selected = true );

            /**
             * Select the current date range based on \p term. This method relies on
             * extractDateRange() to determine if \p term can be used or not.
             *
             * \return \p true if \p term could be handled and the current range
             * has been changed accordingly, \p false otherwise.
             */
            bool selectFromTerm( const Nepomuk::Query::Term& queryTerm );

        protected:
            /**
             * Create the query Term that reflects the dates specified in \p range.
             * The default implementation uses Nepomuk::Utils::dateRangeQuery().
             *
             * Subclasses may reimplement this method to create custom date ranges.
             *
             * If this method is reimplemented extractDateRange() also needs to be
             * implemented in order for selectFromTerm() to work properly.
             */
            virtual Query::Term createDateRangeTerm( const DateRange& range ) const;

            /**
             * This is the counterpart method to createDateRangeTerm(). It should only
             * extract valid date ranges from terms that have been created by createDateRangeTerm()
             * or have the same structure.
             *
             * A working implementation is required in order for selectFromTerm() to
             * work properly.
             *
             * The default implementation can handle query terms created via
             * Nepomuk::Utils::dateRangeQuery().
             */
            virtual DateRange extractDateRange( const Query::Term& queryTerm ) const;

            /**
             * Retrieve a custom date range typically from the user. The default implementation
             * opens a popup at the current cursor position allowing the user to choose the range
             * via appropriate GUI elements.
             *
             * This method is called internally by DateFacet to handle the CustomRange date range
             * candidate.
             *
             * Reimplementing this method allows to provide a customized way to get a range
             * for the CustomRange date range candidate.
             *
             * \param ok The method should set this boolean according to the user feedback. This is
             * necessary since returning an invalid DateRange is ambiguous.
             *
             * \return The date range chosen by the user.
             */
            virtual DateRange getCustomRange( bool* ok = 0 ) const;

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif
