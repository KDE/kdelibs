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

/**
 * KSortableItem is a @ref QPair that provides several operators
 * for sorting.
 * @see KSortableValueList
 */
template<class T, class Key = int> class KSortableItem : public QPair<Key,T>
{
public:
    /**
     * Creates a new KSortableItem with the given values.
     * @param i the first value
     * @param t the second value
     */
    KSortableItem( Key i, const T& t ) : QPair<Key, T>( i, t ) {}
    /**
     * Creates a new KSortableItem that copies another one.
     * @param rhs the other item to copy
     */
    KSortableItem( const KSortableItem<T, Key> &rhs ) 
        : QPair<Key,T>( rhs.first, rhs.second ) {}

    /**
     * Creates a new KSortableItem with uninitialized values.
     */
    KSortableItem() {}

    /**
     * Assignment operator, just copies the item.
     */
    KSortableItem<T, Key> &operator=( const KSortableItem<T, Key>& i ) {
        first  = i.first;
        second = i.second;
        return *this;
    }

    // operators for sorting
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator> ( const KSortableItem<T, Key>& i2 ) const {
        return (i2.first < first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator< ( const KSortableItem<T, Key>& i2 ) const {
        return (first < i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator>= ( const KSortableItem<T, Key>& i2 ) const {
        return (first >= i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator<= ( const KSortableItem<T, Key>& i2 ) const {
        return !(i2.first < first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator== ( const KSortableItem<T, Key>& i2 ) const {
        return (first == i2.first);
    }
    /**
     * Compares the two items. This implementation only compares
     * the first value.
     */
    bool operator!= ( const KSortableItem<T, Key>& i2 ) const {
        return (first != i2.first);
    }

    /**
     * Returns the second value.
     * @return the second value
     */
    T& value() {
        return second;
    }

    /**
     * Returns the second value.
     */
    const T& value() const {
        return second;
    }

    /**
     * Returns the first value.
     * @param the first value
     */
    Key index() const {
        return first;
    }
};


/**
 * KSortableValueList is a special @ref QValueList for 
 * @ref KSortableItem. It includes convenience operators
 * to get the first value of the KSortableItem and a method
 * to sort all items.
 */
template <class T, class Key = int>
class KSortableValueList : public QValueList<KSortableItem<T, Key> >
{
public:
    /**
     * Insert a KSortableItem with the given values.
     * @param i the first value
     * @param t the second value
     */
    void insert( Key i, const T& t ) {
        QValueList<KSortableItem<T, Key> >::append( KSortableItem<T, Key>( i, t ) );
    }
    // add more as you please...

    /**
     * Returns the first value of the KSortableItem at the given position.
     * @return the first value of the KSortableItem
     */
    T& operator[]( Key i ) {
        return QValueList<KSortableItem<T, Key> >::operator[]( i ).value();
    }

    /**
     * Returns the first value of the KSortableItem at the given position.
     * @return the first value of the KSortableItem
     */
    const T& operator[]( Key i ) const {
        return QValueList<KSortableItem<T, Key> >::operator[]( i ).value();
    }

    /**
     * Sorts the KSortableItems.
     */
    void sort() {
        qHeapSort( *this );
    }
};

// template <class T> class KSortableValueListIterator : public QValueListIterator<KSortableItem<T>  >
// {
// };

#endif // KSORTABLEVALUELIST_H
