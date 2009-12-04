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

#ifndef _NEPOMUK_QUERY_LITERAL_TERM_H_
#define _NEPOMUK_QUERY_LITERAL_TERM_H_

#include "term.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class LiteralTerm literalterm.h Nepomuk/Query/LiteralTerm
         *
         * \brief Match literal properties via full text.
         *
         * Other than the rest of the terms LiteralTerm has two uses:
         *
         * \section literalterm_comparison Usage in ComparisonTerm
         *
         * %LiteralTerm can be used in a ComparisonTerm with both properties
         * with a literal range and a resource range. The former case is the simple
         * one: the value() of the %LiteralTerm will simply be compared to the value
         * of the ComparisonTerm::property(). In the latter case value() will be
         * matched to rdfs:label of any resource. So in essence the following two
         * examples are equal:
         *
         * \code
         * ComparisonTerm( Vocabulary::NAO::hasTag(), LiteralTerm("nepomuk") );
         * ComparisonTerm( Vocabulary::NAO::hasTag(), ComparisonTerm( Vocabulary::RDFS::label(), LiteralTerm("nepomuk") ) );
         * \endcode
         *
         *
         * \section literalterm_plain Usage as "stand-alone" term
         *
         * As a term by itself %LiteralTerm will act like a traditional full text query and
         * match all resources that contain the string value in some property.
         *
         * \code
         * LiteralTerm( "nepomuk" );
         * \endcode
         *
         * The example above will match all resources that have a property with a value containing
         * the string "nepomuk".
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT LiteralTerm : public Term
        {
        public:
            /**
             * Copy constructor.
             */
            LiteralTerm( const LiteralTerm& term );

            /**
             * Default constructor
             *
             * \param value The value this %LiteralTerm should match to.
             */
            LiteralTerm( const Soprano::LiteralValue& value = Soprano::LiteralValue() );

            /**
             * Desctructor.
             */
            ~LiteralTerm();

            /**
             * Assignment operator.
             */
            LiteralTerm& operator=( const LiteralTerm& term );

            /**
             * The value this %LiteralTerm should match to.
             *
             * \sa setValue()
             */
            Soprano::LiteralValue value() const;

            /**
             * Set the value this %LiteralTerm should match to.
             *
             * \sa value()
             */
            void setValue( const Soprano::LiteralValue& value );
        };
    }
}

#endif
