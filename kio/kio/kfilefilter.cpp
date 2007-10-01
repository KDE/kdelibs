/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfilefilter.h"

#include <QtCore/QRegExp>

#include <kfileitem.h>
#include <kglobal.h>

class KSimpleFileFilterPrivate
{
public:
    KSimpleFileFilterPrivate()
        : m_filterDotFiles( true ),
          m_filterSpecials( true ),
          m_modeFilter( 0 )
    {
    }
    QStringList         m_mimeFilters;
    bool                m_filterDotFiles :1;
    bool                m_filterSpecials :1;
    mode_t              m_modeFilter;
};

KSimpleFileFilter::KSimpleFileFilter()
    : d( new KSimpleFileFilterPrivate )
{
}

KSimpleFileFilter::~KSimpleFileFilter()
{
    delete d;
}

void KSimpleFileFilter::setFilterDotFiles( bool filter )
{
    d->m_filterDotFiles = filter;
}

bool KSimpleFileFilter::filterDotFiles() const
{
    return d->m_filterDotFiles;
}

void KSimpleFileFilter::setFilterSpecials( bool filter )
{
    d->m_filterSpecials = filter;
}

bool KSimpleFileFilter::filterSpecials() const
{
    return d->m_filterSpecials;
}

void KSimpleFileFilter::setNameFilters( const QStringList& nameFilters,
                                        Qt::CaseSensitivity caseSensitive )
{
    m_nameFilters.clear();

    foreach ( const QString &filter, nameFilters )
        m_nameFilters.append(QRegExp(filter, caseSensitive, QRegExp::Wildcard));
}

void KSimpleFileFilter::setMimeFilters( const QStringList& mimeFilters )
{
    d->m_mimeFilters = mimeFilters;
}

QStringList KSimpleFileFilter::mimeFilters() const
{
    return d->m_mimeFilters;
}

void KSimpleFileFilter::setModeFilter( mode_t mode )
{
    d->m_modeFilter = mode;
}

mode_t KSimpleFileFilter::modeFilter() const
{
    return d->m_modeFilter;
}

bool KSimpleFileFilter::passesFilter( const KFileItem &item ) const
{
    const QString& name = item.name();

    if ( d->m_filterDotFiles && item.isHidden() )
        return false;

    if ( d->m_filterSpecials && (name == "." || name == "..") )
        return false;

    if ( d->m_modeFilter && !(d->m_modeFilter & item.mode()) )
        return false;

    if ( !d->m_mimeFilters.isEmpty() ) {
        // correct or guessed mimetype -- we don't mind
        KMimeType::Ptr mime = item.mimeTypePtr();
        bool ok = false;

        QStringList::ConstIterator it = d->m_mimeFilters.begin();
        for ( ; it != d->m_mimeFilters.end(); ++it ) {
            if ( mime->is(*it) ) { // match!
                ok = true;
                break;
            }
        }
        if ( !ok )
            return false;
    }

    if ( !m_nameFilters.isEmpty() ) {
        bool ok = false;

        QList<QRegExp>::const_iterator it = m_nameFilters.begin();
        const QList<QRegExp>::const_iterator end = m_nameFilters.end();
        for ( ; it != end; ++it ) {
            if ( (*it).exactMatch( name ) ) { // match!
                ok = true;
                break;
            }
        }
        if ( !ok )
            return false;
    }

    return true; // passes the filter!
}

void KFileFilter::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KSimpleFileFilter::virtual_hook( int id, void* data )
{ KFileFilter::virtual_hook( id, data ); }

