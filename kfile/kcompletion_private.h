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
  KCompTreeNode();
  KCompTreeNode( const QChar& );
  ~KCompTreeNode();

  KCompTreeNode * 	find( const QChar& ) const;
  KCompTreeNode *	insert( const QChar&, bool sorted );
  void 			remove( const QString& );

  int 			childrenCount() const { return myChildren.count(); }

  const KCompTreeChildren * 	children() const { return &myChildren; }
  const KCompTreeNode * 	child() const { return myChildren.first(); }

private:
  KCompTreeChildren	myChildren;

};



/**
 * A KCompFork stores one node in the tree, an index describing the current
 * index of its children-list and a string representing the string from the
 * root of the tree to (and including) that node.
 *
 * @internal
 */

struct KCompFork {
  QString		string;
  KCompTreeNode *	node;
  int 			index;
};
typedef QList<KCompFork> 		KCompForkBaseList;

/**
 * Implements a list of KCompFork-items to make KCompletion::previousMatch()
 * and  nextMatch() possible.
 *
 * @internal
 */
class KCompForkList : public KCompForkBaseList
{
public:
  void append( QString string, const KCompTreeNode *node ) {
      ASSERT( node != 0L );
      KCompFork *fork = new KCompFork;
      fork->string = string;
      fork->node = const_cast<KCompTreeNode*>( node );
      fork->index = 0;
  }

  KCompFork * previous() {
      KCompFork *cur = current();
      if ( cur )
	  cur->index++;
      return KCompForkBaseList::prev();
  }

  KCompFork * next() {
      KCompFork *cur = current();
      if ( cur )
	  cur->index--;
      return KCompForkBaseList::next();
  }
};



#endif // KCOMPLETION_PRIVATE_H
