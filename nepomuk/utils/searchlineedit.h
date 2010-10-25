/*
  Copyright (c) 2010 Oszkar Ambrus <aoszkar@gmail.com>
  Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QtGui/QWidget>

#include "query.h"

namespace Nepomuk {
    namespace Utils {
        /**
         * \class SearchLineEdit searchlineedit.h Nepomuk/Utils/SearchLineEdit
         *
         * \brief A line edit which allows to enter user queries.
         *
         * In the future this class will provide some fancy autocompletion.
         *
         * \author Oszkar Ambrus <aoszkar@gmail.com>, Sebastian Trueg <trueg@kde.org>
         */
        class SearchLineEdit : public QWidget
        {
            Q_OBJECT

        public:
            /**
             * Creates a new query edit line with an empty query.
             */
            SearchLineEdit(QWidget *parent = 0);

            /**
             * Destructor
             */
            ~SearchLineEdit();

            /**
             * Sets search while typing option to @p enable.
             * If search while typing is enabled, the queryChanged signal will be emitted when the text is changed,
             * if it is disabled, the signal will be emitted when the user activates it (e.g. presses Enter)
             */
            void setSearchWhileTypingEnabled(bool enable);

            /**
             * @returns true if search while typing is enabled, false otherwise
             */
            bool searchWhileTypingEnabled();

            /**
             * @returns the current query
             */
            Nepomuk::Query::Query query() const;

        public Q_SLOTS:
            /**
             * Extracts parts from \p term which can be converted into a query using the
             * simple desktop query language that is supported by this widget through
             * Nepomuk::Query::QueryParser.
             *
             * \return The rest term that could not be converted into a query string.
             */
            Nepomuk::Query::Term extractUsableTerms( const Nepomuk::Query::Term& term );

        Q_SIGNALS:
            /**
             * Acivated when the query is changed (while typing or when finished)
             */
            void queryChanged( const Nepomuk::Query::Query& query );

        private:
            class Private;
            Private * const d;

            Q_PRIVATE_SLOT( d, void _k_queryStringChanged(const QString&) )
        };
    }
}

#endif // SEARCHLINEEDIT_H
