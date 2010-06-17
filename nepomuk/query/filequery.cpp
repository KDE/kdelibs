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

#include "filequery.h"
#include "query_p.h"

Nepomuk::Query::FileQuery::FileQuery()
    : Query()
{
    d->m_isFileQuery = true;
}


Nepomuk::Query::FileQuery::FileQuery( const Term& term )
    : Query( term )
{
    d->m_isFileQuery = true;
}


Nepomuk::Query::FileQuery::FileQuery( const Query& query )
    : Query( query )
{
    d->m_isFileQuery = true;
}


Nepomuk::Query::FileQuery::~FileQuery()
{
}


Nepomuk::Query::FileQuery& Nepomuk::Query::FileQuery::operator=( const Query& query )
{
    Query::operator=( query );
    d->m_isFileQuery = true;
    return *this;
}


void Nepomuk::Query::FileQuery::addIncludeFolder( const KUrl& folder )
{
    d->m_includeFolders << folder;
}


void Nepomuk::Query::FileQuery::setIncludeFolders( const KUrl::List& folders )
{
    d->m_includeFolders = folders;
}


KUrl::List Nepomuk::Query::FileQuery::includeFolders() const
{
    return d->m_includeFolders;
}


void Nepomuk::Query::FileQuery::addExcludeFolder( const KUrl& folder )
{
    d->m_excludeFolders << folder;
}


void Nepomuk::Query::FileQuery::setExcludeFolders( const KUrl::List& folders )
{
    d->m_excludeFolders = folders;
}


KUrl::List Nepomuk::Query::FileQuery::excludeFolders() const
{
    return d->m_excludeFolders;
}


void Nepomuk::Query::FileQuery::setFileMode( FileMode mode )
{
    d->m_fileMode = mode;
}


Nepomuk::Query::FileQuery::FileMode Nepomuk::Query::FileQuery::fileMode() const
{
    return d->m_fileMode;
}
