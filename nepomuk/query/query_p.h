/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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
#include "filequery.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {

        class QueryBuilderData;

        class QueryPrivate : public QSharedData
        {
        public:
            QueryPrivate()
                : m_limit( 0 ),
                  m_offset( 0 ),
                  m_isFileQuery( false ),
                  m_fileMode( FileQuery::QueryFilesAndFolders ) {
            }

            QString createFolderFilter( const QString& resourceVarName, QueryBuilderData* ) const;
            QStringList buildRequestPropertyVariableList() const;
            QString buildRequestPropertyPatterns() const;

            /**
             * Removes redundant AND and OR operators.
             */
            static Term optimizeTerm( const Term& term );

            Term m_term;
            int m_limit;
            int m_offset;

            QList<Query::RequestProperty> m_requestProperties;

            bool m_isFileQuery;
            KUrl::List m_includeFolders;
            KUrl::List m_excludeFolders;
            FileQuery::FileMode m_fileMode;
        };
    }
}

#endif
