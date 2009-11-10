/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SEARCH_QUERY_P_H_
#define _NEPOMUK_SEARCH_QUERY_P_H_

#include <QtCore/QSharedData>
#include <QtCore/QList>

#include <kurl.h>

#include "term.h"
#include "query.h"

namespace Nepomuk {
    namespace Query {

        class QueryBuilderData;

        class QueryPrivate : public QSharedData
        {
        public:
            QueryPrivate()
                : m_limit( 0 ),
                  m_isFileQuery( false ) {
            }

            QString createFolderFilter( const QString& resourceVarName, QueryBuilderData* ) const;
            QString buildRequestPropertyVariableList() const;
            QString buildRequestPropertyPatterns() const;

            Term m_term;
            int m_limit;

            QList<Query::RequestProperty> m_requestProperties;

            bool m_isFileQuery;
            KUrl::List m_includeFolders;
            KUrl::List m_excludeFolders;
        };
    }
}

#endif
