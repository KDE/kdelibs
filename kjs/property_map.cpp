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

// ------------------------------ PropertyMap ----------------------------------

#include "property_map.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

using namespace KJS;

int uscompare(const UString &s1, const UString &s2)
{
  int len1 = s1.size();
  int len2 = s2.size();
  if (len1 < len2)
    return -1;
  else if (len1 > len2)
    return 1;
  else {
    return memcmp(s1.data(),s2.data(),len1*2);
  }
}

PropertyMap2::PropertyMap2()
{
  root = 0;
}

PropertyMap2::~PropertyMap2()
{
}

void PropertyMap2::put(UString name, ValueImp *value)
{
  // if not root; make the root the new node
  if (!root) {
    root = new PropertyMapNode(name,value,0);
    return;
  }

  // try to find the parent node
  PropertyMapNode *parent = root;
  int isLeft = false;
  while (true) {
    int cmp = uscompare(name,parent->name);
    if (cmp < 0) {
      // traverse to left node (if any)
      isLeft = true;
      if (!parent->left)
        break;
      else
        parent = parent->left;
    }
    else if (cmp > 0) {
      // traverse to right node (if any)
      isLeft = false;
      if (!parent->right)
        break;
      else
        parent = parent->right;
    }
    else {
      // a node with this name already exists; just replace the value
      parent->value = value;
      printf("dbg: existing value, replacing\n");
      return;
    }
  }

  // we found the parent
  assert(parent);

  if (isLeft) {
    assert(!parent->left);
    parent->left = new PropertyMapNode(name,value,parent);
    //if (!parent->right || parent->right->height == 0)
    //  parent->height = 1;
  }
  else {
    assert(!parent->right);
    parent->right = new PropertyMapNode(name,value,parent);
    //if (!parent->left || parent->left->height == 0)
    //  parent->height = 1;
  }
  updateHeight(parent);


  PropertyMapNode *node = parent;
  while (node) {
    PropertyMapNode *p = node->parent;
    balance(node); // may change node
    //    if (!p) {
      //      printf("dbg: setting root to %s\n",node->name.ascii());
      //      root = node; // in case node changed
    //    }
    node = p;
  }


  /*

  ###

  // update height in ancestors & balance the tree
  PropertyMapNode *node = parent;
  int height = 1;
  while (node) {
    if (height > node->height) {
      node->height = height;
    }

    if (node->left && node->right) {
      if (node->left->height > node->right->height+1) {
        // node's tree is "left heavy"
        //        rotateLL(node);
      }
      if (node->right->height > node->left->height+1) {
        // node's tree is "right heavy"
        //        rotateRR(node);
      }
    }

    height++;
    node = node->parent;
  }
  */
}

void PropertyMap2::remove(UString name)
{
  PropertyMapNode *node = getNode(name);
  if (!node) // name not in tree
    return;

  PropertyMapNode *replace;

  if (!node->right) {
    // no right node; replace with the left node
    replace = node->left;
  }
  else if (!node->left) {
    // no left node; replace with the right node
    replace = node->right;
  }
  else {
    // replace with the highest node, or left
    if (node->right->height > node->left->height)
      replace = node->right;
    else
      replace = node->left;
  }

  if (!node->parent) {
    root = replace;
  }
  else {
    assert(node == node->parent->left || node == node->parent->right);
    if (node == node->parent->left) // node is left of parent
      node->parent->left = replace;
    else // node is right of parent
      node->parent->right = replace;
    if (replace)
      replace->parent = node->parent;
  }
  delete node;

  // ### balance the tree
}

ValueImp *PropertyMap2::get(UString name)
{
  PropertyMapNode *n = getNode(name);
  return n ? n->value : 0;
}

void PropertyMap2::mark()
{
  // ###
}

void PropertyMap2::dump(PropertyMapNode *node, int indent)
{
  if (!node && indent > 0)
    return;
  if (!node)
    node = root;

  dump(node->right,indent+1);
  for (int i = 0; i < indent; i++) {
    printf("    ");
  }
  printf("[%d] %s\n",node->height,node->name.ascii());
  dump(node->left,indent+1);
}

PropertyMapNode *PropertyMap2::getNode(const UString &name)
{
  PropertyMapNode *node = root;

  while (true) {
    if (!node)
      return 0;

    int cmp = uscompare(node->name,name);
    if (cmp < 0)
      node = node->left;
    else if (cmp > 0)
      node = node->right;
    else
      return node;
  }
}

void PropertyMap2::balance(PropertyMapNode* node)
{
    printf("dbg: before balance, node = %s\n",node->name.ascii());
  bool rotated = false;
  bool isroot = (node == root);
  int lheight = node->left ? node->left->height : 0;
  int rheight = node->right ? node->right->height : 0;
  if (lheight > rheight+1) {
    assert(node->left);
    int llheight = node->left->left ? node->left->left->height : 0;
    int lrheight = node->left->right ? node->left->right->height : 0;
    if (llheight >= lrheight) {
      singleRotateRight(node->left);
    }
    else {
      // llheight < lrheight
      doubleRotateRight(node);
    }
    rotated = true;
  }
  else if (rheight > lheight+1) {
    int rlheight = node->right->left ? node->right->left->height : 0;
    int rrheight = node->right->right ? node->right->right->height : 0;
    if (rrheight >= rlheight) {
      singleRotateLeft(node->right);
    }
    else {
      // rrheight < rlheight
      doubleRotateLeft(node);
    }
    rotated = true;
  }

    printf("dbg: after balance, node = %s\n",node->name.ascii());
  if (rotated && isroot)
    root = node;
}

