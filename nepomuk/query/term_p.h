/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_SEARCH_TERM_P_H_
#define _NEPOMUK_SEARCH_TERM_P_H_

#include <QtCore/QSharedData>

#include "term.h"

namespace Nepomuk {
    namespace Query {

        class QueryBuilderData;

        class TermPrivate : public QSharedData
        {
        public:
            TermPrivate( Term::Type t = Term::Invalid )
                : m_type( t ) {
            }
            virtual ~TermPrivate() {
            }

            virtual TermPrivate* clone() const { return new TermPrivate( *this ); }

            virtual bool isValid() const { return false; }
            virtual QString toSparqlGraphPattern( const QString&, QueryBuilderData* ) const { return QString(); }
            virtual bool equals( const TermPrivate* other ) const {
                return m_type == other->m_type;
            }

            Term::Type m_type;
        };
    }
}

#define N_D(Class) Class##Private* d = static_cast<Class##Private*>( d_ptr.data() )
#define N_D_CONST(Class) const Class##Private* d = static_cast<const Class##Private*>( d_ptr.constData() )

#endif
