/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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
 */

#include <stdio.h>

#include "kjs.h"
#include "object.h"
#include "types.h"
#include "nodes.h"

using namespace KJS;

Property::Property(const UString &n, KJSO *o, int attr)
{
  name = n;
  object = o->ref();
  attribute = attr;
}

Completion::Completion(Compl c, KJSO *v, const UString &t)
{
  value.c = c;
  complVal = v ? v->ref() : 0L;
  strVal = t;
}

Reference::Reference(KJSO *b, const UString &s)
{
  value.base = b->ref();
  strVal = s;
}

Reference::~Reference()
{
  value.base->deref();
}

ListIterator::ListIterator(const List &l)
  : node(l.hook->next)
{
}

List::List()
{
  hook = new ListNode(0L, 0L, 0L);
  hook->next = hook;
  hook->prev = hook;
}

List::~List()
{
  clear();
  delete hook;
}

void List::append(KJSO *obj)
{
  ListNode *n = new ListNode(obj->ref(), hook->prev, hook);
  hook->prev->next = n;
  hook->prev = n;
}

void List::prepend(KJSO *obj)
{
  ListNode *n = new ListNode(obj->ref(), hook, hook->next);
  hook->next->prev = n;
  hook->next = n;
}

void List::removeFirst()
{
  erase(hook->next);
}

void List::removeLast()
{
  erase(hook->prev);
}

void List::clear()
{
  ListNode *n = hook->next;
  while (n != hook) {
    n = n->next;
    delete n->prev;
  }

  hook->next = hook;
  hook->prev = hook;
}

void List::erase(ListNode *n)
{
  if (n != hook) {
    n->next->prev = n->prev;
    n->prev->next = n->next;
    delete n;
  }
}

int List::size() const
{
  int s = 0;
  ListNode *node = hook;
  while ((node = node->next) != hook)
    s++;

  return s;
}

KJSO *List::at(int i) const
{
  if (i < 0 || i >= size())
    return newUndefined();

  ListIterator it = begin();
  int j = 0;
  while ((j++ < i))
    it++;

  return it->ref();
}

List *List::emptyList = 0L;

const List *List::empty()
{
  if (!emptyList)
    emptyList = new List;

  return emptyList;
}
