// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */


#ifndef _KJS_PROPERTY_MAP_H_
#define _KJS_PROPERTY_MAP_H_

#include "ustring.h"
#include "value.h"

namespace KJS {

  class PropertyMapNode {
  public:
    PropertyMapNode(UString &n, ValueImp *v, PropertyMapNode *p)
      : name(n), value(v), left(0), right(0), parent(p), height(1) {}
    UString name;
    ValueImp *value;

    void setLeft(PropertyMapNode *newLeft);
    void setRight(PropertyMapNode *newRight);
    void replace(PropertyMapNode *other);
    PropertyMapNode *findMax();
    PropertyMapNode *findMin();

    void remove(PropertyMapNode *node);

    PropertyMapNode *left;
    PropertyMapNode *right;
    PropertyMapNode *parent;
    int height;

  private:
    void setParent(PropertyMapNode *newParent);
  };

  /**
   * @internal
   *
   * Provides a name/value map for storing properties based on UString keys. The
   * map is implemented using an AVL tree, which provides O(log2 n) performance
   * for insertion and deletion, and retrieval.
   *
   * For a description of AVL tree operations, see
   * http://www.cis.ksu.edu/~howell/300f99/minavl/rotations.html
   * http://www.cgc.cs.jhu.edu/~jkloss/htmls/structures/avltree.html
   */

  // ### this class is unfinished - will be renamed to PropertyMap once
  // complete
  class PropertyMap2 {
  public:
    PropertyMap2();
    ~PropertyMap2();

    void put(UString name,ValueImp *value);
    void remove(UString name);
    PropertyMapNode *remove(PropertyMapNode *node);
    ValueImp *get(UString name);
    void mark();

    void dump(PropertyMapNode *node = 0, int indent = 0);
    void checkTree(PropertyMapNode *node = 0);

  private:
    PropertyMapNode *getNode(const UString &name);

    void balance(PropertyMapNode* node);
    void updateHeight(PropertyMapNode* &node);

    void rotateRR(PropertyMapNode* &node);
    void rotateLL(PropertyMapNode* &node);
    void rotateRL(PropertyMapNode* &node);
    void rotateLR(PropertyMapNode* &node);

    PropertyMapNode *root;
  };

}; // namespace

#endif // _KJS_PROPERTY_MAP_H_
