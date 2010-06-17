/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_FILE_QUERY_QUERY_H_
#define _NEPOMUK_FILE_QUERY_QUERY_H_

#include "query.h"
#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class FileQuery filequery.h Nepomuk/Query/FileQuery
         *
         * \brief A Nepomuk desktop query specialized for file searches.
         *
         * FileQuery is an extension to Query which adds some syntactic sugar
         * for dealing with file queries. This includes a restriction of the
         * results to files and the possibility to restrict the search to
         * specific folders via setIncludeFolders() and setExcludeFolders().
         *
         * \warning %FileQuery does only return files and folders as results.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT FileQuery : public Query
        {
        public:
            /**
             * Create an empty invalid file query object.
             */
            FileQuery();

            /**
             * Create a file query with root term \a term.
             *
             * \since 4.6
             */
            explicit FileQuery( const Term& term );

            /**
             * Copy constructor.
             */
            FileQuery( const Query& query );

            /**
             * Destructor
             */
            ~FileQuery();

            /**
             * Assignment operator
             */
            FileQuery& operator=( const Query& );

            /**
             * Add a folder to include in the search. If include folders are set the query
             * will be restricted to files from that folders and their subfolders.
             *
             * Be aware that setting a folder filter will implicitely restrict the query
             * to files and folders.
             *
             * \param folder The folder to include in the search.
             *
             * \sa setIncludeFolders, includeFolders, addExcludeFolder
             */
            void addIncludeFolder( const KUrl& folder );

            /**
             * \overload
             *
             * \param folders The folders to include in the search.
             *
             * \sa addIncludeFolder, includeFolders, setExcludeFolders
             */
            void setIncludeFolders( const KUrl::List& folders );

            /**
             * The list of include folders set via addIncludeFolder() and
             * setIncludeFolders().
             *
             * \sa addIncludeFolder, setIncludeFolders, excludeFolders
             */
            KUrl::List includeFolders() const;

            /**
             * Add a folder to exclude from the search. If exclude folders are set the query
             * will be restricted to files that are not in that folder and its subfolders.
             *
             * Be aware that setting a folder filter will implicitely restrict the query
             * to files and folders.
             *
             * \param folder The folder to exclude from the search.
             *
             * \sa setExcludeFolders, excludeFolders, addIncludeFolder
             */
            void addExcludeFolder( const KUrl& folder );

            /**
             * \overload
             *
             * \param folders The folders to exclude from the search.
             *
             * \sa addExcludeFolder, excludeFolders, setIncludeFolders
             */
            void setExcludeFolders( const KUrl::List& folders );

            /**
             * The list of exclude folders set via addExcludeFolder() and
             * setExcludeFolders().
             *
             * \sa addExcludeFolder, setExcludeFolders, includeFolders
             */
            KUrl::List excludeFolders() const;

            /**
             * An enumeration used in setFileMode() to state wether the query
             * should return files and folders or only files or only folders.
             *
             * \since 4.5
             */
            enum FileModeFlags {
                QueryFiles = 0x1,
                QueryFolders = 0x2,
                QueryFilesAndFolders = QueryFiles|QueryFolders
            };
            Q_DECLARE_FLAGS( FileMode, FileModeFlags )

            /**
             * Set the file mode, i.e. wether the query should return
             * files and folders or only files or only folders.
             * By default both files and folders are returned.
             *
             * \sa fileMode()
             *
             * \since 4.5
             */
            void setFileMode( FileMode mode );

            /**
             * \return The file mode set in setFileMode()
             *
             * \since 4.5
             */
            FileMode fileMode() const;
        };
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Query::FileQuery::FileMode )

#endif
