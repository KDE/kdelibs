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

#ifndef _TYPES_H_
#define _TYPES_H_

#include "object.h"

namespace KJS {

  /**
   * @short Handle for an Undefined type.
   */
  class Undefined : public KJSO {
  public:
    Undefined();
    virtual ~Undefined();
  };

  /**
   * @short Handle for a Null type.
   */
  class Null : public KJSO {
  public:
    Null();
    virtual ~Null();
  };

  class BooleanImp;
  class NumberImp;
  class StringImp;

  /**
   * @short Handle for a Boolean type.
   */
  class Boolean : public KJSO {
    friend BooleanImp;
  public:
    Boolean(bool b = false);
    virtual ~Boolean();
    bool value() const;
  private:
    Boolean(BooleanImp *d);
  };

  /**
   * @short Handle for a Number type.
   */
  class Number : public KJSO {
    friend NumberImp;
  public:
    Number(int i);
    Number(unsigned int u);
    Number(double d = 0.0);
    Number(long unsigned int l);
    virtual ~Number();
    double value() const;
    int intValue() const;
    bool isNaN() const;
    bool isInf() const;
  private:
    Number(NumberImp *d);
  };

  /**
   * @short Handle for a String type.
   */
  class String : public KJSO {
    friend StringImp;
  public:
    String(const UString &s = "");
    virtual ~String();
    UString value() const;
  private:
    String(StringImp *d);
  };

  /**
   * @short Handle for a Completion type.
   */
  class Completion : public KJSO {
  public:
    Completion(Imp *d = 0L);
    Completion(Compl c);
    Completion(Compl c, const KJSO& v, const UString &t = UString::null);
    virtual ~Completion();
    Compl complType() const;
    bool isValueCompletion() const;
    KJSO value() const;
  };

  class List;
  class ListIterator;

  /**
   * @internal
   */
  class ListNode {
    friend List;
    friend ListIterator;
    ListNode(KJSO obj, ListNode *p, ListNode *n)
      : member(obj), prev(p), next(n) {};
    KJSO member;
    ListNode *prev, *next;
  };

  /**
   * @short Iterator for @ref KJS::List objects.
   */
  class ListIterator {
    friend List;
    ListIterator();
    ListIterator(ListNode *n) : node(n) { }
  public:
    /**
     * Construct an iterator that points to the first element of the list.
     * @param l The list the iterator will operate on.
     */
    ListIterator(const List &list);
    /**
     * Assignment constructor.
     */
    ListIterator& operator=(const ListIterator &iterator)
      { node=iterator.node; return *this; }
    /**
     * Copy constructor.
     */
    ListIterator(const ListIterator &i) : node(i.node) { }
    /**
     * Dereference the iterator.
     * @return A pointer to the element the iterator operates on.
     */
    KJSO* operator->() const { return &node->member; }
    //    operator KJSO* () const { return node->member; }
    KJSO operator*() const { return node->member; }
    /**
     * Conversion to @ref KJS::KJSO*
     * @return A pointer to the element the iterator operates on.
     */
    //    operator KJSO*() const { return node->member; }
    /**
     * Postfix increment operator.
     * @return The element after the increment.
     */
    KJSO operator++() { node = node->next; return node->member; }
    /**
     * Prefix increment operator.
     */
    KJSO operator++(int) { const ListNode *n = node; ++*this; return n->member; }
    /**
     * Postfix decrement operator.
     */
    KJSO operator--() { node = node->prev; return node->member; }
    /**
     * Prefix decrement operator.
     */
    KJSO operator--(int) { const ListNode *n = node; --*this; return n->member; }
    /**
     * Compare the iterator with another one.
     * @return True if the two iterators operate on the same list element.
     * False otherwise.
     */
    bool operator==(const ListIterator &it) const { return (node==it.node); }
    /**
     * Check for inequality with another iterator.
     * @return True if the two iterators operate on different list elements.
     */
    bool operator!=(const ListIterator &it) const { return (node!=it.node); }
  private:
    ListNode *node;
  };

  /**
   * @short Native list type.
   *
   * List is a native ECMAScript type. List values are only used for
   * intermediate results of expression evaluation and cannot be stored
   * as properties of objects.
   *
   * The class takes care of memory management via reference counting.
   */
  class List {
    friend ListIterator;
  public:
    /**
     * Constructor.
     */
    List();
    /**
     * Destructor.
     */
    ~List();
    /**
     * Append an object to the end of the list.
     *
     * @param obj Pointer to object.
     */
    void append(const KJSO& obj);
    /**
     * Insert an object at the beginning of the list.
     *
     * @param obj Pointer to object.
     */
    void prepend(const KJSO& obj);
    /**
     * Remove the element at the beginning of the list.
     */
    void removeFirst();
    /**
     * Remove the element at the end of the list.
     */
    void removeLast();
    /**
     * Remove all elements from the list.
     */
    void clear();
    /**
     * @return A @ref KJS::ListIterator pointing to the first element.
     */
    ListIterator begin() const { return ListIterator(hook->next); }
    /**
     * @return A @ref KJS::ListIterator pointing to the last element.
     */
    ListIterator end() const { return ListIterator(hook); }
    /**
     * @return true if the list is empty. false otherwise.
     */
    bool isEmpty() const { return (hook->prev == hook); }
    /**
     * @return the current size of the list.
     */
    int size() const;
    /**
     * Retrieve an element at an indexed position. If you want to iterate
     * trough the whole list using @ref KJS::ListIterator will be faster.
     *
     * @param i List index.
     * @return Return the element at position i. @ref KJS::Undefined if the
     * index is out of range.
     */
    KJSO at(int i) const;
    /**
     * Equivalent to @ref at.
     */
    KJSO operator[](int i) const { return at(i); }
    /**
     * Returns a pointer to a static instance of an empty list. Useful if a
     * function has a @ref KJS::List parameter.
     */
    static const List *empty();

#ifdef KJS_DEBUG_MEM
    /**
     * @internal
     */
    static int count;
#endif
  private:
    void erase(ListNode *n);
    ListNode *hook;
    static List *emptyList;
  };

}; // namespace


#endif
