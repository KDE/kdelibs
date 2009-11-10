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
         * \brief LiteralTerm
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUKQUERY_EXPORT LiteralTerm : public Term
        {
        public:
            LiteralTerm( const LiteralTerm& term );
            LiteralTerm( const Soprano::LiteralValue& value = Soprano::LiteralValue() );
            ~LiteralTerm();

            LiteralTerm& operator=( const LiteralTerm& term );

            Soprano::LiteralValue value() const;
            void setValue( const Soprano::LiteralValue& value );
        };
    }
}

#endif
