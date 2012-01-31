/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k3multipledrag.h"
#include "kdebug.h"

#ifndef QT_NO_DRAGANDDROP

K3MultipleDrag::K3MultipleDrag( QWidget *dragSource, const char *name )
    : Q3DragObject( dragSource, name )
{
    m_dragObjects.setAutoDelete( true );
}

void K3MultipleDrag::addDragObject( Q3DragObject *dragObject )
{
    //kDebug() << "K3MultipleDrag::addDragObject";
    m_dragObjects.append( dragObject );
    // We need to find out how many formats this dragObject supports
    int i = 0;
    while ( dragObject->format( i ) )
        ++i;
    m_numberFormats.append( i ); // e.g. if it supports two formats, 0 and 1, store 2.
}

QByteArray K3MultipleDrag::encodedData( const char *mime ) const
{
    //kDebug() << "K3MultipleDrag::encodedData " << mime;
    // Iterate over the drag objects, and find the format in the right one
    Q3PtrListIterator<Q3DragObject> it( m_dragObjects );
    for ( ; it.current(); ++it )
    {
        for ( int i = 0; it.current()->format( i ); ++i )
        {
            if ( ::qstrcmp( it.current()->format( i ), mime ) == 0 )
                return it.current()->encodedData( mime );
        }
    }
    return QByteArray();
}

const char* K3MultipleDrag::format( int i ) const
{
    //kDebug() << "K3MultipleDrag::format " << i;
    // example: m_numberFormats: 1, 4
    //          m_dragObjects: storeddrag, textdrag
    // i=0 -> storeddrag->format( 0 )
    // i=1 -> textdrag->format( 0 )
    // i=2 -> textdrag->format( 1 )
    // etc.
    QList<int>::ConstIterator nit = m_numberFormats.begin();
    QList<int>::ConstIterator nend = m_numberFormats.end();
    Q3PtrListIterator<Q3DragObject> it( m_dragObjects );
    for ( ; nit != nend && i >= *nit ; ++nit, ++it )
        i -= *nit;
    if ( it.current() )
        return it.current()->format( i );
    return 0;
}

void K3MultipleDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "moc_k3multipledrag.cpp"

#endif
