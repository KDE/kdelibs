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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qregexp.h>

#include <kfileitem.h>
#include <kglobal.h>

#include "kfilefilter.h"

KSimpleFileFilter::KSimpleFileFilter()
    : m_filterDotFiles( true ),
      m_filterSpecials( true ),
      m_modeFilter( 0 )
{
    m_nameFilters.setAutoDelete( true );
}

KSimpleFileFilter::~KSimpleFileFilter()
{
}

void KSimpleFileFilter::setFilterDotFiles( bool filter )
{
    m_filterDotFiles = filter;
}

void KSimpleFileFilter::setFilterSpecials( bool filter )
{
    m_filterSpecials = filter;
}

void KSimpleFileFilter::setNameFilters( const QString& nameFilters )
{
    m_nameFilters.clear();

    // Split on white space
    QStringList list = QStringList::split(' ', nameFilters);

    QStringList::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
        m_nameFilters.append(new QRegExp(*it, false, true ));
}

void KSimpleFileFilter::setMimeFilters( const QStringList& mimeFilters )
{
    m_mimeFilters = mimeFilters;
}

void KSimpleFileFilter::setModeFilter( mode_t mode )
{
    m_modeFilter = mode;
}

bool KSimpleFileFilter::passesFilter( const KFileItem *item ) const
{
    static const QString& dot    = KGlobal::staticQString(".");
    static const QString& dotdot = KGlobal::staticQString("..");

    const QString& name = item->name();

    if ( m_filterDotFiles && name.at(0) == '.' )
        return false;

    if ( m_filterSpecials && (name == dot || name == dotdot) )
        return false;

    if ( m_modeFilter && !(m_modeFilter & item->mode()) )
        return false;
    
    if ( !m_mimeFilters.isEmpty() ) {
        // correct or guessed mimetype -- we don't mind
        const QString& mime = item->mimeTypePtr()->name();
        bool ok = false;

        QStringList::ConstIterator it = m_mimeFilters.begin();
        for ( ; it != m_mimeFilters.end(); ++it ) {
            if ( (*it) == mime ) { // match!
                ok = true;
                break;
            }
        }
        if ( !ok )
            return false;
    }

    if ( !m_nameFilters.isEmpty() ) {
        bool ok = false;

        QPtrListIterator<QRegExp> it( m_nameFilters );
        for ( ; it.current(); ++it ) {
            if ( it.current()->exactMatch( name ) ) { // match! 
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

