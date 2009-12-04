/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_OR_TERM_H_
#define _NEPOMUK_QUERY_OR_TERM_H_

#include "groupterm.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class OrTerm orterm.h Nepomuk/Query/OrTerm
         *
         * \brief Match resource that match at least one of the sub terms.
         *
         * This class - like AndTerm - is derived from GroupTerm which
         * handles the sub terms.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT OrTerm : public GroupTerm
        {
        public:
            /**
             * Default constructor: creates an invalid or-term.
             */
            OrTerm();

            /**
             * Copy constructor.
             */
            OrTerm( const OrTerm& term );

            /**
             * Convinience constructor which allows
             * to combine several terms in one line.
             */
            OrTerm( const Term& term1,
                    const Term& term2,
                    const Term& term3 = Term(),
                    const Term& term4 = Term(),
                    const Term& term5 = Term(),
                    const Term& term6 = Term() );

            /**
             * Combine \p terms into one and-term.
             */
            OrTerm( const QList<Term>& terms );

            /**
             * Destructor.
             */
            ~OrTerm();

            /**
             * Assignment operator.
             */
            OrTerm& operator=( const OrTerm& term );
        };
    }
}

#endif
