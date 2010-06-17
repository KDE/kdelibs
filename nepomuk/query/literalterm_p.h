/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_LITERAL_TERM_P_H_
#define _NEPOMUK_QUERY_LITERAL_TERM_P_H_

#include "term_p.h"

#include <Soprano/LiteralValue>

namespace Nepomuk {
    namespace Query {
        class LiteralTermPrivate : public TermPrivate
        {
        public:
            LiteralTermPrivate();

            TermPrivate* clone() const { return new LiteralTermPrivate( *this ); }

            bool equals( const TermPrivate* other ) const;
            bool isValid() const { return m_value.isValid(); }
            QString toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const;

            /// The text that is to be used in the bif:contains pattern.
            QString queryText() const;

            Soprano::LiteralValue m_value;
        };
    }
}

#endif
