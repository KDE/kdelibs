/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
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

#ifndef KSORTABLEVALUELIST_H
#define KSORTABLEVALUELIST_H

#include <qpair.h>
#include <qvaluelist.h>

template<class T> class KSortableItem : public QPair<int,T>
{
public:
    KSortableItem( int i, const T& t ) : QPair<int,T>( i, t ) {}
    KSortableItem() : QPair<int,T>( 0, QString::null ) {}
    virtual ~KSortableItem() {}

    void operator=( const KSortableItem<T>& i ) {
        first  = i.first;
        second = i.second;
    }

    // operators for sorting
    bool operator> ( const KSortableItem<T>& i2 ) {
        return (first > i2.first);
    }
    bool operator< ( const KSortableItem<T>& i2 ) {
        return (first < i2.first);
    }
    bool operator>= ( const KSortableItem<T>& i2 ) {
        return (first >= i2.first);
    }
    bool operator<= ( const KSortableItem<T>& i2 ) {
        return (first <= i2.first);
    }
    bool operator== ( const KSortableItem<T>& i2 ) {
        return (first == i2.first);
    }
    bool operator!= ( const KSortableItem<T>& i2 ) {
        return (first != i2.first);
    }

    T& value() {
        return second;
    }
    const T& value() const {
        return second;
    }

    int index() const {
        return first;
    }
};


// convenience
template <class T>class KSortableValueList : public QValueList<KSortableItem<T> >
{
public:
    void insert( int i, const T& t ) {
        QValueList<KSortableItem<T> >::append( KSortableItem<T>( i, t ) );
    }
    // add more as you please...

    T& operator[]( int i ) {
        return QValueList<KSortableItem<T> >::operator[]( i ).value();
    }
    const T& operator[]( int i ) const {
        return QValueList<KSortableItem<T> >::operator[]( i ).value();
    }
    
    void sort() {
        qHeapSort( *this );
    }
};

// template <class T> class KSortableValueListIterator : public QValueListIterator<KSortableItem<T>  >
// {
// };

#endif // KSORTABLEVALUELIST_H
