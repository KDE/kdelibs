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

#ifndef _NEPOMUK_QUERY_SIMPLE_TERM_H_
#define _NEPOMUK_QUERY_SIMPLE_TERM_H_

#include "term.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class SimpleTerm simpleterm.h Nepomuk/Query/SimpleTerm
         *
         * \brief Abstract base class for NegationTerm and ComparisonTerm
         * which maintains one sub-term.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT SimpleTerm : public Term
        {
        public:
            /**
             * Destructor.
             */
            ~SimpleTerm();

            /**
             * The sub term to match against.
             *
             * \sa setSubTerm()
             */
            Term subTerm() const;

            /**
             * Set the sub term to match against.
             *
             * \sa subTerm()
             */
            void setSubTerm( const Term& term );

            /** \cond protected_error_members */
        protected:
            SimpleTerm( const Term& term );
            SimpleTerm( TermPrivate* );
            /** \endcond */
        };
    }
}

#endif