void PropertyMap2::updateHeight(PropertyMapNode* &node)
{
  int lheight = node->left ? node->left->height : 0;
  int rheight = node->right ? node->right->height : 0;
  if (lheight > rheight)
    node->height = lheight+1;
  else
    node->height = rheight+1;
  assert(node->parent != node);
  if (node->parent)
    updateHeight(node->parent);
}

void PropertyMap2::singleRotateLeft(PropertyMapNode* &node)
{
  /*
    Perform a "single left" rotation, e.g.

      a                b
     / \              / \
    c   b     -->    a   e
       / \          / \
      d   e        c   d
  */

  // set up node variables
  PropertyMapNode *parent = node->parent;
  assert(!parent || (node == parent->left || node == parent->right));
  bool isLeft = (parent && node == parent->left);
  PropertyMapNode *a = node;
  PropertyMapNode *c = a->left;
  PropertyMapNode *b = a->right;
  PropertyMapNode *d = b->left;
  PropertyMapNode *e = b->right;

  assert(a);
  assert(b);

  // perform rotation
  b->left = a;
  a->parent = b;

  b->right = e;
  if (e)
    e->parent = b;

  a->left = c;
  if (c)
    c->parent = a;

  a->right = d;
  if (d)
    d->parent = a;

  // set node's parent correctly
  node = b;
  node->parent = parent;
  if (parent) {
    if (isLeft)
      parent->left = node;
    else
      parent->right = node;
  }

  updateHeight(a);
  updateHeight(b);
}


void PropertyMap2::singleRotateRight(PropertyMapNode* &node)
{
  /*
    Perform a "single right" rotation, e.g.

        b              a
       / \            / \
      a   e   -->    c   b
     / \                / \
    c   d              d   e
  */

  // set up node variables
  PropertyMapNode *parent = node->parent;
  assert(!parent || (node == parent->left || node == parent->right));
  bool isLeft = (parent && node == parent->left);
  PropertyMapNode *b = node;
  PropertyMapNode *a = b->left;
  PropertyMapNode *e = b->right;
  PropertyMapNode *c = a->left;
  PropertyMapNode *d = a->right;

  assert(b);
  assert(a);

  // perform rotation
  a->left = c;
  if (c)
    c->parent = a;

  a->right = b;
  b->parent = a;

  b->left = d;
  if (d)
    d->parent = b;

  b->right = e;
  if (e)
    e->parent = b;

  // set node's parent correctly
  node = a;
  node->parent = parent;
  if (parent) {
    if (isLeft)
      parent->left = node;
    else
      parent->right = node;
  }

  updateHeight(b);
  updateHeight(a);
}

void PropertyMap2::doubleRotateLeft(PropertyMapNode* &node)
{
  /*
    Perform a "double left" rotation, e.g.

        a                      b
      /   \                  /   \
    d       c      -->     a       c
           / \            / \     / \
          b   g          d   e   f   g
         / \
        e   f

  */

  // set up node variables
  PropertyMapNode *parent = node->parent;
  assert(!parent || (node == parent->left || node == parent->right));
  bool isLeft = (parent && node == parent->left);

  PropertyMapNode *a = node;
  PropertyMapNode *d = a->left;
  PropertyMapNode *c = a->right;
  PropertyMapNode *b = c->left;
  PropertyMapNode *g = c->right;
  PropertyMapNode *e = b->left;
  PropertyMapNode *f = b->right;

  assert(a);
  assert(b);
  assert(c);

  // perform rotation
  b->left = a;
  a->parent = b;

  b->right = c;
  c->parent = b;

  a->left = d;
  if (d)
    d->parent = a;

  a->right = e;
  if (e)
    e->parent = a;

  c->left = f;
  if (f)
    f->parent = c;

  c->right = g;
  if (g)
    g->parent = c;


  // set node's parent correctly
  node = b;
  node->parent = parent;
  if (parent) {
    if (isLeft)
      parent->left = node;
    else
      parent->right = node;
  }

  updateHeight(a);
  updateHeight(c);
  updateHeight(b);
}

void PropertyMap2::doubleRotateRight(PropertyMapNode* &node)
{
  /*
    Perform a "double right" rotation, e.g.
  
          c                   b
        /   \               /   \
      a       g   -->     a       c
     / \                 / \     / \
    d   b               d   e   f   g
       / \
      e   f
  */

  // set up node variables
  PropertyMapNode *parent = node->parent;
  assert(!parent || (node == parent->left || node == parent->right));
  bool isLeft = (parent && node == parent->left);
  PropertyMapNode *c = node;
  PropertyMapNode *a = c->left;
  PropertyMapNode *d = a->left;
  PropertyMapNode *b = a->right;
  PropertyMapNode *e = b->left;
  PropertyMapNode *f = b->right;
  PropertyMapNode *g = c->right;

  assert(a);
  assert(b);
  assert(c);

  // perform rotation
  b->left = a;
  a->parent = b;

  b->right = c;
  c->parent = b;

  a->left = d;
  if (d)
    d->parent = a;

  a->right = e;
  if (e)
    e->parent = a;

  c->left = f;
  if (f)
    f->parent = c;

  c->right = g;
  if (g)
    g->parent = c;

  // set node's parent correctly
  node = b;
  node->parent = parent;
  if (parent) {
    if (isLeft)
      parent->left = node;
    else
      parent->right = node;
  }

  updateHeight(a);
  updateHeight(c);
  updateHeight(b);
}

