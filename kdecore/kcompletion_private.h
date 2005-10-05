/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef KCOMPLETION_PRIVATE_H
#define KCOMPLETION_PRIVATE_H

#include <qstring.h>
#include <ksortablevaluelist.h>

class KCompTreeNode;

#include <kallocator.h>

/**
 * @internal
 */
class KDECORE_EXPORT KCompTreeNodeList
{
public:
    KCompTreeNodeList() : first(0), last(0), m_count(0) {}
    KCompTreeNode *begin() const { return first; }
    KCompTreeNode *end() const { return last; }

    KCompTreeNode *at(uint index) const;
    void append(KCompTreeNode *item); 
    void prepend(KCompTreeNode *item); 
    void insert(KCompTreeNode *after, KCompTreeNode *item);
    KCompTreeNode *remove(KCompTreeNode *item);
    uint count() const { return m_count; }

private:
    KCompTreeNode *first, *last;
    uint m_count;
};

typedef KCompTreeNodeList KCompTreeChildren;

/**
 * A helper class for KCompletion. Implements a tree of QChar.
 *
 * The tree looks like this (containing the items "kde", "kde-ui",
 * "kde-core" and "pfeiffer". Every item is delimited with QChar( 0x0 )
 *
 *              some_root_node
 *                  /     \
 *                 k       p
 *                 |       |
 *                 d       f
 *                 |       |
 *                 e       e
 *                /|       |
 *             0x0 -       i
 *                / \      |
 *               u   c     f
 *               |   |     |
 *               i   o     f
 *               |   |     |
 *              0x0  r     e
 *                   |     |
 *                   e     r
 *                   |     |
 *                  0x0   0x0
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @internal
 */
class KDECORE_EXPORT KCompTreeNode : public QChar
{
public:
    KCompTreeNode() : QChar(), myWeight(0) {}
    KCompTreeNode( const QChar& ch, uint weight = 0 )
        : QChar( ch ),
          myWeight( weight ) {}
    ~KCompTreeNode();

    void * operator new( size_t s ) {
      return alloc.allocate( s );
    }
    void operator delete( void * s ) {
      alloc.deallocate( s );
    }

    // Returns a child of this node matching ch, if available.
    // Otherwise, returns 0L
    inline KCompTreeNode * find( const QChar& ch ) const {
      KCompTreeNode * cur = myChildren.begin();
      while (cur && (*cur != ch)) cur = cur->next;
      return cur;
    }
    KCompTreeNode *	insert( const QChar&, bool sorted );
    void 		remove( const QString& );

    inline int		childrenCount() const { return myChildren.count(); }

    // weighting
    inline void confirm() 	{ myWeight++; 		}
    inline void confirm(uint w) { myWeight += w;	}
    inline void decline() 	{ myWeight--; 		}
    inline uint weight() const 	{ return myWeight; 	}

    inline const KCompTreeChildren * children() const {
	return &myChildren;
    }
    inline const KCompTreeNode * childAt(int index) const {
	return myChildren.at(index);
    }
    inline const KCompTreeNode * firstChild() const {
	return myChildren.begin();
    }
    inline const KCompTreeNode * lastChild()  const {
	return myChildren.end();
    }

    /* We want to handle a list of KCompTreeNodes on our own, to not
       need to use QValueList<>.  And to make it even more fast we don't
       use an accessor, but just a public member.  */
    KCompTreeNode *next;
private:
    uint myWeight;
    KCompTreeNodeList	myChildren;
    static KZoneAllocator alloc;
};



// some more helper stuff
typedef KSortableValueList<QString> KCompletionMatchesList;

/**
 * @internal
 */
class KDECORE_EXPORT KCompletionMatchesWrapper
{
public:
    KCompletionMatchesWrapper( bool sort = false )
        : sortedList( sort ? new KCompletionMatchesList : 0L ),
          dirty( false )
    {}
    ~KCompletionMatchesWrapper() {
        delete sortedList;
    }

    void setSorting( bool sort ) {
        if ( sort && !sortedList )
            sortedList = new KCompletionMatchesList;
        else if ( !sort ) {
            delete sortedList;
            sortedList = 0L;
        }
        stringList.clear();
        dirty = false;
    }

    bool sorting() const {
        return sortedList != 0L;
    }

    void append( int i, const QString& string ) {
        if ( sortedList )
            sortedList->insert( i, string );
        else
            stringList.append( string );
        dirty = true;
    }

    void clear() {
        if ( sortedList )
            sortedList->clear();
        stringList.clear();
        dirty = false;
    }

    uint count() const {
        if ( sortedList )
            return sortedList->count();
        return stringList.count();
    }

    bool isEmpty() const {
        return count() == 0;
    }

    QString first() const {
        return list().first();
    }

    QString last() const {
        return list().last();
    }

    QStringList list() const;

    mutable QStringList stringList;
    KCompletionMatchesList *sortedList;
    mutable bool dirty;
};


#endif // KCOMPLETION_PRIVATE_H
