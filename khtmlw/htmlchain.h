/* This file is part of the KDE libraries
    Copyright (C) 1998 Martin Jones (mjones@kde.org)

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

#ifndef __HTMLCHAIN_H__
#define __HTMLCHAIN_H__

class HTMLObject;

class HTMLChainElement
{
public:
    HTMLChainElement(HTMLObject *_obj,HTMLChainElement *_parent,
	HTMLChainElement *_child) :
	object( _obj ), parent( _parent ), child( _child ) {}

    HTMLObject *object;
    HTMLChainElement *parent;
    HTMLChainElement *child;
};

/*
 */
class HTMLChain
{
public:
    HTMLChain() { head = tail = curr = 0; }
    ~HTMLChain();

    void first() { curr = head; }
    void last()  { curr = tail; }
    void next()  { curr = curr->child; }
    void prev()  { curr = curr->parent; }

    HTMLObject *current() { return curr ? curr->object : 0; }

    void push( HTMLObject *obj );
    void pop();

protected:
    HTMLChainElement *head;
    HTMLChainElement *tail;
    HTMLChainElement *curr;
};

#endif

