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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#ifndef KCOMPLETION_PRIVATE_H
#define KCOMPLETION_PRIVATE_H

#include <qlist.h>
#include <qstring.h>

class KCompTreeNode;
typedef QValueList<KCompTreeNode *> KCompTreeChildren;

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
class KCompTreeNode : public QChar
{
public:
    KCompTreeNode() : QChar(), myWeight(0) {}
    KCompTreeNode( const QChar& ch, uint weight = 0 )
        : QChar( ch ),
          myWeight( weight ) {}
    ~KCompTreeNode();

    // Returns a child of this node matching ch, if available.
    // Otherwise, returns 0L
    inline KCompTreeNode * find( const QChar& ch ) const {
	KCompTreeChildren::ConstIterator it;
	for ( it = myChildren.begin(); it != myChildren.end(); ++it )
	    if ( *(*it) == ch )
		return *it;

	return 0L;
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
	return myChildren[index];
    }
    inline const KCompTreeNode * firstChild() const {
	return myChildren.first();
    }
    inline const KCompTreeNode * lastChild()  const {
	return myChildren.last();
    }

private:
    uint myWeight;
    KCompTreeChildren	myChildren;

};

#endif // KCOMPLETION_PRIVATE_H
