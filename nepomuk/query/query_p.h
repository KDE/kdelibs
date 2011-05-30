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
                  m_fullTextScoringEnabled( false ),
                  m_fullTextScoringSortOrder( Qt::DescendingOrder ),
                  m_isFileQuery( false ),
                  m_fileMode( FileQuery::QueryFilesAndFolders ) {
            }

            Term createFolderFilter() const;

            /**
             * Optimizations that cannot be done in Term::optimized() since they use internal
             * features of some of the term types.
             * We might expose some of them in the next major KDE release.
             */
            Nepomuk::Query::Term optimizeEvenMore(const Nepomuk::Query::Term& term) const;

            QStringList buildRequestPropertyVariableList() const;
            QString buildRequestPropertyPatterns() const;

            Term m_term;
            int m_limit;
            int m_offset;

            bool m_fullTextScoringEnabled;
            Qt::SortOrder m_fullTextScoringSortOrder;
            Query::QueryFlags m_flags;

            QList<Query::RequestProperty> m_requestProperties;

            bool m_isFileQuery;
            QHash<KUrl, bool> m_includeFolders;
            KUrl::List m_excludeFolders;
            FileQuery::FileMode m_fileMode;
        };
    }
}

#endif
